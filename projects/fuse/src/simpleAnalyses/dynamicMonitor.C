#include <boost/foreach.hpp>
using namespace std;

#include "dynamicMonitor.h"

#include "sageInterface.h"
#include "sageBuilder.h"
#include "Rose/StringUtility.h"


using namespace SageInterface;
using namespace SageBuilder;

#define dynMonDL 0

namespace fuse {

SgExpression* buildFuncsToCommaInsert(list<string>::iterator funcName, list<string>::iterator funcNameEnd,
                                      SgType* retType,
                                      list<vector< SgExpression *> >::iterator args,
                                      list<SgScopeStatement*>::iterator scope) {
  list<string>::iterator funcNameNext = funcName; ++funcNameNext;

  // If this is the last function call, create and return it
  if(funcNameNext == funcNameEnd)
    return buildFunctionCallExp(SgName(*funcName),
                                retType,
                                buildExprListExp(*args),
                                *scope);
  else {
    list<vector< SgExpression *> >::iterator argsNext = args; ++argsNext;
    list<SgScopeStatement*>::iterator scopeNext = scope; ++scopeNext;
    return buildCommaOpExp(buildFunctionCallExp(SgName(*funcName),
                                                retType,
                                                buildExprListExp(*args),
                                                *scope),
                           buildFuncsToCommaInsert(funcNameNext, funcNameEnd, retType, argsNext, scopeNext));
  }
}

// Given a SgExpression that is a possible target for insertion using insertFuncCallAfterUsingCommaOp,
// return the SgExpression that is a valid target and denotes the same state
SgExpression* getValidInsertTarget(SgExpression* anchor) {
  // Unwrap any arrows, dots and array reference expressions
  while(isSgArrowExp(anchor->get_parent()) || isSgDotExp(anchor->get_parent()) || isSgPntrArrRefExp(anchor->get_parent())) {
    anchor = isSgExpression(anchor->get_parent());
  }
  return anchor;
}

//! Insert an expression (new_exp ) after another expression (anchor) has possible side effects, with minimum changes to the original semantics. This is done by using two comma operators:  type T1; ... ((T1 = anchor, new_exp),T1) )... , where T1 is a temp variable saving the possible side effect of anchor. The top level comma op exp is returned. The reference to T1 in T1 = anchor is saved in temp_ref.
SgCommaOpExp * insertFuncCallAfterUsingCommaOp(list<string>& funcName,
                                               SgType* retType,
                                               list<vector< SgExpression *> >& args,
                                               list<SgScopeStatement*> scope,
                                               SgExpression* anchor)
{
  ROSE_ASSERT (anchor != NULL);

  SgNode* parent = anchor->get_parent();
  //if(parent == NULL) {
    cerr << "anchor="<<SgNode2Str(anchor)<<endl;
    cerr << "funcName="<<endl;
    BOOST_FOREACH(const string& name, funcName) { cerr << "    "<<name<<endl; }
    cerr << "args="<<endl;
    int i=0;
    BOOST_FOREACH(const vector<SgExpression*>& argsL, args) {
      cerr << "    "<<i<<":"<<endl;
      BOOST_FOREACH(SgExpression* arg, argsL) {
        cerr << "        "<<SgNode2Str(arg)<<endl;
      }
      ++i;
    }
  //}
  //ROSE_ASSERT (parent != NULL);
  if(parent==NULL) return NULL;

  // Indicates whether the anchor is assigned within its parent expression.
  // If this is a binary op, where the lhs is assigned, OR
  //            a unary op, that assigns its operand
  bool anchorAssigned =
      ((isSgAssignOp(anchor->get_parent()) || isSgCompoundAssignOp(anchor->get_parent())) &&
       anchor==isSgBinaryOp(anchor->get_parent())->get_lhs_operand()) ||
      ((isSgPlusPlusOp(anchor->get_parent()) || isSgMinusMinusOp(anchor->get_parent())) &&
       anchor==isSgUnaryOp(anchor->get_parent())->get_operand());

  // insert TYPE T1; right before the enclosing statement of anchor
  SgType * t = removeConst(anchor ->get_type());
  ROSE_ASSERT (t != NULL);
  if(anchorAssigned) t = buildPointerType(t);
  SgStatement * enclosing_stmt = getEnclosingStatement(anchor);
  ROSE_ASSERT (enclosing_stmt != NULL);

  gensym_counter ++;
  string temp_name = "_t_"+ Rose::StringUtility::numberToString(gensym_counter);
  SgVariableDeclaration* t_decl = buildVariableDeclaration(temp_name, t, NULL, enclosing_stmt->get_scope());
  insertStatementBefore (enclosing_stmt, t_decl);
  SgVariableSymbol * temp_sym = getFirstVarSym (t_decl);
  ROSE_ASSERT (temp_sym != NULL);

  // build ((T1 = anchor, new_exp),T1) )
  SgCommaOpExp * result;

  // If the anchor is assigned, the temporary variable holds a pointer to the anchor
  if(anchorAssigned) {
    // Add references to temp_sym to all argument lists
    for(list<vector< SgExpression *> >::iterator a=args.begin(); a!=args.end(); ++a)
      a->push_back(buildPointerDerefExp(buildVarRefExp(temp_sym)));

    SgVarRefExp * first_ref = buildVarRefExp(temp_sym);
    result =
        buildCommaOpExp (
           buildCommaOpExp (
               buildAssignOp (first_ref, buildAddressOfOp(deepCopy(anchor))),
               buildFuncsToCommaInsert(funcName.begin(), funcName.end(),
                                       retType,
                                       args.begin(), scope.begin())),
           buildPointerDerefExp(buildVarRefExp(temp_sym)));

  // If the anchor is not assigned, the temporary variable holds the anchor's value
  } else {
    // Add references to temp_sym to all argument lists
    for(list<vector< SgExpression *> >::iterator a=args.begin(); a!=args.end(); ++a)
      a->push_back(buildVarRefExp(temp_sym));

    SgVarRefExp * first_ref = buildVarRefExp(temp_sym);
    result =
        buildCommaOpExp (
           buildCommaOpExp (
               buildAssignOp (first_ref, deepCopy(anchor)),
               buildFuncsToCommaInsert(funcName.begin(), funcName.end(),
                                       retType,
                                       args.begin(), scope.begin())),
           buildVarRefExp(temp_sym));
  }

  replaceExpression(anchor, result, false);

  return result;
}


// Transformation that adds a call to initialize Sight and dynamic monitor state into main)_
class InsertInitFinTransform :public SgSimpleProcessing
{
  public:
  // Maintains al the Statements of each type that need to be included in each function.
  // Once we've collected them all we'll insert them into each function in the appropriate relative order.
  map<SgScopeStatement*, list<SgStatement*> > funcInit;
  map<SgScopeStatement*, list<SgStatement*> > funcFin;

  // The body of the main function
  SgScopeStatement* mainBody;

  public:
  InsertInitFinTransform() {
    mainBody = NULL;
  }

  void visit (SgNode *n)
  {
    SgFunctionDefinition* def = isSgFunctionDefinition(n);
    if(!def) return;

    SgBasicBlock *block = def->get_body();
    ROSE_ASSERT(block);
    if (block != NULL)
    {
  //    list<SgExprStatement*> prepends;

      //cout << "def->get_parent()="<<def->get_parent()->class_name()<<" | "<<def->get_parent()->unparseToString()<<endl;

      if(def->get_declaration()->get_name().getString()=="main") {
        mainBody = def->get_body();

        funcInit[def->get_body()].push_back(buildFunctionCallStmt(SgName("SightInitialize"),
                                                      buildVoidType(),
                                                      buildExprListExp(),
                                                      block));

        funcFin[def->get_body()].push_back(buildFunctionCallStmt(SgName("SightFinalize"),
                                                      buildVoidType(),
                                                      buildExprListExp(),
                                                      block));
      }
    }
  }

  void appendInitStmtToMain(SgStatement* stmt) {
    ROSE_ASSERT(mainBody);
    funcInit[mainBody].push_back(stmt);
  }

  void prependFinStmtToMain(SgStatement* stmt) {
    ROSE_ASSERT(mainBody);
    funcFin[mainBody].push_front(stmt);
  }
};

void prependStatementsToScope(const map<SgScopeStatement*, list<SgStatement*> >& prepends) {
  for(map<SgScopeStatement*, list<SgStatement*> >::const_iterator scope=prepends.begin(); scope!=prepends.end(); ++scope) {
    //cout << "Scope "<<SgNode2Str(scope->first)<<endl;
    for(list<SgStatement*>::const_reverse_iterator p=scope->second.rbegin(); p!=scope->second.rend(); ++p) {
//    cout << "    Prepending "<<SgNode2Str(*p)<<endl;
      prependStatement(*p, scope->first);
    }
  }
}

void appendStatementsToScope(const map<SgScopeStatement*, list<SgStatement*> >& prepends) {
  for(map<SgScopeStatement*, list<SgStatement*> >::const_iterator scope=prepends.begin(); scope!=prepends.end(); ++scope) {
    //cout << "Scope "<<SgNode2Str(scope->first)<<endl;
    for(list<SgStatement*>::const_iterator p=scope->second.begin(); p!=scope->second.end(); ++p) {
//    cout << "    Appending "<<SgNode2Str(*p)<<endl;
      appendStatement(*p, scope->first);
    }
  }
}

/***************************
 ***** Dynamic Monitor *****
 ***************************/
DynamicMonitor::DynamicMonitor(): UndirDataflow(/*trackBase2RefinedPartEdgeMapping*/false, /*useSSA*/false) {
}

// Collects into the set reachingDefs the defs that transitively reach the uses in the given set and are not
// already in defs.
/*int DynamicMonitor::collectReachingDefs(const std::set<SSAMemLocObjectPtr>& uses, std::set<SSAMemLocObjectPtr>& reachingDefs) {
  int numDefsAdded = 0;
  BOOST_FOREACH(const SSAMemLocObjectPtr& u, uses) {
    //cout << ":        reaching use="<<u->str()<<endl;
    const set<SSAMemLocObjectPtr>& defs = ssa->getDefs(u);

    BOOST_FOREACH(const SSAMemLocObjectPtr& d, defs) {
      //cout << ":            def="<<d->str()<<endl;
      // If this def is not already in reachingDefs, add it and transitively add the uses that feed into it
      if(reachingDefs.find(d) == reachingDefs.end()) {
        // Add more reaching definitions.
        if(collectReachingDefs(ssa->getUses(d->getLoc()), reachingDefs)==0) {
          // If no new reaching definitions are found
          ++numDefsAdded;
      }
    }
  }
  }
*/

set<SSAMemLocObjectPtr> emptyRDSet;
void updateFrontier(SSAMemLocObjectPtr rd, SSAMemLocObjectPtr d,
                    map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >& frontier,
                    set<SSAMemLocObjectPtr>& interior,
                    map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >& newFrontier,
                    set<SSAMemLocObjectPtr>& freshFrontier,
                    bool& modified) {
  SIGHT_VERB_DECL(scope, ("updateFrontier"), 1, dynMonDL)
  // If rd is not already on the interior or the current frontier, move d from the
  // frontier to the interior and add the current reaching def to the frontier
  SIGHT_VERB(dbg << "reaching def="<<rd->str()<<endl, 1, dynMonDL)
  SIGHT_VERB(dbg << "in interior="<<(interior.find(rd) != interior.end())<<", on newFrontier="<<(newFrontier.find(rd) != newFrontier.end())<<", on frontier="<<(frontier.find(rd) != frontier.end())<<endl, 1, dynMonDL)
  if(interior.find(rd) == interior.end() && newFrontier.find(rd) == newFrontier.end()) {
    newFrontier.erase(d);
    interior.insert(d);
    newFrontier.insert(make_pair(rd, emptyRDSet));
    freshFrontier.insert(rd);
    modified = true;
  // If rd is in the interior or the frontier, remove d since its dominated by the interior
  } else
    newFrontier.erase(d);
}

// Find all the uses at the location of the startingDef and the defs that reach them.
// Return a mapping of these uses to their corresponding reaching defs
map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > DynamicMonitor::getReachingUse2Defs(const set<SSAMemLocObjectPtr>& startingDefs) {
  map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > reachingUse2Defs;

  BOOST_FOREACH(const SSAMemLocObjectPtr& du, startingDefs) {
    if(du->getAccess()==SSAMemLocObject::def) {
      const set<SSAMemLocObjectPtr>& reachingUses = ssa->getUses(du->getLoc());
      BOOST_FOREACH(const SSAMemLocObjectPtr& u, reachingUses) {
        const set<SSAMemLocObjectPtr>& reachingDefs = ssa->getDefs(u);
        BOOST_FOREACH(const SSAMemLocObjectPtr& rd, reachingDefs) {
          reachingUse2Defs[u].insert(rd);
        }
      }
    } else if(du->getAccess()==SSAMemLocObject::phiDef) {
      const set<SSAMemLocObjectPtr>& reachingDefs = ssa->getReachingDefsAtPhiDef(du);
      BOOST_FOREACH(const SSAMemLocObjectPtr& rd, reachingDefs) {
        reachingUse2Defs[du].insert(rd);
      }
    } else if(du->getAccess()==SSAMemLocObject::use) {
      const set<SSAMemLocObjectPtr>& reachingDefs = ssa->getDefs(du);
      BOOST_FOREACH(const SSAMemLocObjectPtr& rd, reachingDefs) {
        reachingUse2Defs[du].insert(rd);
      }
    }
  }

  return reachingUse2Defs;
}

std::set<SSAMemLocObjectPtr> DynamicMonitor::collectReachingDefs(SSAMemLocObjectPtr startingDef) {
  SIGHT_VERB_DECL(scope, ("collectReachingDefs"), 1, dynMonDL)

  // Set of MemLocs that have already been passed on the way from startingDef to frontier
  set<SSAMemLocObjectPtr> interior;

  // Maps non-phi scalar defs on the frontier to the empty set.
  // Maps scalar uses on the frontier to the set set of phiDefs that reach them.
  // A use may be in the key of frontier and on the interior if it is reached from
  // both defs and phiDefs.
  // We will generate tracking statements for all the keys of frontier that are not in interior.
  // This scheme makes it possible to keep track of the defs and uses that reach
  // startingDefs while allowing for the possibility of tracking through phiDefs to find the scalar defs
  // that reach them.
  map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > frontier;
  frontier[startingDef] = set<SSAMemLocObjectPtr>();

  // Frontier entries that just got placed on the frontier and thus need further traversal
  set<SSAMemLocObjectPtr> freshFrontier;
  freshFrontier.insert(startingDef);

  // Records the SSAMemLocObjects that have already been visited. This is important
  // for phiDefs that are added as values rather than keys in frontier, and are not
  // added to interior.
  set<SSAMemLocObjectPtr> visited;
  set<pair<SSAMemLocObjectPtr, SSAMemLocObjectPtr> > visitedPair;

  bool modified;
  do {
    SIGHT_VERB_DECL(scope, ("Iteration"), 1, dynMonDL)
    modified = false;

    SIGHT_VERB_IF(1, dynMonDL)
#ifndef DISABLE_SIGHT
    {scope s("frontier");
      for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator f=frontier.begin(); f!=frontier.end(); ++f) {
        scope s2(sight::txt()<<"Use "<<f->first->str());
        BOOST_FOREACH(const SSAMemLocObjectPtr& d, f->second) {
          dbg << d->str()<<endl;
        }
      }
    }

    {scope s("freshFrontier");
      BOOST_FOREACH(const SSAMemLocObjectPtr& d, freshFrontier) {
        dbg << d->str()<<endl;
      }
    }

    {scope s("interior");
    BOOST_FOREACH(const SSAMemLocObjectPtr& d, interior) {
      dbg << d->str()<<endl;
    }
    }
#endif
    SIGHT_VERB_FI()

    map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > newFrontier = frontier;
    set<SSAMemLocObjectPtr> newFreshFrontier;

    {
      SIGHT_VERB_DECL(scope, ("Frontier iteration"), 1, dynMonDL)
    // Iterate over all the defs on the frontier
    //for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator f=frontier.begin(); f!=frontier.end(); ++f) {
    BOOST_FOREACH(const SSAMemLocObjectPtr& frontierKey, freshFrontier) {
      map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator f = frontier.find(frontierKey);
      //ROSE_ASSERT(f != frontier.end());
      if(f == frontier.end()) continue;

      map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > reachingUse2Defs;
      SIGHT_VERB_DECL(scope, (sight::txt()<<"Entry "<<f->first->str()), 1, dynMonDL)

      if(f->first->getAccess()==SSAMemLocObject::def) {
        ROSE_ASSERT(f->second.size()==0);
        set<SSAMemLocObjectPtr> defs; defs.insert(f->first);
        reachingUse2Defs = getReachingUse2Defs(defs);

      } else if(f->first->getAccess()==SSAMemLocObject::use) {
        //ROSE_ASSERT(f->second.size()>0);
        reachingUse2Defs = getReachingUse2Defs(f->second);
        newFrontier.erase(f->first);
      }

      for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator u2rd=reachingUse2Defs.begin(); u2rd!=reachingUse2Defs.end(); ++u2rd) {
        SIGHT_VERB_DECL(scope, (sight::txt()<<"Use "<<u2rd->first->str()), 1, dynMonDL)
        BOOST_FOREACH(const SSAMemLocObjectPtr& rd, u2rd->second) {
          SIGHT_VERB_DECL(scope, (sight::txt()<<rd->str()), 1, dynMonDL);

          // If this def is definite, replace the original def with this one
          if(rd->getAccess()==SSAMemLocObject::def) {
            SgType* type = getType(rd->getSgNode());
            if(type==NULL) cerr << "ERROR: NULL type for "<<SgNode2Str(rd->getSgNode())<<", rd="<<rd->str()<<endl;
            ROSE_ASSERT(type);
            SIGHT_VERB(dbg << "ScalarType="<<(type->isIntegerType() || type->isFloatType())<<", rd="<<rd->str()<<endl, 1, dynMonDL)

            // If this is a scalar def
            if(type->isIntegerType() || type->isFloatType()) {
              if(f->first->getAccess()==SSAMemLocObject::def) {
                if(visited.find(rd) == visited.end()) {
                  visited.insert(rd);
                  updateFrontier(rd, f->first, frontier, interior, newFrontier, newFreshFrontier, modified);
                }
              } else if(f->first->getAccess()==SSAMemLocObject::use) {
                if(visited.find(rd) == visited.end()) {
                  visited.insert(rd);
                  interior.insert(f->first);
                  SIGHT_VERB(dbg << "Use -> Def, Adding to frontier"<<endl, 1, dynMonDL)
                  newFrontier[rd] = emptyRDSet;
                  newFreshFrontier.insert(rd);
                }
                modified = true;
              }
            }
          } else if(rd->getAccess()==SSAMemLocObject::phiDef) {
            if(f->first->getAccess()==SSAMemLocObject::def) {
              if(visitedPair.find(make_pair(u2rd->first, rd)) == visitedPair.end()) {
                modified = interior.find(f->first)==interior.end();
                if(getType(u2rd->first->getSgNode()) == getType(rd->getSgNode()))
                interior.insert(f->first);
                visitedPair.insert(make_pair(u2rd->first, rd));

                modified = (newFrontier.find(u2rd->first) == newFrontier.end() ||
                            newFrontier[u2rd->first].find(rd) == newFrontier[u2rd->first].end()) || modified;

                newFrontier.erase(f->first);
                newFrontier[u2rd->first].insert(rd);
                newFreshFrontier.insert(u2rd->first);
                SIGHT_VERB(dbg << "Def -> PhiDef, Adding to frontier @ "<<u2rd->first->str()<<endl, 1, dynMonDL)
              } else {
                visited.insert(f->first);
                if(newFrontier.find(f->first)==newFrontier.end()) {
                  //updateFrontier(rd, f->first, frontier, interior, newFrontier, newFreshFrontier, modified);
                  newFrontier[f->first] = emptyRDSet;
                  // Do not add f->first onto newFreshFrontier since we've now abandoned the search
                  // for defs that reach this use
                  SIGHT_VERB(dbg << "Stopping search for defs reaching "<<u2rd->first->str()<<endl, 1, dynMonDL)
                    } else {
                  SIGHT_VERB(dbg << "Tried to stop search for defs reaching "<<u2rd->first->str()<<", but another search path discovered"<<endl, 1, dynMonDL) }
              }
            } else if(f->first->getAccess()==SSAMemLocObject::use) {
              if(visitedPair.find(make_pair(f->first, rd)) == visitedPair.end()) {
                visitedPair.insert(make_pair(f->first, rd));
                modified = (newFrontier.find(f->first) == newFrontier.end() ||
                            newFrontier[f->first].find(rd) == newFrontier[f->first].end());
                newFrontier[f->first].insert(rd);
                newFreshFrontier.insert(f->first);
                SIGHT_VERB(dbg << "Use -> PhiDef, Adding to frontier @ "<<f->first->str()<<endl, 1, dynMonDL)
              } else {
                visited.insert(f->first);
                //updateFrontier(rd, f->first, frontier, interior, newFrontier, newFreshFrontier, modified);
                if(newFrontier.find(f->first)==newFrontier.end()) {
                  newFrontier[f->first] = emptyRDSet;
                  // Do not add f->first onto newFreshFrontier since we've now abandoned the search
                  // for defs that reach this use
                  SIGHT_VERB(dbg << "Stopping search for defs reaching "<<f->first->str()<<endl, 1, dynMonDL)
                    } else {
                  SIGHT_VERB(dbg << "Tried to stop search for defs reaching "<<f->first->str()<<", but another search path discovered"<<endl, 1, dynMonDL) }
              }
            }
          }
        }
      }
//      BOOST_FOREACH(const SSAMemLocObjectPtr& d, f->second) {
//        // For each frontier def look for the definitions that reach it
//        //if(d->getAccess() == SSAMemLocObject::def) {
//          scope s2(sight::txt()<<"def "<<d->str());
//          const set<SSAMemLocObjectPtr>& reachingUses = ssa->getUses(d->getLoc());
//          BOOST_FOREACH(const SSAMemLocObjectPtr& u, reachingUses) {
//            scope s3(sight::txt()<<"use "<<u->str());
//            const set<SSAMemLocObjectPtr>& reachingDefs = ssa->getDefs(u);
//
//            BOOST_FOREACH(const SSAMemLocObjectPtr& rd, reachingDefs) {
//              SgType* type = getType(rd->getSgNode());
//              ROSE_ASSERT(type);
//              dbg << "ScalarType="<<(type->isIntegerType() && type->isFloatType())<<endl;
//
//              if(type->isIntegerType() || type->isFloatType()) {
//                // If this is a scalar def, add it to the frontier
//                if(rd->getAccess()==SSAMemLocObject::def) {
//
//                    continue;
//                  }
//                // If this is a scalar phiDef, add its use->phiDef relation to the frontier
//                } else if(rd->getAccess()==SSAMemLocObject::phiDef) {
//                  if(d->getAccess()==SSAMemLocObject::phiDef) {
//                    updateFrontierPhiDef(rd, f->first, d, frontier, interior, newFrontier, modified);
//                }
//
//              // If this is a
//            // First determine whether the definitions that reach this use are interesting
//            // enough to track or whether we should be tracking the use itself
//            //bool anyPhiDefsRD=false; // Whether any of the reaching defs are phi-defs
//            //bool anyNonScalarRD=false; // Whether any of the reaching defs are non-scalars
//            bool allPhiDefsRD=true;   // Whether all of the reaching defs are phi-defs
//            bool allNonScalarRD=true; // Whether all of the reaching defs are non-scalars
//            BOOST_FOREACH(const SSAMemLocObjectPtr& rd, reachingDefs) {
//              dbg << "reaching def"<<rd->str()<<endl;
//              /*if(rd->getAccess()==SSAMemLocObject::phiDef) {
//                anyPhiDefsRD=false;
//                break;
//              }
//
//              SgType* type = getType(rd->getSgNode());
//              ROSE_ASSERT(type);
//              dbg << "non-ScalarType="<<(!type->isIntegerType() && !type->isFloatType())<<endl;
//              if(!type->isIntegerType() && !type->isFloatType()) {
//                anyNonScalarRD=true;
//                break;
//                //dbg << "type="<<SgNode2Str(type)<<endl;
//              }*/
//              if(rd->getAccess()==SSAMemLocObject::def) {
//                SgType* type = getType(rd->getSgNode());
//                ROSE_ASSERT(type);
//                dbg << "non-ScalarType="<<(!type->isIntegerType() && !type->isFloatType())<<endl;
//                if(type->isIntegerType() || type->isFloatType()) {
//
//                }
//                //dbg << "type="<<SgNode2Str(type)<<endl;
//              }
//            }
//
//            dbg << "anyPhiDefsRD="<<anyPhiDefsRD<<", anyNonScalarRD="<<anyNonScalarRD<<endl;
//            // If this use has any non-scalar or indefinite reaching defs, add it to the frontier
//            if(anyPhiDefsRD || anyNonScalarRD) {
//              updateFrontier(u, d, frontier, interior, newFrontier, modified);
//
//            // Otherwise, add the reaching defs
//            } else {
//              BOOST_FOREACH(const SSAMemLocObjectPtr& rd, reachingDefs) {
//                updateFrontier(rd, d, frontier, interior, newFrontier, modified);
//              }
//            }
//          }
//        }
//      //}
/*      // For each frontier phiDef look for the definitions that reach it
      } else if(d->getAccess() == SSAMemLocObject::phiDef) {
        const map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > defs = ssa->getDefsUsesAtPhiNode(d->getLoc());
        dbg << "#phiDefs="<<defs.size()<<endl;
        for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::const_iterator group=defs.begin(); group!=defs.end(); ++group) {
          for(set<SSAMemLocObjectPtr>::const_iterator rd=group->second.begin(); rd!=group->second.end(); ++rd) {
            updateFrontier(*rd, d, frontier, interior, newFrontier, modified);
          }
        }
      }*/
    }}

    frontier = newFrontier;
    freshFrontier = newFreshFrontier;
  } while(freshFrontier.size()>0);

  // Place the starting def back on the frontier, since we want to use the last
  // value of the starting as input and normally it is likely to have migrated
  // to the interior.
//  frontier.insert(startingDef);

  SIGHT_VERB_IF(1, dynMonDL)
#ifndef DISABLE_SIGHT
  {scope s("frontier");
    for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator f=frontier.begin(); f!=frontier.end(); ++f) {
      scope s2(sight::txt()<<"Use "<<f->first->str());
      BOOST_FOREACH(const SSAMemLocObjectPtr& d, f->second) {
        dbg << d->str()<<endl;
      }
    }
  }

  {scope s("interior");
    BOOST_FOREACH(const SSAMemLocObjectPtr& d, interior) {
      dbg << d->str()<<endl;
    }
  }
#endif
  SIGHT_VERB_FI()

  set<SSAMemLocObjectPtr> finalFrontier;
  for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator f=frontier.begin(); f!=frontier.end(); ++f) {
    if(f->first->getAccess()==SSAMemLocObject::def)
      finalFrontier.insert(f->first);
    else if(f->first->getAccess()==SSAMemLocObject::use) {
      if(interior.find(f->first) == interior.end())
        finalFrontier.insert(f->first);
    }
  }

  SIGHT_VERB_IF(1, dynMonDL)
#ifndef DISABLE_SIGHT
  {scope s("finalFrontier");
    BOOST_FOREACH(const SSAMemLocObjectPtr& d, finalFrontier) {
      dbg << d->str()<<endl;
    }
  }
#endif
  SIGHT_VERB_FI()

  return finalFrontier;
}

bool isInteresting(SgNode* n) {
  if((!isSgAssignOp(n) && !isSgCompoundAssignOp(n)) ||
      !isSgExpression(n)->get_type()->isFloatType()) return false;

  SgStatement* enclosingStmt=NULL;
  if(isSgStatement(n)) enclosingStmt = isSgStatement(n);
  else                 enclosingStmt = getEnclosingStatement(n);

  SgStatement *priorStmt = getPreviousStatement(enclosingStmt);
  if(SgPragmaDeclaration* pragma = isSgPragmaDeclaration(priorStmt)) {
    if(pragma->get_pragma()->get_pragma()=="dynamicMonitor")
      return true;
  }

  return false;
}

void DynamicMonitor::runAnalysis() {
  set<PartPtr> startStates = getComposer()->GetStartAStates(NULL);
  std::map<PartPtr, int> partID;
  set<PartPtr> visited;

  map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > rd2targets;
  map<SSAMemLocObjectPtr, int> rdIdx;
  //map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > target2rds;
  map<SSAMemLocObjectPtr, set<SgNode*> > target2rds;
  map<SSAMemLocObjectPtr, int> targetIdx;
  map<SSAMemLocObjectPtr, SgExpression* > target2anchor;

  ssa = getComposer()->GetSSAGraph(this);

  for(fw_partEdgeIterator state(startStates, /*incrementalGraph*/ false); !state.isEnd(); state++) {
    PartPtr part = state.getPart();
    set<CFGNode> cfgNodes = part->CFGNodes();
    ROSE_ASSERT(cfgNodes.size()==1);
    CFGNode cn = *cfgNodes.begin();

    // Skip boring parts
    if(!isInteresting(cn.getNode())) continue;
#ifndef DISABLE_SIGHT
    scope s(part->str());
#endif
    const set<SSAMemLocObjectPtr>& defs = ssa->getDefs(part);
    SIGHT_VERB(dbg << "  #defs="<<defs.size()<<endl, 1, dynMonDL)
    BOOST_FOREACH(const SSAMemLocObjectPtr& d, defs) {
      // Skip boring defs
      if(isSgAssignOp(d->getBase()) || isSgCompoundAssignOp(d->getBase())) continue;

      SIGHT_VERB(dbg <<"    def="<<d->str()<<endl, 1, dynMonDL)
      // If this is a regular def, not a phi definition
      if(d->getAccess() == SSAMemLocObject::def) {
        // Collect all the defs that transitively and acyclically reach this def
        //reachingDefs.insert(d);
        SIGHT_VERB(dbg << "    #uses="<<ssa->getUses(part).size()<<endl, 1, dynMonDL)
        set<SSAMemLocObjectPtr> reachingDefs = collectReachingDefs(d);
        //collectReachingDefs(ssa->getUses(part), reachingDefs);

        ROSE_ASSERT(targetIdx.find(d) == targetIdx.end());
        targetIdx[d] = targetIdx.size();
        //cout << "targetIdx["<<d->str()<<"]="<<targetIdx[d]<<endl;

        ROSE_ASSERT(isSgExpression(cn.getNode()));
        target2anchor[d] = getValidInsertTarget(isSgExpression(cn.getNode()));

        SIGHT_VERB(dbg << "At expression "<<d->str()<<": "<<(d->getBase()? SgNode2Str(d->getBase()): "NULL")<<endl, 1, dynMonDL)
        //cout << "At expression "<<d->str()<<": "<<(d->getBase()? SgNode2Str(d->getBase()): "NULL")<<endl;
        BOOST_FOREACH(const SSAMemLocObjectPtr& rd, reachingDefs) {
          if(!isSgExpression(rd->getBase())) continue;
          if(isSgGlobal(getScope(isSgExpression(rd->getBase())))) continue;
          // Filter out constant values as reaching defs since they're not informative
          if(isSgValueExp(rd->getBase())) continue;

          SIGHT_VERB(dbg << "    "<<rd->str()<<": "<<(rd->getBase()? SgNode2Str(rd->getBase()): "NULL")<<endl, 1, dynMonDL)
          //cout << "    "<<rd->str()<<": "<<(rd->getBase()? SgNode2Str(rd->getBase()): "NULL")<<endl;

          if(rdIdx.find(rd) == rdIdx.end()) rdIdx[rd] = rdIdx.size();
          //cout << "rdIdx["<<rd->str()<<"]="<<rdIdx[rd]<<endl;

          rd2targets[rd].insert(d);
          //target2rds[d].insert(rd);
          target2rds[d].insert(rd->getBase());
        }
      }
    }
  }

  InsertInitFinTransform initfinT;
  initfinT.traverseInputFiles (getProject(), preorder);

  // Insert declarations of the data structures where observations will be placed
  { SIGHT_VERB_DECL(scope, ("Prepending declareTarget()"), 1, dynMonDL)
  SIGHT_VERB(dbg << "#targetIdx="<<targetIdx.size()<<endl, 1, dynMonDL)
  for(map<SSAMemLocObjectPtr, int>::iterator t=targetIdx.begin(); t!=targetIdx.end(); ++t) {
    SIGHT_VERB_DECL(scope, ("t"), 1, dynMonDL)
    SIGHT_VERB(dbg << t->first->str()<<endl, 1, dynMonDL)
    SIGHT_VERB(dbg << "t->first->getBase()="<<t->first->getBase()<<"="<<SgNode2Str(t->first->getBase())<<endl, 1, dynMonDL)
    vector< SgExpression *> args;
    args.push_back(buildIntVal(t->second));
    args.push_back(buildStringVal(SgNode2Str(target2anchor[t->first])));

//    prependStatement(
//        buildExprStatement(
//          buildFunctionCallExp(SgName("declareTarget"),
//              buildVoidType(),
//              buildExprListExp(args),
//              //getGlobalScope(t->first->getBase())
//              getScope(t->first->getBase())
//              /*getFirstGlobalScope(getProject())*/)),
//        //getGlobalScope(t->first->getBase())
//        (SgScopeStatement*)getEnclosingProcedure(t->first->getBase())->get_body()
//        //(SgScopeStatement*)getEnclosingStatement(getEnclosingProcedure(t->first->getBase()))
//        );
//        //getScope(t->first->getBase())
//        /*getFirstGlobalScope(getProject())*///);

    SgExpression* call = buildFunctionCallExp(SgName("declareTarget"),
        buildVoidType(),
        buildExprListExp(args),
        //getGlobalScope(t->first->getBase())
        getScope(t->first->getBase()));
    attachArbitraryText(getGlobalScope(t->first->getBase()), call->unparseToString(), /*position*/ PreprocessingInfo::before);

    initfinT.appendInitStmtToMain(buildExprStatement(
        buildFunctionCallExp(SgName("newTarget"),
            buildVoidType(),
            buildExprListExp(args),
            //getGlobalScope(t->first->getBase())
            getScope(t->first->getBase()))));

    initfinT.prependFinStmtToMain(buildExprStatement(
        buildFunctionCallExp(SgName("deleteTarget"),
            buildVoidType(),
            buildExprListExp(args),
            //getGlobalScope(t->first->getBase())
            getScope(t->first->getBase()))));
  }}

  // Insert functions to track the relationship between reachind defs and the defs they feed into
  { SIGHT_VERB_DECL(scope, ("Inserting trackReachingDef()"), 1, dynMonDL)
  SIGHT_VERB(dbg << "#rdIdx="<<rdIdx.size()<<endl, 1, dynMonDL)
  map<SSAMemLocObjectPtr, SgVariableDeclaration*> defVarDecl;
  map<SSAMemLocObjectPtr, string> defVarName;
  //map<SSAMemLocObjectPtr, list<SgStatement*> > reachingDefTrackInsertions;
  //map<SSAMemLocObjectPtr, list<SgExpression*> > reachingDefTrackInsertions;
  map<SgExpression*, list<string > > trackInsertionsFName;
  map<SgExpression*, list<vector<SgExpression *> > > trackInsertionsArgs;
  map<SgExpression*, list<SgScopeStatement*> > trackInsertionsScope;

  for(map<SSAMemLocObjectPtr, int>::iterator rd=rdIdx.begin(); rd!=rdIdx.end(); ++rd) {
    SIGHT_VERB_DECL(scope, (sight::txt()<<"RD="<<rd->first->str()), 1, dynMonDL)
    //cout<<"RD="<<rd->first->str()<<endl;
    // Prepare the current reaching def for new tracking statements to be inserted
    // immediately after the SgNode that denotes its reference. Make sure to do this
    // exactly once.
/*    bool insertTargetIdentified=false;
    if(defVarDecl.find(rd->first) == defVarDecl.end()) {
      if(SgInitializedName* rdIName = isSgInitializedName(rd->first->getBase())) {
        if(isSgFunctionParameterList(rdIName->get_declaration())) {

        } else {
          defVarDecl[rd->first] = isSgVariableDeclaration(rdIName->get_declaration());
          defVarName[rd->first] = rdIName->get_name().getString();
          ROSE_ASSERT(defVarDecl[rd->first]);
          insertTargetIdentified=true;
        }
      } else {
        SIGHT_VERB(dbg << "Splitting expression (GLOBAL="<<(isSgGlobal(getScope((SgExpression*)rd->first->getBase())))<<") = "<<SgNode2Str((SgExpression*)rd->first->getBase())<<endl, 1, dynMonDL)

    cout << "rd->first->getBase()="<<SgNode2Str(rd->first->getBase())<<endl;
        cout << "rd->first->getBase()->get_parent()="<<SgNode2Str(rd->first->getBase()->get_parent())<<endl;

        if(isSgGlobal(getScope(rd->first->getBase())) ||
           isSgAssignInitializer(rd->first->getBase()) || isSgInitializedName(rd->first->getBase())) {}
        / *if(isSgGlobal(getScope((SgExpression*)rd->first->getBase()))) {
          SgInitializedName* iname=NULL;
          if(SgAssignInitializer* init = isSgAssignInitializer(rd->first->getBase())) {
            if(isSgInitializedName(init->get_parent()))
              iname = isSgInitializedName(init->get_parent());
          } else if(SgAssignInitializer* init = isSgAssignInitializer(rd->first->getBase()->get_parent())) {
            if(isSgInitializedName(init->get_parent()))
              iname = isSgInitializedName(init->get_parent());
          } else if(isSgInitializedName(init->get_parent())) {
            iname = isSgInitializedName(init->get_parent());
          }
          ROSE_ASSERT(iname);

          defVarDecl[rd->first] = isSgVariableDeclaration(iname->get_declaration());
          ROSE_ASSERT(defVarDecl[rd->first]);
        } else if(isSgInitializedName(rd->first->getBase()) &&
                  isSgGlobal(getScope(isSgInitializedName(rd->first->getBase())))) {
          SgInitializedName* iname = isSgInitializedName(rd->first->getBase());
          defVarDecl[rd->first] = isSgVariableDeclaration(iname->get_declaration());
          ROSE_ASSERT(defVarDecl[rd->first]);
        } * /
        //else if(isSgFor)

        else {
          defVarName[rd->first] = rd->first->getBase()->unparseToString();

          SgAssignInitializer *rdInit = splitExpression ((SgExpression*)rd->first->getBase());
          SgInitializedName* newRDName = isSgInitializedName(rdInit->get_parent());
          ROSE_ASSERT(newRDName);
          defVarDecl[rd->first] = isSgVariableDeclaration(newRDName->get_declaration());
          ROSE_ASSERT(defVarDecl[rd->first]);
          insertTargetIdentified=true;
        }
      }
    }

    if(insertTargetIdentified) {*/
    { SIGHT_VERB_DECL(scope, (sight::txt()<< "rd2targets["<<rd->first->str()), 1, dynMonDL)
    // For each target that this def reaches, add a call to track this reaching def in the
    // target's data structure
    for(set<SSAMemLocObjectPtr>::iterator t=rd2targets[rd->first].begin(); t!=rd2targets[rd->first].end(); ++t) {
      if(isSgExpression(rd->first->getBase())) {
        /*dbg << "    "<<t->str()<<endl;
        dbg << "defVarDecl[rd->first]="<<SgNode2Str(defVarDecl[rd->first])<<endl;
        dbg << "defVarName[rd->first]="<<defVarName[rd->first]<<endl;*/
        vector< SgExpression *> args;
        args.push_back(buildIntVal(rd->second));
        args.push_back(buildIntVal(targetIdx[*t]));
        //args.push_back(buildVarRefExp(defVarDecl[rd->first]));
        //args.push_back(buildStringVal(defVarName[rd->first]));
        args.push_back(buildStringVal(rd->first->getBase()->unparseToString()));

  /*      reachingDefTrackInsertions[rd->first].push_back(
                        //buildExprStatement(
                          buildFunctionCallExp(SgName("trackReachingDef"),
                              buildVoidType(),
                              buildExprListExp(args),
                              //getScope(defVarDecl[rd->first])
                              getScope(rd->first->getBase())
                              )//)
                        );*/
        SgExpression* anchor = getValidInsertTarget(isSgExpression(rd->first->getBase()));
        trackInsertionsFName[anchor].push_back("trackReachingDef");
        trackInsertionsArgs [anchor].push_back(args);
        trackInsertionsScope[anchor].push_back(getScope(rd->first->getBase()));
      }
    } } //}
  }

//  //for(map<SSAMemLocObjectPtr, list<SgStatement*> >::iterator ins=reachingDefTrackInsertions.begin();
//  //for(map<SSAMemLocObjectPtr, list<SgExpression*> >::iterator ins=reachingDefTrackInsertions.begin();
////  ins!=reachingDefTrackInsertions.end(); ++ins) {
//  for(map<SSAMemLocObjectPtr, list<vector<SgExpression *> > >::iterator ins=reachingDefTrackInsertionsArgs.begin(); ins!=reachingDefTrackInsertionsArgs.end(); ++ins) {
//    //BOOST_FOREACH(SgStatement* stmt, ins->second) {
//    //  insertStatement(defVarDecl[ins->first], stmt, /*insertBefore*/ false);
//    //}
//    if(isSgExpression(ins->first->getBase())) {
//      /*cout << "insert "<<SgNode2Str(*ins->second.begin())<<endl;
//      cout << "after "<<SgNode2Str(isSgExpression(ins->first->getBase()))<<endl;*/
//      //insertAfterUsingCommaOp(*ins->second.begin(), isSgExpression(ins->first->getBase()));
///*      insertFuncCallAfterUsingCommaOp("trackReachingDef", buildVoidType(), *ins->second.begin(),
//          *reachingDefTrackInsertionsScope[ins->first].begin(), isSgExpression(ins->first->getBase()));*/
//    }
//  }}


  { SIGHT_VERB_DECL(scope, ("Inserting trackTargetDef()"), 1, dynMonDL)
  // Insert calls to track results of targets
  for(map<SSAMemLocObjectPtr, int>::iterator t=targetIdx.begin(); t!=targetIdx.end(); ++t) {
    SIGHT_VERB_DECL(scope, ("t"), 1, dynMonDL)
    SIGHT_VERB(dbg << "t->first->getBase()="<<t->first->getBase()<<"="<<SgNode2Str(t->first->getBase())<<endl, 1, dynMonDL)

    /*SgAssignInitializer *tgtInit = splitExpression ((SgExpression*)t->first->getBase(), string(sight::txt()<<"targetDef"<<t->second));
    SgInitializedName* newTgtName = isSgInitializedName(tgtInit->get_parent());
    ROSE_ASSERT(newTgtName);
    SgVariableDeclaration* newTgtDecl = isSgVariableDeclaration(newTgtName->get_declaration());
    ROSE_ASSERT(newTgtDecl);
*/
    vector< SgExpression *> args;
    ROSE_ASSERT(isSgExpression(t->first->getBase()));
    args.push_back(buildIntVal(t->second));
    args.push_back(buildIntVal(target2rds[t->first].size()));
    //args.push_back(/*buildVarRefExp(newTgtDecl)*/ isSgExpression(deepCopyNode(t->first->getBase())));

    /*insertAfterUsingCommaOp(
        buildFunctionCallExp(SgName("trackTargetDef"),
                             buildVoidType(),
                             buildExprListExp(args),
                             getGlobalScope(t->first->getBase())),
        target2anchor[t->first]);*/

    trackInsertionsFName[target2anchor[t->first]].push_back("trackTargetDef");
    trackInsertionsArgs [target2anchor[t->first]].push_back(args);
    trackInsertionsScope[target2anchor[t->first]].push_back(getScope(t->first->getBase()));
  } }

  for(map<SgExpression*, list<vector<SgExpression *> > >::iterator ins=trackInsertionsArgs.begin();
      ins!=trackInsertionsArgs.end(); ++ins) {
      /*cout << "insert "<<SgNode2Str(*ins->second.begin())<<endl;
      cout << "after "<<SgNode2Str(isSgExpression(ins->first))<<endl;*/
      //cout << "      "<<ins->first->str()<<endl;
      insertFuncCallAfterUsingCommaOp(trackInsertionsFName[ins->first],
                                      buildVoidType(),
                                      ins->second,
                                      trackInsertionsScope[ins->first],
                                      ins->first);
    }}


  prependStatementsToScope(initfinT.funcInit);
  appendStatementsToScope (initfinT.funcFin);
}

}; // namespace fuse
