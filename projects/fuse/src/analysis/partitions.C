#include "sage3basic.h"
using namespace std;

#include "partitions.h"
#include "abstract_object.h"
#include "compose.h"
#include <boostGraphCFG.h>
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

#ifndef DISABLE_SIGHT
#include "sight.h"
using namespace sight;
#endif

using namespace boost;

namespace fuse {

#ifndef DISABLE_SIGHT
#define partitionsDebugLevel 0
#endif

/* #########################
   ##### Remap Functor #####
   ######################### */

MLMapping::MLMapping(MemLocObjectPtr from, MemLocObjectPtr to, bool replaceMapping) :
  from(from), to(to), replaceMapping(replaceMapping) { }

bool MLMapping::operator <(const MLMapping& that) const {
  return (from <  that.from) ||
         (from == that.from && to <  that.to) ||
         (from == that.from && to == that.to  && replaceMapping < that.replaceMapping);
}

string MLMapping::str(string indent) const {
  ostringstream oss;
  oss << "[MLMapping: replaceMapping="<<replaceMapping<<endl;
  oss << indent << "    from="<<from->str()<<endl;
  oss << indent << "    to="<<to->str()<<"]";
  return oss.str();
}

MLRemapper::MLRemapper() {}

MLRemapper::MLRemapper(const MLRemapper& that) :
      pedge(that.pedge), fwML2ML(that.fwML2ML), bwML2ML(that.bwML2ML), initialized(that.initialized)
{}

// This initialization method must be called by the MLRemapper's host PartEdge before it calls
//    forwardRemapML() or backwardRemapML(). The MLRemapper's full initalization is delayed in this way
//    because the pedge argument is a shared_ptr created from the host PartEdge's this. shared_ptrs
//    cannot be created from this in the object's constructor, thus forcing us to delay initialization
//    until the host PartEdge is fully initialized.
// It is legal to call this function multiple times but in each call the field pedge.
void MLRemapper::init(const PartEdgePtr pedge, ComposedAnalysis* client)
{
  // Initialize pedge, fwML2ML and bwML2ML only once
  if(initialized.find(client) != initialized.end()) return;
  initialized.insert(client);

  //scope reg(txt()<<"MLRemapper::init() pedge=" << pedge.get()->str()<<" client="<<client->str(), scope::medium);
  // Record the given PartEdge and ensure that it is consistent
  if(this->pedge) assert(this->pedge == pedge);
  else            this->pedge = pedge;

  if(pedge->source()) {
    set<CFGNode> srcNodes = pedge->source()->CFGNodes();
    for(set<CFGNode>::iterator n=srcNodes.begin(); n!=srcNodes.end(); n++) {
      // If this is a non-void return
      if(isSgReturnStmt(n->getNode()) && isSgReturnStmt(n->getNode())->get_expression()) {
        set<MLMapping> ml2ml;
        // Map the return value to a FunctionResultML
        ml2ml.insert(
            MLMapping(client->getComposer()->Expr2MemLoc(isSgReturnStmt(n->getNode())->get_expression(), pedge->source()->inEdgeFromAny(), client),
                      boost::make_shared<FuncResultMemLocObject>(Function(SageInterface::getEnclosingFunctionDeclaration(n->getNode()))),
                      //boost::make_shared<CombinedMemLocObject>(Union, client, boost::make_shared<FuncResultMemLocObject>(Function(SageInterface::getEnclosingFunctionDeclaration(n->getNode())))),
                      false));
        fwML2ML[client].insert(ml2ml);
      // Else, if this is an edge out of a function's call (must be to a SgFunctionParameterList)
      } else if(isSgFunctionCallExp(n->getNode()) && n->getIndex()==2) {
        //dbg << "Function call out"<<endl;
        // Map the arguments of a function call <-> the function's parameters
        set<MLMapping> ml2ml_fw;
        setArgParamMap(pedge, isSgFunctionCallExp(n->getNode()), ml2ml_fw,
                       client->getComposer(),
                       client, true);
        fwML2ML[client].insert(ml2ml_fw);

        set<MLMapping> ml2ml_bw;
        setArgParamMap(pedge, isSgFunctionCallExp(n->getNode()), ml2ml_bw,
                       client->getComposer(),
                       client, false);
        bwML2ML[client].insert(invertArg2ParamMap(ml2ml_bw));
      }
    }
  }

  if(pedge->target()) {
    set<CFGNode> tgtNodes = pedge->target()->CFGNodes();
    for(set<CFGNode>::iterator n=tgtNodes.begin(); n!=tgtNodes.end(); n++) {
      // If this is an edge into a function's call (must be from a SgFuncDefinition)
      if(isSgFunctionCallExp(n->getNode()) && n->getIndex()==3) {
        // Mapping the function parameters that are passed by reference <-> the arguments in the function call, AND
        //         the function's return MemLocObject <-> the call's return MemLocObject
        set<MLMapping> ml2ml;
        setArgByRef2ParamMap(pedge, isSgFunctionCallExp(n->getNode()), ml2ml,
                             client->getComposer(),
                             client);
        bwML2ML[client].insert(ml2ml);
        fwML2ML[client].insert(invertArg2ParamMap(ml2ml));
      }
    }
  }
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "MLRemapper::init"<<endl<<str() <<endl, 2, partitionsDebugLevel)
#endif
}

// Given a lattice returns a freshly-allocated Lattice object that points to Lattice remapped in the forward direction
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    some MemLocs are in scope on one side of Part, while others are in scope on the other side.
//    fromPEdge is the edge from which control is passing and the current PartEdge (same as the PartEdge of
//    the Lattice) is the one to which control is passing.
Lattice* MLRemapper::forwardRemapML(Lattice* lat, PartEdgePtr fromPEdge, ComposedAnalysis* client) const {
  assert(initialized.find(client) != initialized.end());
  assert(pedge == lat->getPartEdge());

  /*scope reg("MLRemapper::forwardRemapML()", scope::medium);
  dbg << "client="<<client->str()<<endl;
  dbg << "#fwML2ML="<<fwML2ML.size()<<" #initialized="<<initialized.size()<<" pedge="<<pedge.get()->str()<<endl;*/

  map<ComposedAnalysis*, set<set<MLMapping> > >::const_iterator clientML2ML = fwML2ML.find(client);
  if(clientML2ML == fwML2ML.end()) return NULL;

  // Iterate through all the possible remappings, computing the Lattices that they produce
  Lattice* ret=NULL;
  for(set<set<MLMapping> >::const_iterator i=clientML2ML->second.begin(); i!=clientML2ML->second.end(); i++) {
    //dbg << "#i="<<i->size()<<endl;
    if(ret==NULL) ret = lat->remapML(*i, fromPEdge);
    else {
      // Merge the lattice that results from the current remapping into ret, deleting the intermediate Lattice object
      Lattice* tmp = lat->remapML(*i, fromPEdge);
      ret->meetUpdate(tmp);
      delete tmp;
    }
  }
  return ret;
}

// Given a lattice returns a freshly-allocated Lattice object that points to Lattice remapped in the forward direction
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    some MemLocs are in scope on one side of Part, while others are in scope on the other side.
//    fromPEdge is the edge from which control is passing and the current PartEdge (same as the PartEdge of
//    the Lattice) is the one to which control is passing.
Lattice* MLRemapper::backwardRemapML(Lattice* lat, PartEdgePtr fromPEdge, ComposedAnalysis* client) const {
  assert(initialized.find(client) != initialized.end());
  assert(pedge == lat->getPartEdge());

 // SIGHT_VERB(dbg << "MLRemapper::backwardRemapML() #bwML2ML="<<bwML2ML[client].size()<<" pedge="<<pedge.get()->str()<<endl, 1, partitionsDebugLevel)

  map<ComposedAnalysis*, set<set<MLMapping> > >::const_iterator clientML2ML = bwML2ML.find(client);
  if(clientML2ML == bwML2ML.end()) return NULL;

  // Iterate through all the possible remappings, computing the Lattices that they produce
  Lattice* ret=NULL;
  for(set<set<MLMapping> >::const_iterator i=clientML2ML->second.begin(); i!=clientML2ML->second.end(); i++) {
    if(ret==NULL) ret = lat->remapML(*i, fromPEdge);
    else {
      // Merge the lattice that results from the current remapping into ret, deleting the intermediate Lattice object
      Lattice* tmp = lat->remapML(*i, fromPEdge);
      ret->meetUpdate(tmp);
      delete tmp;
    }
  }
  return ret;
}

// Returns whether if the two given ml2ml maps are equal.
bool MLRemapper::equalMaps(const set<set<MLMapping> >& ml2mlA,
                           const set<set<MLMapping> >& ml2mlB)
{
  if(ml2mlA.size() != ml2mlB.size()) return false;

  set<set<MLMapping> >::const_iterator iA=ml2mlA.begin();
  set<set<MLMapping> >::const_iterator iB=ml2mlB.begin();
  for(; iA!=ml2mlA.end() && iB!=ml2mlB.end(); iA++, iB++) {
    if(iA->size() != iB->size()) return false;
    std::set<MLMapping>::const_iterator jA=iA->begin();
    std::set<MLMapping>::const_iterator jB=iB->begin();
    for(; jA!=iA->end() && jB!=iB->end(); jA++, jB++)
      if(jA->from != jB->from ||
         jA->to != jB->to ||
         jA->replaceMapping != jB->replaceMapping)
        return false;
  }

  // If both sets have the same mappings, they're equal
  return true;
}

bool MLRemapper::operator==(const MLRemapper& that) const
{
  if(fwML2ML.size() != that.fwML2ML.size()) return false;
  std::map<ComposedAnalysis*, std::set<std::set<MLMapping > > >::const_iterator
    fwThisIt = fwML2ML.begin(),
    bwThisIt = bwML2ML.begin(),
    fwThatIt = that.fwML2ML.begin(),
    bwThatIt = that.bwML2ML.begin();

  for(; fwThisIt != fwML2ML.end(); fwThisIt++, bwThisIt++, fwThatIt++, bwThatIt++) {
    if(!equalMaps(fwThisIt->second, fwThatIt->second) ||
        equalMaps(bwThisIt->second, bwThatIt->second))
      return false;
  }
  return true;
}

// String representation of object
std::string MLRemapper::map2Str(const map<ComposedAnalysis*, set<set<MLMapping> > >& ml2ml, std::string indent) const {
  ostringstream oss;

  for(map<ComposedAnalysis*, set<set<MLMapping> > >::const_iterator client=ml2ml.begin(); client!=ml2ml.end(); client++) {
    if(client!=ml2ml.begin()) oss << indent;
    oss << client->first->str() << ": "<<endl;

    for(set<set<MLMapping> >::iterator i=client->second.begin(); i!=client->second.end(); i++) {
    for(set<MLMapping>::iterator j=i->begin(); j!=i->end(); j++){
      oss << indent << "    " << (j->from? j->from->str(indent+"        "): "NULL") << " -&gt; " << endl;
      oss << indent << "        " << (j->to? j->to->str(indent+"        "): "NULL") << endl;
      oss << indent << "        replaceMapping=" <<j->replaceMapping<<endl;
    } }
  }

  return oss.str();
}

std::string MLRemapper::str(std::string indent) const
{
  ostringstream oss;

  oss << "[MLRemapper: fwML2ML="<<endl<<
                       map2Str(fwML2ML, indent+"                 ")<<endl<<
                       indent << "             bwML2ML="<<endl<<
                       map2Str(bwML2ML, indent+"                 ");
  return oss.str();
}

// Given a function call, sets argParamMap to map all arguments of this function call to their
// corresponding parameters.
// Supports caller->callee transfers for forwards analyses and callee->caller transfers for backwards analyses
// (direction specified by the fw flag).
void setArgParamMap(PartEdgePtr callEdge, SgFunctionCallExp* call,
                    std::set<MLMapping>& argParamMap,
                    Composer* composer, ComposedAnalysis* client,
                    bool fw)
{
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("setArgParamMap", scope::medium), 1, partitionsDebugLevel)
#endif
  Function func(call);
#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(1, partitionsDebugLevel)
    dbg << "call="<<SgNode2Str(call)<<endl;
    dbg << "callEdge="<<callEdge->str()<<endl;
  SIGHT_VERB_FI()
#endif

  PartPtr callPart = callEdge->source();
  PartPtr funcStartPart = callEdge->target();

  // Part that corresponds to the function, which for now is set to be the start of its definition
  //PartPtr funcStartPart = client->getComposer()->GetFunctionStartPart(func, client);
  /*
  dbg << "call args="<<call->get_args()<<endl;
  dbg << "func params="<<(func.get_params()? SgNode2Str(func.get_params()): "NULL")<<endl;
  dbg << "func args="<<func.get_args()<<endl;
  dbg << "func declaration="<<(func.get_declaration()? SgNode2Str(func.get_declaration()): "NULL")<<endl;
  dbg << "func definition="<<(func.get_definition()? SgNode2Str(func.get_definition()): "NULL")<<endl;

  std::set<CFGNode> funcNodes = callEdge->target()->CFGNodes();
  { scope s(txt()<<"nodes of "<<callEdge->target()->str());
  for(std::set<CFGNode>::iterator f=funcNodes.begin(); f!=funcNodes.end(); f++) {
    dbg << SgNode2Str(f->getNode())<<endl;
    if(isSgFunctionParameterList(f->getNode())) {
      Function func(SageInterface::getEnclosingFunctionDefinition(f->getNode()));
      dbg << "func params="<<(func.get_params()? SgNode2Str(func.get_params()): "NULL")<<endl;
      dbg << "func args="<<func.get_args()<<endl;
      dbg << "func declaration="<<(func.get_declaration()? SgNode2Str(func.get_declaration()): "NULL")<<endl;
      dbg << "func definition="<<(func.get_definition()? SgNode2Str(func.get_definition()): "NULL")<<endl;
    }
  }}

  */

  SgExpressionPtrList args = call->get_args()->get_expressions();

  std::set<CFGNode> funcNodes = callEdge->target()->CFGNodes();
  assert(funcNodes.size()==1);
  assert(isSgFunctionParameterList(funcNodes.begin()->getNode()));
  const SgInitializedNamePtrList& params = isSgFunctionParameterList(funcNodes.begin()->getNode())->get_args();
  //SgInitializedNamePtrList params = funcArgToParamByRef(call);
  //SgInitializedNamePtrList* params = func.get_args();

  if(args.size()!=params.size()) {
    cout << "callPart="<<callPart->str()<<endl;
    cout << "call="<<SgNode2Str(call)<<endl;
    cout << "args="<<SgNode2Str(call->get_args())<<endl;
    for(SgExpressionPtrList::iterator a=args.begin(); a!=args.end(); a++)
      cout << "    "<<SgNode2Str(*a)<<endl;
    cout << "params="<<SgNode2Str(funcNodes.begin()->getNode())<<"="<<endl;
    for(SgInitializedNamePtrList::const_iterator p=params.begin(); p!=params.end(); p++)
      cout << "    "<<SgNode2Str(*p)<<", type="<<SgNode2Str((*p)->get_type())<<endl;
  }
  assert(args.size() == params.size());

  //dbg << "setArgParamMap() #args="<<args.size()<<" #params="<<params->size()<<"\n";
  // the state of the callee's variables at the call site
  SgExpressionPtrList::iterator itA;
  SgInitializedNamePtrList::const_iterator itP;
  for(itA = args.begin(), itP = params.begin();
      itA!=args.end() && itP!=params.end();
      itA++, itP++)
  {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, ("iter", scope::low), 1, partitionsDebugLevel)
    SIGHT_VERB_IF(1, partitionsDebugLevel)
      dbg << "itA="<<SgNode2Str(*itA)<<endl;
      dbg << "itP="<<SgNode2Str(*itP)<<endl;
    SIGHT_VERB_FI()
#endif
    SgType* typeParam = (*itP)->get_type();

    // Skip "..." types, which are used to specify VarArgs.
    // NEED TO SUPPORT VAR ARGS MORE EFECTIVELY IN THE FUTURE
    if(isSgTypeEllipse(typeParam)) continue;
    // Skip variables with no names (possible in function declarations that have no definitions).
    // NEED BETTER SUPPORT FOR FUNCTIONS WITH NO BODIES
    //if((*itP)->get_name().getString() == "") continue;

    //cout << "itA="<<cfgUtils::SgNode2Str(*itA)<<", itP="<<cfgUtils::SgNode2Str(*itP)<<endl;

    // MemLocObjectPtrPair argP = composer->Expr2MemLoc(*itA, funcStartPart->inEdgeFromAny(), client);
    // The argument MemLoc is preferrably the argument expression but may be a memory location if the expression is not available
    MemLocObjectPtr arg = composer->OperandExpr2MemLoc(call, *itA, callEdge->source()->outEdgeToAny(), client);
    // if(argP.expr) arg = argP.expr;
    // else    arg = argP.mem;
    //if(analysisDebugLevel>=1) dbg << "argParamMap["<<arg->str()<<"]="<< composer->Expr2MemLoc(*itP, funcStartPart->inEdgeFromAny(), client)->str()<<endl;
    /*dbg << "funcStartPart="<<funcStartPart->str()<<endl;
    dbg << "itP="<<SgNode2Str(*itP)<<endl;*/
    argParamMap.insert(MLMapping(arg,
                                 composer->Expr2MemLoc(*itP, funcStartPart->outEdgeToAny(), client),
                                 // In the forward direction only replace parameters passed by reference
                                 // In the backward direction replace everything
                                 !fw || (fw && isSgReferenceType(typeParam))));
  }
}

// Given a function call, sets paramArgByRef2ParamMap to map all arguments of this function call that are passed by
// reference to their corresponding parameters and to map the call's SgFunctionCallExp expression to the MemLocObject
// that denotes the function's declaration (associated with its return value).
// Supports callee->caller transfers for forwards analyses and caller->callee transfers for backwards analyses.
void setArgByRef2ParamMap(PartEdgePtr callEdge, SgFunctionCallExp* call,
                          std::set<MLMapping>& paramArgByRef2ParamMap,
                          Composer* composer, ComposedAnalysis* client)
{
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("setArgByRef2ParamMap", scope::medium), 1, partitionsDebugLevel)
#endif
  std::set<CFGNode> exitNodes;
  assert(callEdge->source()->mustFuncExit(exitNodes));
  // For now we can only handle 1 CFGNode per Part
  assert(exitNodes.size()==1);
  assert(isSgFunctionDefinition(exitNodes.begin()->getNode()));
  Function func(isSgFunctionDefinition(exitNodes.begin()->getNode()));

  PartPtr callPart = callEdge->source();
  PartPtr funcStartPart = callEdge->target();
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "callEdge="<<callEdge->str()<<endl, 1, partitionsDebugLevel)
#endif

  // Part that corresponds to the function, which for now is set to be the start of its definition
  //PartPtr funcStartPart = client->getComposer()->GetFunctionStartPart(func, client);

  SgExpressionPtrList args = call->get_args()->get_expressions();
  SgInitializedNamePtrList* params = func.get_args();
  if(!params) {
    cout << "callEdge="<<callEdge->str()<<endl;
    cout << "func="<<func.str();
  }
  assert(params);

  SgExpressionPtrList::iterator itArgs;
  SgInitializedNamePtrList::iterator itParams;
  //cout << "      #params="<<params->size()<<" #args="<<args.size()<<"\n";
  for(itParams = params->begin(), itArgs = args.begin();
      itParams!=params->end() && itArgs!=args.end();
      itParams++, itArgs++)
  {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, ("iter", scope::low), 1, partitionsDebugLevel)
    SIGHT_VERB_IF(1, partitionsDebugLevel)
      dbg << "itArgs="<<SgNode2Str(*itArgs)<<endl;
      dbg << "itParams="<<SgNode2Str(*itParams)<<endl;
    SIGHT_VERB_FI()
#endif
    SgType* typeParam = (*itParams)->get_type();

    // Skip "..." types, which are used to specify VarArgs.
    // NEED TO SUPPORT VAR ARGS MORE EFECTIVELY IN THE FUTURE
    if(isSgTypeEllipse(typeParam)) continue;
    // Skip variables with no names (possible in function declarations that have no definitions).
    // NEED BETTER SUPPORT FOR FUNCTIONS WITH NO BODIES
    //if((*itParams)->get_name().getString() == "") continue;

    if(isSgReferenceType(typeParam)) {
  // If the current argument expression corresponds to a real memory location, make its key the MemLocObject
  // that corresponds to its memory location
  /*scope reg("setArgByRef2ParamMap", scope::medium);
  dbg << "itParams=["<<(*itParams)->unparseToString()<<" | "<<(*itParams)->class_name()<<"]"<<endl;
  dbg << "itParams MemLoc = "<<composer->Expr2MemLoc(*itParams, funcStartPart, client).strp(funcStartPart)<<endl;*/
  // if(isSgVarRefExp(*itArgs) || isSgPntrArrRefExp(*itArgs))
  //   paramArgByRef2ParamMap.insert(make_pair(composer->Expr2MemLoc(*itArgs, callPart->inEdgeFromAny(), client).mem,
  //             composer->Expr2MemLoc(*itParams, funcStartPart->outEdgeToAny(), client).mem));
  // // Otherwise, use the expression MemLocObject
  // else
  //   paramArgByRef2ParamMap.insert(make_pair(composer->Expr2MemLoc(*itArgs, callPart->inEdgeFromAny(), client).expr,
  //             composer->Expr2MemLoc(*itParams, funcStartPart->outEdgeToAny(), client).mem));
  //   }
      paramArgByRef2ParamMap.insert(MLMapping(composer->OperandExpr2MemLoc(call, *itArgs,   callEdge->target()->inEdgeFromAny(), client),
                                              composer->Expr2MemLoc(*itParams, funcStartPart->outEdgeToAny(),       client), true));
    // Parameters that are not passed in by reference are mapped to the NULL MemLoc to indicate that their scope is
    // purely inside the function and they should not be propagated across function boundaries
    } else
      paramArgByRef2ParamMap.insert(MLMapping(NULLMemLocObject,
                                              composer->Expr2MemLoc(*itParams, funcStartPart->outEdgeToAny(), client), true));
  }
  // Add the mapping from the FuncResultMemLocObject that denotes the return value to the function's call expression
  /*dbg << "declSymbol=["<<func.get_declaration()->search_for_symbol_from_symbol_table()->unparseToString()<<" | "<<func.get_declaration()->search_for_symbol_from_symbol_table()->class_name()<<"]"<<endl;
  dbg << "declSymbol MemLoc = "<<composer->Expr2MemLoc(func.get_declaration()->search_for_symbol_from_symbol_table(), funcStartPart, client).str()<<endl;
  dbg << "declSymbol MemLoc (funcStartPart)= "<<composer->Expr2MemLoc(func.get_declaration()->search_for_symbol_from_symbol_table(), funcStartPart, client).strp(funcStartPart)<<endl;
  dbg << "declSymbol MemLoc (callPart)= "<<composer->Expr2MemLoc(func.get_declaration()->search_for_symbol_from_symbol_table(), funcStartPart, client).strp(callPart)<<endl;*/

  // paramArgByRef2ParamMap.insert(make_pair(composer->Expr2MemLoc(call, callPart->inEdgeFromAny(), client).expr,
  //           composer->Expr2MemLoc(func.get_declaration()->search_for_symbol_from_symbol_table(), funcStartPart->outEdgeToAny(), client).mem));
  paramArgByRef2ParamMap.insert(MLMapping(composer->Expr2MemLoc(call, callEdge, client),
                                          //composer->Expr2MemLoc(func.get_declaration()->search_for_symbol_from_symbol_table(), funcStartPart->outEdgeToAny(), client)
                                          boost::make_shared<FuncResultMemLocObject>(func), true));
                                          //boost::make_shared<CombinedMemLocObject>(Union, client, boost::make_shared<FuncResultMemLocObject>(func)), true));
}

// Given a map produced by setArgParamMap or setArgByRef2ParamMap, return the same map but where the key->value
// mappings are inverted to value->key
std::set<MLMapping> invertArg2ParamMap(std::set<MLMapping> ml2ml)
{
  std::set<MLMapping> ret;
  for(std::set<MLMapping>::iterator m=ml2ml.begin(); m!=ml2ml.end(); m++)
    ret.insert(MLMapping(m->to, m->from, m->replaceMapping));
  return ret;
}


/* ###################
   ##### Context #####
   ################### */

PartContextPtr NULLPartContextPtr;

bool PartContext::operator!=(const PartContextPtr& that) const { return !(*this==that); }
bool PartContext::operator>=(const PartContextPtr& that) const { return !(*this<that); }
bool PartContext::operator<=(const PartContextPtr& that) const { return (*this<that) || (*this == that); }
bool PartContext::operator> (const PartContextPtr& that) const { return !(*this<=that); }

Context::Context(PartPtr part)/*: part(part)*/ {
  // Initialize partContexts from this part and its ancestors
  PartPtr p = part;
  while(p) {
    if(p->getPartContext()) partContexts.push_back(p->getPartContext());
    PartPtr parent = p->getInputPart();
    // If the Part is its own parent (true for Stx analysis, which has no suparset), break out
    if(parent == p->getInputPart()) break;
    else p = parent;
  }
}

// Returns the list of PartContexts that comprise this Context
const list<PartContextPtr>& Context::getPartContexts() const
{ return partContexts; }

// Returns a more detailed PartContexts lists that comprise this Context. This list contains one list for each
// PartContext implemented by some analysis and this list contains the PartContext(s) that contain a fully detailed
// description of the internal notion of context within the analysis.
list<list<PartContextPtr> > Context::getDetailedPartContexts() const {
  list<list<PartContextPtr> > subPartContexts;
  for(list<PartContextPtr>::const_iterator i=partContexts.begin(); i!=partContexts.end(); i++) {
    list<PartContextPtr> cur = (*i)->getSubPartContexts();

    // If there is no internal structure in this context, use the context itself
    if(cur.size()==0) cur.push_back(*i);

    //subPartContexts.splice(subPartContexts.end(), cur);
    subPartContexts.push_back(cur);
  }
  return subPartContexts;
}

/*
//bool Context::equalContext(ConstPartPtr a, ConstPartPtr b) {
bool Context::equalContext(PartPtr a, PartPtr b) {
  // If we're currently not at the root of the parent hierarchy
  if(a && b) {
           // Return whether the parent contexts are equal AND
    return equalContext(a->getInputPart(), b->getInputPart()) &&
           // This level's contexts are equal
           (a->getPartContext() && b->getPartContext() ? a->getPartContext() == b->getPartContext() :
                                                         true);
  // If we're at the root, all contexts are equal
  } else {
    // Make sure that a and b agree about their ancestry
    assert(!a); assert(!b);
    return true;
  }
}*/

bool Context::operator==(const ContextPtr& that) const {
  //dbg << "Context::operator==  #partContexts="<<partContexts.size()<<" #that->partContexts="<<that->partContexts.size()<<endl;
  if(partContexts.size() != that->partContexts.size()) return false;

  list<PartContextPtr>::const_iterator thisI = partContexts.begin(),
                                       thatI = that->partContexts.begin();
  for(; thisI!=partContexts.end() && thatI!=that->partContexts.end(); thisI++, thatI++) {
    if(*thisI != *thatI) return false;
  }
  return true;
}

/*
//bool Context::lessContext(ConstPartPtr a, ConstPartPtr b) {
bool Context::lessContext(PartPtr a, PartPtr b) {
  // If we're currently not at the root of the parent hierarchy
  if(a && b) {
           // Return whether the parent of a is LESS-THAN the parent of b OR
    return lessContext(a->getInputPart(), b->getInputPart()) ||
           // The parents of a and b are EQUAL equal AND
           (equalContext(a->getInputPart(), b->getInputPart()) &&
           // This level's contexts are equal
            (a->getPartContext() && b->getPartContext() ? a->getPartContext() < b->getPartContext():
                                                          false));
  // If we're at the root, all contexts are equal
  } else {
    // Make sure that a and b agree about their ancestry
    assert(!a); assert(!b);
    return false;
  }
}*/
bool Context::operator<(const ContextPtr& that) const {
  //return lessContext(part, that->part);
  //dbg << "Context::operator<  #partContexts="<<partContexts.size()<<" #that->partContexts="<<that->partContexts.size()<<endl;
  if(partContexts.size() < that->partContexts.size()) return true;
  if(partContexts.size() > that->partContexts.size()) return false;

  list<PartContextPtr>::const_iterator thisI = partContexts.begin(),
                                       thatI = that->partContexts.begin();
  for(; thisI!=partContexts.end() && thatI!=that->partContexts.end(); thisI++, thatI++) {
    if(*thisI < *thatI) return true;
    if(*thisI > *thatI) return false;
  }
  // All contexts must be equal
  return false;
}

bool Context::operator!=(const ContextPtr& that) const { return !(*this==that); }
bool Context::operator>=(const ContextPtr& that) const { return !(*this<that); }
bool Context::operator<=(const ContextPtr& that) const { return (*this<that) || (*this == that); }
bool Context::operator> (const ContextPtr& that) const { return !(*this<=that); }

std::string Context::str(std::string indent) const
{
  ostringstream oss;

  oss << "[Context: ";
  //oss << str_rec(part, indent);
  bool wroteAnyCtxts = false;
  for(list<PartContextPtr>::const_iterator p=partContexts.begin(); p!=partContexts.end(); p++) {
    string partStr = (*p)->str();
    if(partStr != "") {
      if(!wroteAnyCtxts) oss << endl << indent;
      wroteAnyCtxts = true;
      oss << partStr;
    }
  }

  oss << "]";
  return oss.str();
}

// Generates the string representation of this context by recursively
// calling the string method of this parent's
//std::string Context::str_rec(ConstPartPtr part, std::string indent)
/*std::string Context::str_rec(PartPtr part, std::string indent)
{
  if(part) {
    ostringstream oss;
    if(part->getInputPart()) {
      // Get the current part's context
      string partStr = str_rec(part->getInputPart(), indent);
      // If this part has a non-trivial context, add its string representation
      if(partStr != "") oss << str_rec(part->getInputPart(), indent) << endl << indent;
    }
    oss << part->getPartContext().str();
    return oss.str();
  } else
    return "";
}*/

/* ################
   ##### Part #####
   ################ */

PartPtr NULLPart;
PartEdgePtr NULLPartEdge;

Part::Part(ComposedAnalysis* analysis, PartPtr inputPart, PartContextPtr pContext) :
    analysis(analysis), inputPart(inputPart), pContext(pContext)
{
  // Set the NodeStateLocPart to be the inputPart by default. If we're constructing an
  // ATS in a tight composition the NodeStateLocPart will be reset once this Part is added
  // to the IntersectionPart that contains it.
  NodeStateLocPart = inputPart;
}

Part::Part(const Part& that) :
    analysis(that.analysis), inputPart(that.inputPart), NodeStateLocPart(that.NodeStateLocPart), pContext(that.pContext)
{
  if(analysis)
    analysis->registerBase2RefinedMapping(inputPart, shared_from_this());
}

Part::~Part()
{
  /*scope reg(txt()<<"Deleting Part "<<this, scope::medium);*/
}

// Returns the NodeState where the Lattices computed by the analysis that implements this Part
// (given as an argument) may be found
/*NodeState* Part::getNodeState(ComposedAnalysis* analysis) const {
  assert(NodeStateLocPart);

  NodeState* state = NodeState::getNodeState(analysis, NodeStateLocPart);
  assert(state);
  return state;
}*/

// Returns the context that includes this Part and its ancestors.
ContextPtr Part::getContext() {
  //return makePtr<const Context>(makePtrFromThis(shared_from_this()));
  //return makePtr<Context>(this);
  boost::shared_ptr<Part> me_shr = shared_from_this();
  PartPtr me = makePtrFromThis(me_shr);
  return makePtr<Context>(me);
  //return NULLContextPtr;
}

// Returns the specific context of this Part. Can return the NULLPartContextPtr if this
// Part doesn't implement a non-trivial context.
PartContextPtr Part::getPartContext() const {
  return pContext;
}

// If the filter accepts (returns true) on any of the CFGNodes within this part, return true)
bool Part::filterAny(bool (*filter) (CFGNode cfgn))
{
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(filter(*i)) return true;
  }
  return false;
}

// If the filter accepts (returns true) on all of the CFGNodes within this part, return true)
bool Part::filterAll(bool (*filter) (CFGNode cfgn))
{
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(!filter(*i)) return false;
  }
  return true;
}

// Returns whether this node denotes the portion of a function call that targets the entry into another function
bool Part::isOutgoingFuncCall(CFGNode cfgn)
{ return isSgFunctionCallExp(cfgn.getNode()) && cfgn.getIndex()==2; }

// Returns whether all of this Part's CFGNodes denote the outgoing portion of a function call and
// return the relevant CFGNode(s)
bool Part::mustOutgoingFuncCall(set<CFGNode>& ret) {
  ret.clear();
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(!isOutgoingFuncCall(*i)) { ret.clear(); return false; }
    ret.insert(*i);
  }
  return true;
}

// Returns whether some of this Part's CFGNodes denote the outgoing portion of a function call and
// return the relevant CFGNode(s)
bool Part::mayOutgoingFuncCall(set<CFGNode>& ret) {
  ret.clear();
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(isOutgoingFuncCall(*i)) ret.insert(*i);
  }
  return ret.size() > 0;
}

bool Part::mustOutgoingFuncCall() {
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(!isOutgoingFuncCall(*i)) return false;
  }
  return true;
}

bool Part::mayOutgoingFuncCall() {
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(isOutgoingFuncCall(*i)) return true;
  }
  return false;
}

// Returns whether this node denotes the portion of a function call to which control from the callee function
// flows after it terminates.
bool Part::isIncomingFuncCall(CFGNode cfgn)
{ return isSgFunctionCallExp(cfgn.getNode()) && cfgn.getIndex()==3; }

// Returns whether all of this Part's CFGNodes denote the incoming portion of a function call and
// return the relevant CFGNode(s)
bool Part::mustIncomingFuncCall(set<CFGNode>& ret) {
  ret.clear();
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(!isIncomingFuncCall(*i)) { ret.clear(); return false; }
    ret.insert(*i);
  }
  return true;
}

// Returns whether some of this Part's CFGNodes denote the incoming portion of a function call and
// return the relevant CFGNode(s)
bool Part::mayIncomingFuncCall(set<CFGNode>& ret) {
  ret.clear();
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(isIncomingFuncCall(*i)) ret.insert(*i);
  }
  return ret.size() > 0;
}

bool Part::mustIncomingFuncCall() {
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(!isIncomingFuncCall(*i)) return false;
  }
  return true;
}

bool Part::mayIncomingFuncCall() {
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(isIncomingFuncCall(*i)) return true;
  }
  return false;
}

// Returns whether both this and that parts have the same context and their CFGNode lists consist
// exclusively of matching pairs of outgoing and incoming function calls (for each outgoing call in one
// list there's an incoming call in the other and vice versa).
bool Part::mustMatchFuncCall(PartPtr that) {
  //scope reg("Part::mustMatchFuncCall()", scope::medium);
  //dbg << "this = "<<str()<<endl;
  //dbg << "that = "<<that->str()<<endl;

  set<CFGNode> thisCN = CFGNodes();
  set<CFGNode> thatCN = that->CFGNodes();
  //dbg << "#thisCN="<<thisCN.size()<< " #thatCN="<<thatCN.size()<<endl;

  if(getContext() != that->getContext()) return false;
  if(thisCN.size() != thatCN.size()) return false;

  for(set<CFGNode>::iterator thisI=thisCN.begin(); thisI!=thisCN.end(); thisI++) {
    //dbg << "thisI="<<cfgUtils::CFGNode2Str(*thisI)<<" isIncomingFuncCall(*thisI)="<<isIncomingFuncCall(*thisI)<<endl;
    if(isIncomingFuncCall(*thisI)) {
      set<CFGNode>::iterator thatI;
      for(thatI = thatCN.begin(); thatI!=thatCN.end(); thatI++) {
        //dbg << "thatI="<<cfgUtils::CFGNode2Str(*thatI)<<" isIncomingFuncCall(*thatI)="<<isIncomingFuncCall(*thatI)<<endl;
        //dbg << "thisI->getNode() == thatI->getNode()="<<(thisI->getNode() == thatI->getNode())<<endl;
        // If thisI is an incoming call and thatI is an outgoing call
        if(isOutgoingFuncCall(*thatI) &&
           // and both correspond to the same function call
           thisI->getNode() == thatI->getNode()) break;
      }
      // If we couldn't find a match for *thisI
      if(thatI == thatCN.end()) return false;

    } else if(isOutgoingFuncCall(*thisI)) {
      set<CFGNode>::iterator thatI;
      for(thatI = thatCN.begin(); thatI!=thatCN.end(); thatI++) {
        // If thisI is an outgoing call and thatI is an incoming call
        if(isIncomingFuncCall(*thatI) &&
           // and both correspond to the same function call
           thisI->getNode() == thatI->getNode()) break;
      }
      // If we couldn't find a match for *thisI
      if(thatI == thatCN.end()) return false;
    }
  }

  // If all of thisCN has a match in all of thatCN and vice versa
  return true;
}

// Returns whether both this and that parts have the same context and their CFGNode lists include some
// matching pairs of outgoing and incoming function calls.
bool Part::mayMatchFuncCall(PartPtr that) {
  set<CFGNode> thisCN = CFGNodes();
  set<CFGNode> thatCN = that->CFGNodes();

  if(getContext() != that->getContext()) return false;

  for(set<CFGNode>::iterator thisI=thisCN.begin(); thisI!=thisCN.end(); thisI++) {
    if(isIncomingFuncCall(*thisI)) {
      for(set<CFGNode>::iterator thatI = thatCN.begin(); thatI!=thatCN.end(); thatI++)
        // If thisI is an incoming call and thatI is an outgoing call
        if(isOutgoingFuncCall(*thatI) &&
           // and both correspond to the same function call
           thisI->getNode() == thatI->getNode()) return true;

    } else if(isOutgoingFuncCall(*thisI)) {
      for(set<CFGNode>::iterator thatI = thatCN.begin(); thatI!=thatCN.end(); thatI++)
        // If thisI is an outgoing call and thatI is an incoming call
        if(isIncomingFuncCall(*thatI) &&
           // and both correspond to the same function call
           thisI->getNode() == thatI->getNode()) return true;
    }
  }

  // If we couldn't find even one match
  return false;
}

// Returns whether this node denotes the entry into a function
bool Part::isFuncEntry(CFGNode cfgn)
{ return isSgFunctionParameterList(cfgn.getNode()); }

// Returns whether all of this Part's CFGNodes denote a function's entry node and return the relevant CFGNode(s)
bool Part::mustFuncEntry(set<CFGNode>& ret) {
  ret.clear();
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(!isFuncEntry(*i)) { ret.clear(); return false; }
    ret.insert(*i);
  }
  return true;
}

// Returns whether all of this Part's CFGNodes denote a function's entry node and return the relevant CFGNode(s)
bool Part::mayFuncEntry(set<CFGNode>& ret) {
  ret.clear();
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(isFuncEntry(*i)) ret.insert(*i);
  }
  return ret.size() > 0;
}

// Returns whether this node denotes the exit from a function
bool Part::isFuncExit(CFGNode cfgn)
{ return isSgFunctionDefinition(cfgn.getNode()); }

// Returns whether all of this Part's CFGNodes denote a function's exit node and return the relevant CFGNode(s)
bool Part::mustFuncExit(set<CFGNode>& ret) {
  ret.clear();
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(!isFuncExit(*i)) { ret.clear(); return false; }
    ret.insert(*i);
  }
  return true;
}

// Returns whether all of this Part's CFGNodes denote a function's exit node and return the relevant CFGNode(s)
bool Part::mayFuncExit(set<CFGNode>& ret) {
  ret.clear();
  std::set<CFGNode> v=CFGNodes();
  for(std::set<CFGNode>::iterator i=v.begin(); i!=v.end(); i++) {
    if(isFuncExit(*i)) ret.insert(*i);
  }
  return ret.size() > 0;
}

// If this and that come from the same analysis, call the type-specific equality test implemented
// in the derived class. Otherwise, these Parts are not equal.
bool Part::operator==(const PartPtr& that) const
{
/*  cout << "Part::operator=="<<endl;
  cout << "    this="<<str()<<endl;
  cout << "    that="<<that.str()<<endl;*/
  const StartPart*   thisStart = dynamic_cast<const StartPart*>(this);
  StartPartPtr thatStart = dynamicPtrCast<StartPart>(that);
//  cout << "        thisStart="<<thisStart<<", thatStart="<<thatStart.get()<<endl;
  // If either part is a starting part
  if(thisStart || thatStart) {
    // If this is a Start, use its equal method
    if(thisStart) return equal(that);
    // If that is a Start but this is not, use that->equal method
    else          return thatStart->equal(((Part*)this)->shared_from_this());
  }

  const EndPart*   thisEnd = dynamic_cast<const EndPart*>(this);
  EndPartPtr thatEnd = dynamicPtrCast<EndPart>(that);
//  cout << "        thisEnd="<<thisEnd<<", thatEnd="<<thatEnd.get()<<endl;
  // If either part is an ending part
  if(thisEnd || thatEnd) {
    // If this is a End, use its equal method
    if(thisEnd) return equal(that);
    // If that is a End but this is not, use that->equal method
    else          return thatEnd->equal(((Part*)this)->shared_from_this());
  }

  // If neither part is start, nor end

  // If both parts belong to the same analysis
  //cout << "        analysis="<<analysis<<", that->analysis="<<that->analysis<<", equal="<<equal(that)<<endl;
  if(getAnalysis() == that->getAnalysis()) return equal(that);
  // If the parts belong to different analyses, they are definitely not equal
  else                                     return false;
}

// If this and that come from the same analysis, call the type-specific inequality test implemented
// in the derived class. Otherwise, determine inequality by comparing the analysis pointers.
bool Part::operator<(const PartPtr& that) const
{
/*  cout << "Part::operator<"<<endl;
  cout << "    this="<<str()<<endl;
  cout << "    that="<<that.str()<<endl;*/

  const StartPart*   thisStart = dynamic_cast<const StartPart*>(this);
  StartPartPtr thatStart = dynamicPtrCast<StartPart>(that);
//  cout << "        thisStart="<<thisStart<<", thatStart="<<thatStart.get()<<endl;
  // If either part is a starting part
  if(thisStart || thatStart) {
    // If this is a Start, use its less method
    if(thisStart) return less(that);
    // If that is a Start but this is not, use that->less method
    else          return !thatStart->equal(((Part*)this)->shared_from_this()) &&
                         !thatStart->less(((Part*)this)->shared_from_this());
  }

  const EndPart*   thisEnd = dynamic_cast<const EndPart*>(this);
  EndPartPtr thatEnd = dynamicPtrCast<EndPart>(that);
//  cout << "        thisEnd="<<thisEnd<<", thatEnd="<<thatEnd.get()<<endl;
  // If either part is an ending part
  if(thisEnd || thatEnd) {
    // If this is a End, use its less method
    if(thisEnd) return less(that);
    // If that is a End but this is not, use that->less method
    else          return thatEnd->less(((Part*)this)->shared_from_this());
  }

  // If neither part is start, nor end
//  cout << "        analysis="<<analysis<<", that->analysis="<<that->analysis<<", less="<<less(that)<<endl;
  // If both parts belong to the same analysis
  if(getAnalysis() == that->getAnalysis()) return less(that);
  // If the edges belong to different analyses, order them according to analysis
  else                                     return getAnalysis() < that->getAnalysis();
}

bool Part::operator!=(const PartPtr& that) const { return !(*this==that); }
bool Part::operator>=(const PartPtr& that) const { return !(*this<that); }
bool Part::operator<=(const PartPtr& that) const { return (*this<that) || (*this == that); }
bool Part::operator> (const PartPtr& that) const { return !(*this<=that); }

/* #####################
   ##### StartPart #####
   ##################### */

std::list<PartEdgePtr> StartPart::outEdges() {
  list<PartEdgePtr> out;
  for(std::set<PartPtr>::const_iterator p=parts.begin(); p!=parts.end(); p++)
    out.push_back(makePtr<TerminalPartEdge>(shared_from_this(), *p));
  return out;
}

std::list<PartEdgePtr> StartPart::inEdges()  {
  list<PartEdgePtr> empty;
  return empty;
}

std::set<CFGNode> StartPart::CFGNodes() const {
  set<CFGNode> empty;
  return empty;
}

// If this Part corresponds to a function call/return, returns the set of Parts that contain
// its corresponding return/call, respectively.
std::set<PartPtr> StartPart::matchingCallParts() const {
  set<PartPtr> empty;
  return empty;
}

// If this Part corresponds to a function entry/exit, returns the set of Parts that contain
// its corresponding exit/entry, respectively.
std::set<PartPtr> StartPart::matchingEntryExitParts() const {
  set<PartPtr> empty;
  return empty;
}

// Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
PartEdgePtr StartPart::inEdgeFromAny() {
  return makePtr<TerminalPartEdge>(NULLPart, shared_from_this());
}

// Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
PartEdgePtr StartPart::outEdgeToAny() {
  return makePtr<TerminalPartEdge>(shared_from_this(), NULLPart);
}

bool StartPart::equal(const PartPtr& that_arg) const {
  StartPartPtr that = dynamicPtrCast<StartPart>(that_arg);
  // Start is equal to itself and nothing else
  if(that) return true;
  else     return false;
}
bool StartPart::less(const PartPtr& that_arg) const {
  StartPartPtr that = dynamicPtrCast<StartPart>(that_arg);
  // Start is less than anything else
  if(that) return false;
  else     return true;
}

std::string StartPart::str(std::string indent) const
{ return "[StartPart]"; }

/* #####################
   ##### EndPart #####
   ##################### */

std::list<PartEdgePtr> EndPart::outEdges() {
  list<PartEdgePtr> empty;
  return empty;
}

std::list<PartEdgePtr> EndPart::inEdges()  {
  list<PartEdgePtr> in;
  for(std::set<PartPtr>::const_iterator p=parts.begin(); p!=parts.end(); p++)
    in.push_back(makePtr<TerminalPartEdge>(*p, shared_from_this()));
  return in;
}

std::set<CFGNode> EndPart::CFGNodes() const {
  set<CFGNode> empty;
  return empty;
}

// If this Part corresponds to a function call/return, returns the set of Parts that contain
// its corresponding return/call, respectively.
std::set<PartPtr> EndPart::matchingCallParts() const {
  set<PartPtr> empty;
  return empty;
}

// If this Part corresponds to a function entry/exit, returns the set of Parts that contain
// its corresponding exit/entry, respectively.
std::set<PartPtr> EndPart::matchingEntryExitParts() const {
  set<PartPtr> empty;
  return empty;
}

// Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
PartEdgePtr EndPart::inEdgeFromAny() {
  return makePtr<TerminalPartEdge>(NULLPart, shared_from_this());
}

// Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
PartEdgePtr EndPart::outEdgeToAny() {
  return makePtr<TerminalPartEdge>(shared_from_this(), NULLPart);
}

bool EndPart::equal(const PartPtr& that_arg) const {
  EndPartPtr that = dynamicPtrCast<EndPart>(that_arg);
  // End is equal to itself and nothing else
  if(that) return true;
  else     return false;
}
bool EndPart::less(const PartPtr& that_arg) const {
  EndPartPtr that = dynamicPtrCast<EndPart>(that_arg);
  // End is greater than anything else
  return false;
}

std::string EndPart::str(std::string indent) const
{ return "[EndPart]"; }

/* ####################
   ##### PartEdge #####
   #################### */
PartEdge::PartEdge(ComposedAnalysis* analysis, PartEdgePtr inputPartEdge) :
    analysis(analysis), inputPartEdge(inputPartEdge) {
}

PartEdge::PartEdge(ComposedAnalysis* analysis, PartEdgePtr NodeStateLocPartEdge, PartEdgePtr inputPartEdge) :
    analysis(analysis), NodeStateLocPartEdge(NodeStateLocPartEdge), inputPartEdge(inputPartEdge) {
}


PartEdge::PartEdge(const PartEdge& that) :
    analysis(that.analysis),
    NodeStateLocPartEdge(that.NodeStateLocPartEdge), inputPartEdge(that.inputPartEdge), remap(that.remap) {
}

// Function that will always be called after a PartEdge is created and before it is returned
// to the caller. It is called outside of the PartEdge constructor, which makes it possible to
// place code inside that registers a shared pointer to this PartEdge with a directory of some kind.
void PartEdge::init() {
  if(analysis)
    analysis->registerBase2RefinedMapping(inputPartEdge, shared_from_this());
  /*dbg << "PartEdge::init() mapping base "<<(parent?parent->str():"NULL")<<" to "<<endl;
  dbg << "     refined "<<str()<<endl;*/
}

// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    PartEdge that denotes a subset of A (the function is called on this PartEdge),
//    it returns a list of PartEdges that partition O.
std::list<PartEdgePtr> PartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand)
{
  // operand precedes anchor in the CFG, either immediately or at some distance. As such, the edge
  //   we're looking for is not necessarily the edge from operand to anchor but rather the first
  //   edge along the path from operand to anchor. Since operand is part of anchor's expression
  //   tree we're guaranteed that there is only one such path.
  // The implementor of the partition we're running on may have created multiple parts for
  //   operand to provide path sensitivity and indeed, may have created additional outgoing edges
  //   from each of the operand's parts. Fortunately, since in the original program the original
  //   edge led from operand to anchor and the implementor of the partition could have only hierarchically
  //   refined the original partition, all the new edges must also lead from operand to anchor.
  //   As such, the returned list contains all the outgoing edges from all the parts that correspond
  //   to operand.
  // Note: if the partitioning process is not hierarchical we may run into minor trouble since the
  //   new edges from operand may lead to parts other than anchor. However, this is just an issue
  //   of precision since we'll account for paths that are actually infeasible.

  struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);

  // The target of this edge identifies the termination point of all the execution prefixes
  // denoted by this edge. We thus use it to query for the parts of the operands and only both
  // if this part is itself live.
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("PartEdge::getOperandPartEdge()", scope::medium), 2, partitionsDebugLevel)
  SIGHT_VERB_IF(2, partitionsDebugLevel)
    dbg << "anchor="<<SgNode2Str(anchor)<<" operand="<<SgNode2Str(operand)<<endl;
    dbg << "this PartEdge="<<str()<<endl;
    dbg << "getInputPartEdge()="<<getInputPartEdge()->str()<<endl;
  SIGHT_VERB_FI()
#endif

  std::list<PartEdgePtr> baseEdges = getInputPartEdge()->getOperandPartEdge(anchor, operand);
  // Convert the list of edges into a set for easier/faster lookups
  set<PartEdgePtr> baseEdgesSet;
  for(list<PartEdgePtr>::iterator be=baseEdges.begin(); be!=baseEdges.end(); be++)
    baseEdgesSet.insert(*be);

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(2, partitionsDebugLevel)
    SIGHT_VERB_DECL(scope, ("baseOperandEdges", scope::medium), 2, partitionsDebugLevel)
    for(list<PartEdgePtr>::iterator be=baseEdges.begin(); be!=baseEdges.end(); be++)
      dbg << be->get()->str();
  SIGHT_VERB_FI()
#endif

  /*cout << "------------------------------------"<<endl;
  cout << "analysis="<<getAnalysis()->str()<<endl;
  cout << "this="<<str()<<endl;
  for(list<PartEdgePtr>::iterator be=baseEdges.begin(); be!=baseEdges.end(); be++) {
    cout << "base Edge="<<be->get()->str()<<endl;
    set<PartEdgePtr> refined = getAnalysis()->getRefinedPartEdges(*be);
    for(set<PartEdgePtr>::iterator ref=refined.begin(); ref!=refined.end(); ++ref) {
      cout << "    "<<(*ref)->str()<<endl;
    }
  }*/

  // Collect all refined edges in this ATS that correspond to the operand edges in the
  // superset ATS
  set<PartEdgePtr> refined;
  for(list<PartEdgePtr>::iterator be=baseEdges.begin(); be!=baseEdges.end(); be++) {
    set<PartEdgePtr> curRefined = getAnalysis()->getRefinedPartEdges(*be);
    refined.insert(curRefined.begin(), curRefined.end());
  }

  // !!! Should change API to return a set!!!
  list<PartEdgePtr> refinedL;
  for(set<PartEdgePtr>::iterator i=refined.begin(); i!=refined.end(); ++i)
    refinedL.push_back(*i);

  return refinedL;
  /*
  std::set<PartEdgePtr> ccsOperandEdges;
  bw_dataflowPartEdgeIterator it(succ_back);
  it.add(makePtrFromThis(shared_from_this()));

  / * // There are scenarios where getOperandPartEdge() is called on edges that immediately precede
  // the desired operand->anchor edge (e.g. live-dead analysis performs its transfer function
  // on the incoming edge of the current part to correctly read live-dead decisions of any prior
  // live-dead analyses). As such, initialize the graph iterator with this PartEdge's successors.
  std::list<PartEdgePtr> out;
  if(target())      out = target()->outEdges();
  else if(source()) out = source()->outEdges();
  for(std::list<PartEdgePtr>::iterator o=out.begin(); o!=out.end(); o++)
    it.add(*o);* /

  gettimeofday(&gopeEnd, NULL); cout << "PartEdge::getOperandPartEdge() setup\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;

  SIGHT_VERB(dbg << "it="<<it.str()<<endl, 2, partitionsDebugLevel)
  SIGHT_VERB_DECL(scope, ("Backward search", scope::medium), 2, partitionsDebugLevel)

  struct timeval searchStart, searchEnd; gettimeofday(&searchStart, NULL);

  // Walk backwards through the CCS edges, looking for the most recent CCS edge the parent of which is in list baseEdges
  int numSteps=0;
  while(!it.isEnd()) {
    SIGHT_VERB_DECL(scope, ("Predecessor", scope::low), 2, partitionsDebugLevel)
    SIGHT_VERB_IF(2, partitionsDebugLevel)
    dbg << it.getPartEdge().get()->str()<<endl;
    dbg << "pred-parent "<<it.getPartEdge()->getInputPartEdge()->str()<<", "<<
           "source is "<<(it.getPartEdge()->getInputPartEdge()->source()==NULLPart? "wildcard": "concrete")<<", "<<
           "target is "<<(it.getPartEdge()->getInputPartEdge()->target()==NULLPart? "wildcard": "concrete")<<", "<<endl;
    SIGHT_VERB_FI()

    // If the parent of the current edge is one of the base edges
    bool isOperandEdge = false;

    // If the current edge has any wildcards (may occur in the first iteration, which touches this edge)
    if(it.getPartEdge()->getInputPartEdge()->source()==NULLPart ||
       it.getPartEdge()->getInputPartEdge()->target()==NULLPart) {
      // Look it up in baseEdges using a linear lookup that is sensitive to wildcards (this case should be
      // rare enough that we don't optimize for it).
      for(list<PartEdgePtr>::iterator be=baseEdges.begin(); be!=baseEdges.end(); be++) {
        SIGHT_VERB_IF(3, partitionsDebugLevel)
          SIGHT_VERB_DECL(scope, (txt()<<"baseEdge="<<be->get()->str(), scope::low), 3, partitionsDebugLevel)
          dbg << "it.getPartEdge()->getInputPartEdge()->source()==NULLPart="<<(it.getPartEdge()->getInputPartEdge()->source()==NULLPart)<<", "<<
                 "it.getPartEdge()->getInputPartEdge()->target()==(*be)->target()="<<(it.getPartEdge()->getInputPartEdge()->target()==(*be)->target())<<", "<<
                 "it.getPartEdge()->getInputPartEdge()->target()==NULLPart="<<(it.getPartEdge()->getInputPartEdge()->target()==NULLPart)<<", "<<
                 "it.getPartEdge()->getInputPartEdge()->source()==(*be)->source()="<<(it.getPartEdge()->getInputPartEdge()->source()==(*be)->source())<<endl;
          dbg << "it.getPartEdge()->getInputPartEdge()->source()="<<it.getPartEdge()->getInputPartEdge()->source()->str()<<endl;
          dbg << "(*be)->source()="<<(*be)->source()->str()<<endl;
        SIGHT_VERB_FI()

        if((it.getPartEdge()->getInputPartEdge()->source()==NULLPart &&
            it.getPartEdge()->getInputPartEdge()->target()==(*be)->target()) ||
           (it.getPartEdge()->getInputPartEdge()->target()==NULLPart &&
            it.getPartEdge()->getInputPartEdge()->source()==(*be)->source())) {
          isOperandEdge = true;
          break;
        }
      }
    // If the current edge is not a wildcard, use efficient lookups to search for edges that match it.
    } else
      isOperandEdge = (baseEdgesSet.find(it.getPartEdge()->getInputPartEdge()) != baseEdgesSet.end());

    if(isOperandEdge) {
      SIGHT_VERB(dbg << "    Predecessor is an Operand edge."<<endl, 2, partitionsDebugLevel)
      // Add it to the operand edges
      ccsOperandEdges.insert(it.getPartEdge());
    // Otherwise, keep searching backward
    } else {
      SIGHT_VERB(dbg << "    Not an Operand edge. Moving on..."<<endl, 2, partitionsDebugLevel)
      it.pushAllDescendants();
    }
    it++;
    ++numSteps;
  }
  gettimeofday(&searchEnd, NULL); cout << "PartEdge::getOperandPartEdge() search\t"<<(((searchEnd.tv_sec*1000000 + searchEnd.tv_usec) - (searchStart.tv_sec*1000000 + searchStart.tv_usec)) / 1000000.0)<<"\t"<<numSteps<<endl;

  struct timeval convStart, convEnd; gettimeofday(&convStart, NULL);

  // !!! Should change API to return a set!!!
  list<PartEdgePtr> ccsOperandEdgesL;
  for(set<PartEdgePtr>::iterator i=ccsOperandEdges.begin(); i!=ccsOperandEdges.end(); ++i)
    ccsOperandEdgesL.push_back(*i);

  gettimeofday(&convEnd, NULL); cout << "PartEdge::getOperandPartEdge() convert\t"<<(((convEnd.tv_sec*1000000 + convEnd.tv_usec) - (convStart.tv_sec*1000000 + convStart.tv_usec)) / 1000000.0)<<"\t"<<ccsOperandEdgesL.size()<<endl;

  return ccsOperandEdgesL;*/
}

// If this and that come from the same analysis, call the type-specific equality test implemented
// in the derived class. Otherwise, these Parts are not equal.
bool PartEdge::operator==(const PartEdgePtr& that) const
{
  //scope s("PartEdge::operator==");
  const TerminalPartEdge*   thisTPEdge = dynamic_cast<const TerminalPartEdge*>(this);
  TerminalPartEdgePtr thatTPEdge = dynamicPtrCast<TerminalPartEdge>(that);
  // If either edge is a terminal edge
  if(thisTPEdge || thatTPEdge) {
    // If this Terminal, use its equal method
    if(thisTPEdge) return equal(that);
    // If this is not terminal but that is, use that->equal method
    else return thatTPEdge->equal(((PartEdge*)this)->shared_from_this());

  // If neither edge is terminal
  } else {
    //dbg << "analysis="<<analysis<<", that->analysis="<<that->analysis<<endl;
    // If both edges belong to the same analysis
    if(getAnalysis() == that->getAnalysis()) return equal(that);
    // If the edges belong to different analyses, they are definitely not equal
    else                                     return false;
  }
}

// If this and that come from the same analysis, call the type-specific inequality test implemented
// in the derived class. Otherwise, determine inequality by comparing the analysis pointers.
bool PartEdge::operator<(const PartEdgePtr& that) const
{
  const TerminalPartEdge*   thisTPEdge = dynamic_cast<const TerminalPartEdge*>(this);
  TerminalPartEdgePtr thatTPEdge = dynamicPtrCast<TerminalPartEdge>(that);
  // If either edge is a terminal edge
  if(thisTPEdge || thatTPEdge) {
    // If this Terminal, use its less method
    if(thisTPEdge) return less(that);
    // If this is not terminal but that is, use that->equal method
    else return !thatTPEdge->equal(((PartEdge*)this)->shared_from_this()) &&
                !thatTPEdge->less(((PartEdge*)this)->shared_from_this());

  // If neither edge is terminal
  } else {
    // If both edges belong to the same analysis
    if(getAnalysis() == that->getAnalysis()) return less(that);
    // If the edges belong to different analyses, compare the analyses
    else                                     return getAnalysis() < that->getAnalysis();
  }
}

bool PartEdge::operator!=(const PartEdgePtr& that) const { return !(*this==that); }
bool PartEdge::operator>=(const PartEdgePtr& that) const { return !(*this<that); }
bool PartEdge::operator<=(const PartEdgePtr& that) const { return (*this<that) || (*this == that); }
bool PartEdge::operator> (const PartEdgePtr& that) const { return !(*this<=that); }

// Remaps the given Lattice as needed to take into account any function call boundaries on behalf of the given
//    client analysis (Expr2* is called with this analysis as the client rather than the analysis that created
//    the ATS graph).
// Remapping is performed both in the forwards and backwards directions.
// Returns the resulting Lattice object, which is freshly allocated.
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    some MemLocs are in scope on one side of Part, while others are in scope on the other side.
//    fromPEdge is the edge from which control is passing and the current PartEdge (same as the PartEdge of
//    the Lattice) is the one to which control is passing.
Lattice* PartEdge::forwardRemapML(Lattice* lat, PartEdgePtr fromPEdge, ComposedAnalysis* client) {
  remap.init(makePtrFromThis(shared_from_this()), client);
  assert(makePtrFromThis(shared_from_this()) == lat->getPartEdge());
  return remap.forwardRemapML(lat, fromPEdge, client);
}
Lattice* PartEdge::backwardRemapML(Lattice* lat, PartEdgePtr fromPEdge, ComposedAnalysis* client) {
  remap.init(makePtrFromThis(shared_from_this()), client);
  assert(makePtrFromThis(shared_from_this()) == lat->getPartEdge());
  return remap.backwardRemapML(lat, fromPEdge, client);
}

/* ############################
   ##### TerminalPartEdge #####
   ############################ */

PartPtr TerminalPartEdge::source() const { return src; }
PartPtr TerminalPartEdge::target() const { return tgt; }

std::map<CFGNode, boost::shared_ptr<SgValueExp> > TerminalPartEdge::getPredicateValue()
{ std::map<CFGNode, boost::shared_ptr<SgValueExp> > empty; return empty; }

bool TerminalPartEdge::equal(const PartEdgePtr& that_arg) const {
  TerminalPartEdgePtr that = dynamicPtrCast<TerminalPartEdge>(that_arg);
  if(that) return src==that->src && tgt==that->tgt;
  else     return false;
}

bool TerminalPartEdge::less(const PartEdgePtr& that_arg) const {
  // Determine whether this edge departs from the start or arrives at the end
  StartPartPtr thisStart = dynamicPtrCast<StartPart>(src);
  EndPartPtr   thisEnd   = dynamicPtrCast<EndPart>  (tgt);

  TerminalPartEdgePtr that = dynamicPtrCast<TerminalPartEdge>(that_arg);
  if(that) {
    StartPartPtr thatStart = dynamicPtrCast<StartPart>(that->src);
    EndPartPtr   thatEnd   = dynamicPtrCast<EndPart>  (that->tgt);

    if(thisStart) {
      // If both edges depart the starting node, they're ordered according to their target parts
      if(thatStart) return tgt < that->tgt;
      // If this edge departs the starting node, it is ordered before succeeding edges
      else          return true;
    } else {
      // If that edge departs the starting node, it is ordered before succeeding edges
      if(thatStart) return false;
      // If neither edge departs the starting node, they must both be arriving at the ending node
      else {
        ROSE_ASSERT(thisEnd);
        ROSE_ASSERT(thatEnd);
        // Order them according to their source parts
        return src < that->src;
      }
    }
  // If that is not a TerminalPartEdge
  } else {
    // Edges departing from start are ordered before all succeeding edges
    if(thisStart) return true;
    // Edges arriving at end are ordered after all preceding edges
    if(thisEnd)   return false;
  }

  // There should be no other cases
  ROSE_ASSERT(0);
}

std::string TerminalPartEdge::str(std::string indent) const {
  ostringstream s;
  s << "[TerminalPartEdge: src="<<src->str()<<", tgt="<<tgt->str()<<endl;
  return s.str();
}

/* ################################
   ##### ATS 2 DOT Conversion #####
   ################################ */

/* ###################################
   ##### IntersectionPartContext #####
   ################################### */

bool IntersectionPartContext::operator==(const PartContextPtr& that_arg) const {
//  const IntersectionPartContextPtr& that = dynamic_cast<const IntersectionPartContext&>(that_arg);
  const IntersectionPartContextPtr& that = dynamicPtrCast<IntersectionPartContext>(that_arg);

  if(pContexts.size() != that->pContexts.size()) return false;

  set<PartContextPtr>::const_iterator thisI=pContexts.begin();
  set<PartContextPtr>::const_iterator thatI=that->pContexts.begin();
  for(; thisI!=pContexts.end() && thatI!=that->pContexts.end(); thisI++, thatI++) {
    if(*thisI != *thatI) return false;
  }

  return true;
}

bool IntersectionPartContext::operator< (const PartContextPtr& that_arg) const {
  //const IntersectionPartContext& that = dynamic_cast<const IntersectionPartContext&>(that_arg);
  const IntersectionPartContextPtr& that = dynamicPtrCast<IntersectionPartContext>(that_arg);

  if(pContexts.size() < that->pContexts.size()) return true;
  if(pContexts.size() > that->pContexts.size()) return false;

  set<PartContextPtr>::const_iterator thisI=pContexts.begin();
  set<PartContextPtr>::const_iterator thatI=that->pContexts.begin();
  for(; thisI!=pContexts.end() && thatI!=that->pContexts.end(); thisI++, thatI++) {
    if(*thisI <= *thatI) return true;
    if(*thisI > *thatI)  return false;
  }

  return false;
}

std::string IntersectionPartContext::str(std::string indent) const {
  ostringstream oss;
  oss << "[IntersectionPartContext: ";
  bool anyValidCtxtStrs = false;
  for(set<PartContextPtr>::iterator i=pContexts.begin(); i!=pContexts.end(); i++) {
    string ctxStr = (*i).get()->str();
    if(ctxStr!="") {
      if(anyValidCtxtStrs) oss << endl << indent;
      oss << ctxStr;
      anyValidCtxtStrs = true;
    }
  }
  oss << "]";
  return oss.str();
}


/* ############################
   ##### IntersectionPart #####
   ############################ */

// Recursive computation of the cross-product of the edges in the range of analysis2Edges. Hierarchically
// builds a recursion tree that contains more and more combinations of PartEdgePtrs from the analysis2Edges,
// which are associated with the partition implementations of different analyses. When the recursion
// tree reaches its full depth (one level per part in parts), it creates an intersection the current combination of
// parent - the common PartEdge that is the parent of all the edges in the range of analysis2Edges
// curA - current iterator into analysis2Edges
// outPartEdges - the list of outgoing edges of the current combination of analysis2Edges's sub-edges, upto curA->first
// edges - vector that contains all the edges in the intersection
// analysisOfIntersection - the analysis associated with the Intersection edge
void intersectEdges(PartEdgePtr parent,
                    map<ComposedAnalysis*, set<PartEdgePtr> >::iterator curA,
                    map<ComposedAnalysis*, set<PartEdgePtr> >& analysis2Edges,
                    map<ComposedAnalysis*, PartEdgePtr> outPartEdges,
                    list<PartEdgePtr>& edges,
                    ComposedAnalysis* analysisOfIntersection)
{
  ComposedAnalysis* curAnalysis = curA->first;

  // If we've reached the last part in parts and outEdgeParts contains all the outgoing PartEdges
  if(curA == analysis2Edges.end()) {
    PartEdgePtr newEdge = makePtr<IntersectionPartEdge>(outPartEdges, parent, analysisOfIntersection);
    newEdge->init();
    //dbg << "analysisOfIntersection="<<analysisOfIntersection<<" newEdge="<<newEdge->str()<<endl;
    edges.push_back(newEdge);
  // If we haven't yet reached the end, recurse on all the outgoing edges of the current part
  } else {
    // Set nextA to follow curA
    map<ComposedAnalysis*, set<PartEdgePtr> >::iterator nextA = curA;
    nextA++;

    // Recurse on the cross product of the outgoing edges of this part and the outgoing edges of subsequent parts
    for(set<PartEdgePtr>::iterator e=curA->second.begin(); e!=curA->second.end(); e++){
      outPartEdges[curAnalysis] = *e;
      intersectEdges(parent, nextA, analysis2Edges, outPartEdges, edges, analysisOfIntersection);
      outPartEdges.erase(curAnalysis);
    }
  }
}

// Maps each IntersectionPart implemented by each analysis to the set of IntersectionPartEdges that come into or
// out of it.
// This data structure caches edges as they're created during the execution of an analysis, making it possible
// to return lists of edges in the opposite direction of analysis traversal without invoking the inEdges()
// and outEdges() methods of the Parts in parts, which would end up being a circularly recursive call.
std::map<ComposedAnalysis*, std::map<IntersectionPartPtr, std::set<IntersectionPartEdgePtr> > > IntersectionPart::Part2InEdges;
std::map<ComposedAnalysis*, std::map<IntersectionPartPtr, std::set<IntersectionPartEdgePtr> > > IntersectionPart::Part2OutEdges;

/*IntersectionPart::IntersectionPart(PartPtr part, ComposedAnalysis* analysis) :
    Part(analysis)
{ parts.push_back(part); }*/

IntersectionPart::IntersectionPart(const std::map<ComposedAnalysis*, PartPtr>& parts, PartPtr parent, ComposedAnalysis* analysis) :
    Part(analysis, parent), parts(parts), initialized(false)
{}

// Initializes this object and its relationships with the Parts that it contains
void IntersectionPart::init()
{
  // First initialize the Part base class
  Part::init();

  // Set this Part as the ATS location of all the Parts it contains
  for(map<ComposedAnalysis*, PartPtr>::iterator p=parts.begin(); p!=parts.end(); ++p) {
    p->second->setNodeStateLocPart(shared_from_this());
  }
  initialized = true;
}

// Returns the list of outgoing IntersectionPartEdge of this Part, which are the cross-product of the outEdges()
// of its sub-parts.
std::list<PartEdgePtr> IntersectionPart::outEdges()
{
  assert(initialized);

#ifndef DISABLE_SIGHT
  scope s("IntersectionPart::outEdges()");
  dbg << "getInputPart()="<<getInputPart()->str()<<endl;
#endif
  // For each part in parts, maps the parent part of each outgoing part to the set of parts that share this parent
  map<PartEdgePtr, map<ComposedAnalysis*, set<PartEdgePtr> > > parent2Out;
  for(map<ComposedAnalysis*, PartPtr>::iterator part=parts.begin(); part!=parts.end(); part++) {
#ifndef DISABLE_SIGHT
    dbg << "part->second="<<part->second->str()<<endl;
#endif
    // Get this part's outgoing edges
    list<PartEdgePtr> out = part->second->outEdges();

    // Group these edges according to their common parent edge
    {
#ifndef DISABLE_SIGHT
      scope s(txt()<<"outEdges #out="<<out.size());
#endif
    for(list<PartEdgePtr>::iterator e=out.begin(); e!=out.end(); e++) {
#ifndef DISABLE_SIGHT
      dbg << "        e="<<(*e)->str()<<endl;
      dbg << "        (*e)->getInputPartEdge()="<<(*e)->getInputPartEdge()->str()<<endl;
#endif
      parent2Out[(*e)->getInputPartEdge()][part->first].insert(*e);
    }
    }
  }

  /*for(map<PartEdgePtr, map<ComposedAnalysis*, set<PartEdgePtr> > >::iterator p=parent2Out.begin(); p!=parent2Out.end(); p++) {
    PartEdgePtr pf = p->first;
    dbg << "parent="<<pf->str()<<endl;
    indent ind;
    for(map<ComposedAnalysis*, set<PartEdgePtr> >::iterator a=p->second.begin(); a!=p->second.end(); a++) {
      indent ind;
      for(set<PartEdgePtr>::iterator e=a->second.begin(); e!=a->second.end(); e++) {
        PartEdgePtr ef = *e;
        dbg << ef->str()<<endl;
   } } }*/

  // Create a cross-product of the edges in parent2Out, one parent edge at a time
  std::list<PartEdgePtr> edges;
  {
#ifndef DISABLE_SIGHT
    scope s("Cross-product");
#endif
  for(map<PartEdgePtr, map<ComposedAnalysis*, set<PartEdgePtr> > >::iterator par=parent2Out.begin();
      par!=parent2Out.end(); par++) {
    map<ComposedAnalysis*, PartEdgePtr> outPartEdges;
    assert(par->second.size()!=0);
#ifndef DISABLE_SIGHT
    dbg << "IntersectionPart::outEdges(): parent = "<<par->first->str()<<endl;
#endif
    intersectEdges(par->first, par->second.begin(), par->second, outPartEdges, edges, analysis);
  } }

  /*dbg << "edges="<<endl;
  indent ind;
  for(list<PartEdgePtr>::iterator e=edges.begin(); e!=edges.end(); e++)
    dbg << (*e)->str()<<endl;*/

  // Cache the outgoing edges in Part2InEdges
  for(std::list<PartEdgePtr>::iterator e=edges.begin(); e!=edges.end(); ++e) {
    Part2InEdges[getAnalysis()][(*e)->target()].insert(*e);
#ifndef DISABLE_SIGHT
    dbg << "IntersectionPart::outEdges(): #Part2InEdges["<<getAnalysis()->str()<<"]["<<(*e)->target()->str()<<"]="<<Part2InEdges[getAnalysis()][(*e)->target()].size()<<endl;
    dbg << "    "<<(*e)->str()<<endl;
#endif
  }

  return edges;
}

// Returns the list of incoming IntersectionPartEdge of this Part, which are the cross-product of the inEdges()
// of its sub-parts.
std::list<PartEdgePtr> IntersectionPart::inEdges()
{
  assert(initialized);

#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("IntersectionPart::inEdges()"), 2, partitionsDebugLevel)
  SIGHT_VERB(dbg << "#Part2InEdges["<<getAnalysis()->str()<<"]["<<str()<<"]="<<Part2InEdges[getAnalysis()][shared_from_this()].size()<<endl, 2, partitionsDebugLevel)
#endif

  // If we've already cached this Part's incoming edges, return the cached set
  if(Part2InEdges[getAnalysis()].find(shared_from_this()) != Part2InEdges[getAnalysis()].end()) {
    set<IntersectionPartEdgePtr>& edgesS = Part2InEdges[getAnalysis()][shared_from_this()];
    list<PartEdgePtr> edgesL;
    //scope s("inEdges");
    for(set<IntersectionPartEdgePtr>::iterator e=edgesS.begin(); e!=edgesS.end(); ++e) {
      //dbg << "    "<<(*e)->str()<<endl;
      edgesL.push_back(*e);
    }
    return edgesL;

  // Otherwise, if this Part has no incoming edges, return an empty list
  } else if(getInputPart()->inEdges().size()==0) {
    return list<PartEdgePtr>();
  // Otherwise, compute the incoming edges by calling inEdges function of the Parts inside this IntersectionPart
  } else {
    // For each part in parts, maps the parent part of each incoming part to the set of parts that share this parent
    map<PartEdgePtr, map<ComposedAnalysis*, set<PartEdgePtr> > > parent2In;
    for(map<ComposedAnalysis*, PartPtr>::iterator part=parts.begin(); part!=parts.end(); part++) {
      // Get this part's outgoing edges
      list<PartEdgePtr> in = part->second->inEdges();

      // Group these edges according to their common parent edge
      for(list<PartEdgePtr>::iterator e=in.begin(); e!=in.end(); e++)
        parent2In[(*e)->getInputPartEdge()][part->first].insert(*e);
    }

    // Create a cross-product of the edges in parent2Out, one parent edge at a time
    std::list<PartEdgePtr> edges;
    for(map<PartEdgePtr, map<ComposedAnalysis*, set<PartEdgePtr> > >::iterator par=parent2In.begin();
        par!=parent2In.end(); par++) {
      map<ComposedAnalysis*, PartEdgePtr> inPartEdges;
      intersectEdges(par->first, par->second.begin(), par->second, inPartEdges, edges, analysis);
    }
    return edges;
  }
}

/* // Recursive computation of the cross-product of the outEdges of all the sub-parts of this Intersection part.
// Hierarchically builds a recursion tree that contains more and more combinations of PartsPtr from the outEdges
// of different sub-parts. When the recursion tree reaches its full depth (one level per part in parts), it creates
// an intersection the current combination of
// partI - refers to the current part in parts
// outPartEdges - the list of outgoing edges of the current combination of this IntersectionPart's sub-parts,
//         upto partI
void IntersectionPart::outEdges_rec(std::map<ComposedAnalysis*, PartPtr>::iterator partI,
                                    std::map<ComposedAnalysis*, PartPtr> outPartEdges,
                                    std::vector<PartEdgePtr>& edges) {
  // If we've reached the last part in parts and outEdgeParts contains all the outgoing PartEdges
  if(partI == parts.end())
    edges.push_back(makePtr<IntersectionPartEdge>(outPartEdges, analysis));
  // If we haven't yet reached the end, recurse on all the outgoing edges of the current part
  else {
    // Get this part's outgoing edges
    vector<PartEdgePtr> out = (*partI)->outEdges();
    // Maps the parent part of each outgoing part to the set of parts that share this parent
    map<PartPrt, set<PartEdgePtr> > parent2Out;
    for(vector<PartEdgePtr>::iterator e=out.begin(); e!=out.end(); e++)
      parent2Out[(*e)->getInputPartEdge()].insert(*e);

    ComposedAnalysis* curAnalysis = partI->first;

    // Advance to the next part in parts
    partI++;

    // Recurse on the cross product of the outgoing edges of this part and the outgoing edges of subsequent parts
    for(vector<PartEdgePtr>::iterator e=partOut.begin(); e!=partOut.end(); e++){
      outPartEdges[curAnalysis] = *e;
      outEdges_rec(partI, outPartEdges, edges);
      outPartEdges.pop_back();
    }
  }
}*/

/*// Returns the list of incoming IntersectionPartEdge of this Part, which are the cross-product of the inEdges()
// of its sub-parts.
std::vector<PartEdgePtr> IntersectionPart::inEdges() {
  list<PartEdgePtr> inPartEdges;
  vector<PartEdgePtr> edges;
  inEdges_rec(parts.begin(), inPartEdges, edges);
  return edges;
}
// Recursive computation of the cross-product of the inEdges of all the sub-parts of this Intersection part.
// Hierarchically builds a recursion tree that contains more and more combinations of PartsPtr from the inEdges
// of different sub-parts. When the recursion tree reaches its full depth (one level per part in parts), it creates
// an intersection the current combination of
// partI - refers to the current part in parts
// inPartEdges - the list of incoming edges of the current combination of this IntersectionPart's sub-parts,
//         upto partI
void IntersectionPart::inEdges_rec(list<PartPtr>::iterator partI, list<PartEdgePtr> inPartEdges,
                                   vector<PartEdgePtr>& edges) {
  // If we've reached the last part in parts and inEdgeParts contains all the incoming PartEdges
  if(partI == parts.end())
    edges.push_back(makePtr<IntersectionPartEdge>(inPartEdges, analysis));
  // If we haven't yet reached the end, recurse on all the incoming edges of the current part
  else {
    // Get this part's incoming edges
    vector<PartEdgePtr> partIn = (*partI)->inEdges();

    // Advance to the next part in parts
    partI++;

    // Recurse on the cross product of the ingoing edges of this part and the incoming edges of subsequent parts
    for(vector<PartEdgePtr>::iterator e=partIn.begin(); e!=partIn.end(); e++){
      inPartEdges.push_back(*e);
      inEdges_rec(partI, inPartEdges, edges);
      inPartEdges.pop_back();
    }
  }
}*/

// Returns the intersection of the lists of CFGNodes returned by the Parts in parts
set<CFGNode> IntersectionPart::CFGNodes() const {
  assert(initialized);

  set<CFGNode> nodes;
  bool initializedNodes=false; // Records whether nodes has been initialized from one of the parts
  for(map<ComposedAnalysis*, PartPtr>::const_iterator part=parts.begin(); part!=parts.end(); part++) {
    // If nodes has not yet been initialized, simply copy this Part's CFGNodes to nodes
    if(!initializedNodes) {
      // Make sure to only copy if part is not NULL (not a wildcard source or destination of an edge)
      if(part->second) {
        nodes = part->second->CFGNodes();
        initializedNodes=true;
      }
    // Otherwise, remove any nodes from node that are not in (*part)->CFGNodes()
    } else {
      set<CFGNode> partNodes=part->second->CFGNodes();
      for(set<CFGNode>::const_iterator nI=nodes.begin(); nI!=nodes.end(); ) {
        bool found=false;
        for(set<CFGNode>::const_iterator pnI=partNodes.begin(); pnI!=partNodes.end(); pnI++) {
          if(nI==pnI) { found=true; break; }
        }
        // If the current element in nodes was not found in partNodes, erase it and move on to the next one
        if(!found) {
          set<CFGNode>::const_iterator nINext = nI; nINext++;
          nodes.erase(nI);
          nI = nINext;
        // If it was found, just move on to the next CFGNode in nodes
        } else
          nI++;
      }
    }
  }

  return nodes;
}

// If this Part corresponds to a function call/return, returns the set of Parts that contain
// its corresponding return/call, respectively.
set<PartPtr> IntersectionPart::matchingCallParts() const {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (txt()<<"IntersectionPart::matchingCallParts()"), 1, partitionsDebugLevel)
  SIGHT_VERB(dbg<<"this="<<str()<<endl, 1, partitionsDebugLevel)
#endif
  assert(initialized);

  // Maps inputParts to the parts derived from them by each analysis in the keys of parts.
  // The map associated with each inputPart will induce an IntersectionPart and the set of
  // these IntersectionParts will be returned.
  map<PartPtr, map<ComposedAnalysis*, PartPtr> > allMCParts;
  {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, ("Computing allMCParts"), 1, partitionsDebugLevel)
#endif
  for(map<ComposedAnalysis*, PartPtr>::const_iterator part=parts.begin(); part!=parts.end(); ++part) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"part="<<part->second->str()), 1, partitionsDebugLevel)
#endif
    set<PartPtr> matchParts = part->second->matchingCallParts();
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "#matchParts="<<matchParts.size()<<endl, 1, partitionsDebugLevel)
#endif
    for(set<PartPtr>::iterator mp=matchParts.begin(); mp!=matchParts.end(); ++mp) {
#ifndef DISABLE_SIGHT
      dbg << (*mp)->str()<<endl;
#endif
      assert(allMCParts[(*mp)->getInputPart()].find(part->first) == allMCParts[(*mp)->getInputPart()].end());
      allMCParts[(*mp)->getInputPart()][part->first] = *mp;
    }
  }}
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "#allMCParts="<<allMCParts.size()<<endl, 1, partitionsDebugLevel)
#endif

  // Verify that all the superset parts in allMCParts map the same number of Parts
  unsigned int numMappedParts=0;
  for(map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator mcp=allMCParts.begin(); mcp!=allMCParts.end(); ++mcp) {
    if(mcp==allMCParts.begin()) numMappedParts = mcp->second.size();
    else                        assert(numMappedParts == mcp->second.size());
  }
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "numMappedParts="<<numMappedParts<<endl, 1, partitionsDebugLevel)
#endif

  // The set of IntersectionParts that correspond to the mappes associated with the superset keys in allMCParts
  set<PartPtr> allMCInterParts;
  for(map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator mcp=allMCParts.begin(); mcp!=allMCParts.end(); ++mcp) {
    IntersectionPartPtr interPart = makePtr<IntersectionPart>(mcp->second, mcp->first, getAnalysis());
    interPart->init();
    allMCInterParts.insert(interPart);
  }
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "#allMCInterParts="<<allMCInterParts.size()<<endl, 1, partitionsDebugLevel)
#endif
  return allMCInterParts;

/*  set<PartPtr> matchParts;
  bool initializedMatchParts=false; // Records whether matchParts has been initialized from one of the parts
  for(map<ComposedAnalysis*, PartPtr>::const_iterator part=parts.begin(); part!=parts.end(); part++) {
    // If nodes has not yet been initialized, simply copy this Part's matchingCallParts to matchParts
    if(!initializedMatchParts) {
      // Make sure to only copy if part is not NULL (not a wildcard source or destination of an edge)
      if(part->second) {
        matchParts = part->second->matchingCallParts();
        initializedMatchParts = true;
      }
    // Otherwise, remove any nodes from node that are not in (*part)->CFGNodes()
    } else {
      set<PartPtr> partMatchParts=part->second->matchingCallParts();
      for(set<PartPtr>::const_iterator mpI=matchParts.begin(); mpI!=matchParts.end(); ) {
        bool found=false;
        for(set<PartPtr>::const_iterator pmpI=partMatchParts.begin(); pmpI!=partMatchParts.end(); pmpI++) {
          if(mpI==pmpI) { found=true; break; }
        }
        // If the current element in nodes was not found in partNodes, erase it and move on to the next one
        if(!found) {
          set<PartPtr>::const_iterator mpINext = mpI; mpINext++;
          matchParts.erase(mpI);
          mpI = mpINext;
        // If it was found, just move on to the next CFGNode in nodes
        } else
          mpI++;
      }
    }
  }
  return matchParts;*/
}

// If this Part corresponds to a function entry/exit, returns the set of Parts that contain
  // its corresponding exit/entry, respectively.
set<PartPtr> IntersectionPart::matchingEntryExitParts() const {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (txt()<<"IntersectionPart::matchingEntryExitParts()"), 1, partitionsDebugLevel)
  SIGHT_VERB(dbg<<"this="<<str()<<endl, 1, partitionsDebugLevel)
#endif
  assert(initialized);

  // Maps inputParts to the parts derived from them by each analysis in the keys of parts.
  // The map associated with each inputPart will induce an IntersectionPart and the set of
  // these IntersectionParts will be returned.
  map<PartPtr, map<ComposedAnalysis*, PartPtr> > allEEParts;
  {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, ("Computing allEEParts"), 1, partitionsDebugLevel)
#endif
  for(map<ComposedAnalysis*, PartPtr>::const_iterator part=parts.begin(); part!=parts.end(); ++part) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"part="<<part->second->str()), 1, partitionsDebugLevel)
#endif
    set<PartPtr> matchParts = part->second->matchingEntryExitParts();
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "#matchParts="<<matchParts.size()<<endl, 1, partitionsDebugLevel)
#endif
    for(set<PartPtr>::iterator mp=matchParts.begin(); mp!=matchParts.end(); ++mp) {
#ifndef DISABLE_SIGHT
      dbg << (*mp)->str()<<endl;
#endif

      assert(allEEParts[(*mp)->getInputPart()].find(part->first) == allEEParts[(*mp)->getInputPart()].end());
      allEEParts[(*mp)->getInputPart()][part->first] = *mp;
    }
  }}
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "#allEEParts="<<allEEParts.size()<<endl, 1, partitionsDebugLevel)
#endif

  // Verify that all the superset parts in allEEParts map the same number of Parts
  unsigned int numMappedParts=0;
  for(map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator eep=allEEParts.begin(); eep!=allEEParts.end(); ++eep) {
    if(eep==allEEParts.begin()) numMappedParts = eep->second.size();
    else                        assert(numMappedParts == eep->second.size());
  }
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "numMappedParts="<<numMappedParts<<endl, 1, partitionsDebugLevel)
#endif

  // The set of IntersectionParts that correspond to the mappes associated with the superset keys in allEEParts
  set<PartPtr> allEEInterParts;
  for(map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator eep=allEEParts.begin(); eep!=allEEParts.end(); ++eep) {
    IntersectionPartPtr interPart = makePtr<IntersectionPart>(eep->second, eep->first, getAnalysis());
    interPart->init();
    allEEInterParts.insert(interPart);
  }

#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "#allEEInterParts="<<allEEInterParts.size()<<endl, 1, partitionsDebugLevel)
#endif

    return allEEInterParts;
}
/*
// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    Part that denotes a subset of A (the function is called on this part),
//    it returns a list of Parts that partition O.
std::list<PartPtr> IntersectionPart::getOperandPart(SgNode* anchor, SgNode* operand)
{
  list<PartPtr> accumOperandParts;
  list<PartPtr> allParts;
  getOperandPart_rec(anchor, operand, parts.begin(), accumOperandParts, allParts);
  return allParts;
}
// Recursive computation of the cross-product of the getOperandParts of all the sub-parts of this Intersection part.
// Hierarchically builds a recursion tree that contains more and more combinations of PartPtrs from the results of
// getOperandPart of different sub-parts. When the recursion tree reaches its full depth (one level per part in parts),
// it creates an intersection the current combination of parts
// partI - refers to the current part in parts
// accumOperandParts - the list of incoming parts of the current combination of this IntersectionPart's sub-parts,
//         upto partI
void IntersectionPart::getOperandPart_rec(SgNode* anchor, SgNode* operand,
                                          list<PartPtr>::iterator partI, list<PartPtr> accumOperandParts,
                                          list<PartPtr>& allParts)
{
  // If we've reached the last part in parts and accumOperandParts contains all the parts for the current combination
  if(partI == parts.end())
    allParts.push_back(makePtr<IntersectionPart>(accumOperandParts));
  // If we haven't yet reached the end, recurse on all the incoming edges of the current part
  else {
    // Get this part's incoming edges
    list<PartPtr> operandParts = (*partI)->getOperandPart(anchor, operand);

    // Advance to the next part in parts
    partI++;

    // Recurse on the cross product of the ingoing edges of this part and the incoming edges of subsequent parts
    for(list<PartPtr>::iterator opP=operandParts.begin(); opP!=operandParts.end(); opP++){
      accumOperandParts.push_back(*opP);
      getOperandPart_rec(anchor, operand, partI, accumOperandParts, allParts);
      accumOperandParts.pop_back();
    }
  }
}*/

// Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
PartEdgePtr IntersectionPart::inEdgeFromAny()
{
  assert(initialized);
  //cout << "IntersectionPart::inEdgeFromAny() getInputPart()="<<getInputPart()->str()<<endl;
  IntersectionPartEdgePtr iEdge =
      makePtr<IntersectionPartEdge>(/*isInEdgeFromAny*/ true, /*outEdgeToAny*/ false,
                                       /*sourceTargetPart*/ shared_from_this(),
                                       /*parent*/ (getInputPart()? getInputPart()->inEdgeFromAny() : NULLPartEdge),
                                       /*analysis*/ getAnalysis());
  iEdge->init();
  return iEdge;
/*  // Collect the incoming edges from each sub-part and intersect them
  map<ComposedAnalysis*, PartEdgePtr> edges;
  for(map<ComposedAnalysis*, PartPtr>::iterator part=parts.begin(); part!=parts.end(); part++) {
    PartPtr ps = part->second;
    edges[part->first] = part->second->inEdgeFromAny();
  }

  return makePtr<IntersectionPartEdge>(edges, (getInputPart()? getInputPart()->inEdgeFromAny() : NULLPartEdge), analysis);*/
}

// Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
PartEdgePtr IntersectionPart::outEdgeToAny()
{
  assert(initialized);

  /*scope s("IntersectionPart::outEdgeToAny()");
  dbg << "this="<<str()<<endl;
  dbg << "getInputPart()="<<getInputPart()->str()<<endl;
  dbg << "getInputPart()->outEdgeToAny()="<<getInputPart()->outEdgeToAny()->str()<<endl;*/

  IntersectionPartEdgePtr iEdge =
      makePtr<IntersectionPartEdge>(/*isInEdgeFromAny*/ false, /*outEdgeToAny*/ true,
                                       /*sourceTargetPart*/ shared_from_this(),
                                       /*parent*/ (getInputPart()? getInputPart()->outEdgeToAny() : NULLPartEdge),
                                       /*analysis*/ getAnalysis());
  iEdge->init();
  return iEdge;
/*  // Collect the outgoing edges from each sub-part and intersect them
  map<ComposedAnalysis*, PartEdgePtr> edges;
  for(map<ComposedAnalysis*, PartPtr>::iterator part=parts.begin(); part!=parts.end(); part++)
    edges[part->first] = part->second->outEdgeToAny();
  return makePtr<IntersectionPartEdge>(edges, (getInputPart()? getInputPart()->outEdgeToAny() : NULLPartEdge), analysis);*/
}

// Two IntersectionParts are equal if their parents and all their constituent sub-parts are equal
bool IntersectionPart::equal(const PartPtr& that_arg) const
{
  assert(initialized);

  IntersectionPartPtr that = dynamicPtrCast<IntersectionPart>(that_arg);
  /*IntersectionPart copy(parts, getInputPart(), analysis);
  dbg << "IntersectionPart::equal("<<copy.str()<<", "<<that->str()<<")"<<endl;*/

  // Two intersection parts with different numbers of sub-parts are definitely not equal
  if(parts.size() != that->parts.size()) { /*dbg << "NOT EQUAL: size\n"; */return false; }

  // Two intersections with different parents are definitely not equal
  if(getInputPart() != that->getInputPart()) { /*dbg << "NOT EQUAL: parents\n"; */return false; }

  for(map<ComposedAnalysis*, PartPtr>::const_iterator thisIt=parts.begin(), thatIt=that->parts.begin();
      thisIt!=parts.end(); thisIt++) {
    assert(thisIt->first == thatIt->first);
    if(thisIt->second != thatIt->second) { /*dbg << "NOT EQUAL\n"; */return false; }
  }

  //dbg << "EQUAL: size\n";
  return true;
}

// Lexicographic ordering: This IntersectionPart is < that IntersectionPart if
// - their parents are < ordered, OR
// - if this has fewer parts than that, OR
// - there exists an index i in this.parts and that.parts s.t. forall j<i. this.parts[j]==that.parts[j] and
//   this.parts[i] < that.parts[i].
bool IntersectionPart::less(const PartPtr& that_arg) const
{
  assert(initialized);
  /*scope s(txt()<<"IntersectionPart::less()");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that_arg->str()<<endl;*/
  IntersectionPartPtr that = dynamicPtrCast<IntersectionPart>(that_arg);
  /*IntersectionPart copy(parts, getInputPart(), analysis);
  dbg << "IntersectionPart::less("<<copy.str()<<", "<<that->str()<<")"<<endl;*/

  // If parents are properly ordered, use their ordering
  if(getInputPart() < that->getInputPart()) { /*dbg << "LESS-THAN: parent\n";*/ return true; }
  if(getInputPart() > that->getInputPart()) { /*dbg << "GREATER-THAN: parent\n";*/ return false; }

  // If this has fewer parts than that, it is ordered before it
  if(parts.size() < that->parts.size()) { /*dbg << "LESS-THAN: size\n";*/ return true; }
  // If greater number of parts, it is order afterwards
  if(parts.size() > that->parts.size()) { /*dbg << "GREATER-THAN: size\n";*/ return false; }

  // Keep iterating for as long as the sub-parts are equal and declare this < that if we find
  // a sub-part in this < the corresponding sub-part in that
  for(map<ComposedAnalysis*, PartPtr>::const_iterator thisIt=parts.begin(), thatIt=that->parts.begin();
      thisIt!=parts.end(); thisIt++) {
    assert(thisIt->first == thatIt->first);
    if(thisIt->second < thatIt->second) { /*dbg << "LESS-THAN\n";*/ return true; }
    else if(thisIt->second > thatIt->second) { /*dbg << "GREATER-THAN\n"; */return false; }
  }

  // If the lexicographic < condition was not met then this is not < than that
  //dbg << "NOT LESS THAN\n";
  return false;
}

std::string IntersectionPart::str(std::string indent) const
{
  assert(initialized);

  ostringstream oss;
  oss << "[IntersectionPart:";
  if(parts.size() > 1) oss << endl;
  for(map<ComposedAnalysis*, PartPtr>::const_iterator part=parts.begin(); part!=parts.end(); ) {
    if(parts.size() > 1) oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
    oss << (part->second? part->second->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;") : "NULL");
    part++;
    if(part!=parts.end()) oss << endl;
  }
  oss << "]"; //", parent="<<(getInputPart()? getInputPart()->str(): "NULL")<<", analysis="<<analysis<<"]";
  return oss.str();
}

// Given a map of sets of PartPtrs from multiple analyses, returns a set of corresponding IntersectionParts.
// Two Parts appear within the same IntersectionPart if they have the same parent Part. If a given parent
// Part has derived parts in sets associated with some but not all analyses, these derived parts are excluded
// from the set returned by this function.
std::set<PartPtr> createIntersectionPartSet(const std::map<ComposedAnalysis*, std::set<PartPtr> >& subParts,
                                            ComposedAnalysis* analysis) {
  /*scope reg("IntersectionPart::outEdges", scope::high);*/
  // Maps each parent Part to the Parts in subParts that derive from it, one per ComposedAnalysis
  map<PartPtr, map<ComposedAnalysis*, PartPtr> > parent2Parts;
  for(map<ComposedAnalysis*, set<PartPtr> >::const_iterator s=subParts.begin(); s!=subParts.end(); ++s) {
    for(set<PartPtr>::const_iterator p=s->second.begin(); p!=s->second.end(); ++p) {
      PartPtr parent = (*p)->getInputPart();
      assert(parent2Parts[parent].find(s->first) == parent2Parts[parent].end());
      parent2Parts[parent][s->first] = *p;
    }
  }

  // The set of IntersectionParts for each parent Part that is mapped to one Part for each ComposedAnalysis
  set<PartPtr> interParts;
  for(map<PartPtr, map<ComposedAnalysis*, PartPtr> >::const_iterator p=parent2Parts.begin(); p!=parent2Parts.begin(); ++p) {
    // If there is one sub-Part for the current parent Part for each ComposedAnalysis
    if(p->second.size() == subParts.size()) {
      /* // Convert the map to a set of Parts
      set<PartPtr> analysisParts;
      for(map<ComposedAnalysis*, PartPtr>::const_iterator i=p->second.begin(); i!=p->second.end(); ++i)
        analysisParts.insert(i->second);*/

      // Create an IntersectionPart for this parent Part and add it to interParts
      IntersectionPartPtr newPart = makePtr<IntersectionPart>(p->second, p->first, analysis);
      newPart->init();
      interParts.insert(newPart);
    }
  }

  return interParts;
}

/* ################################
   ##### IntersectionPartEdge #####
   ################################ */
/*IntersectionPartEdge::IntersectionPartEdge(PartEdgePtr edge, ComposedAnalysis* analysis) :
    PartEdge(analysis)
{ edges.push_back(edge); }*/

IntersectionPartEdge::IntersectionPartEdge(const map<ComposedAnalysis*, PartEdgePtr>& edges,
                                           PartEdgePtr parent, ComposedAnalysis* analysis) :
    PartEdge(analysis, parent), edges(edges),
    isInEdgeFromAny(false), isOutEdgeToAny(false)
{}

// Constructs an edge where either the source or the target is a wildcard
// isInEdgeFromAny/isOutEdgeToAny: indicates which side of the edge is a wildcard
// sourceTargetPart: if isInEdgeFromAny==true, this is the edge's target Part
//                   if isOutEdgeToAny==true, this is the edge's source Part
IntersectionPartEdge::IntersectionPartEdge(bool isInEdgeFromAny, bool isOutEdgeToAny, IntersectionPartPtr sourceTargetPart,
                                           PartEdgePtr parent, ComposedAnalysis* analysis) :
    PartEdge(analysis, parent),
    isInEdgeFromAny(isInEdgeFromAny), isOutEdgeToAny(isOutEdgeToAny)
{
  assert((isInEdgeFromAny && !isOutEdgeToAny) || (!isInEdgeFromAny && isOutEdgeToAny));
  if(isInEdgeFromAny) cachedTarget = sourceTargetPart;
  else                cachedSource = sourceTargetPart;
}

// Returns the PartEdge associated with this analysis. If the analysis does not implement the partition graph
// (is not among the keys of parts), returns the parent PartEdge.
PartEdgePtr IntersectionPartEdge::getPartEdge(ComposedAnalysis* analysis)
{
  //dbg << "IntersectionPartEdge::getPartEdge(analysis="<<analysis<<" = "<<analysis->str()<<") #edges="<<edges.size()<<endl;
  if(isInEdgeFromAny) {
    assert(cachedTarget);
    if(cachedTarget->parts.find(analysis)!=cachedTarget->parts.end()) return cachedTarget->parts[analysis]->inEdgeFromAny();
    else return getInputPartEdge();
  } else if(isOutEdgeToAny) {
    assert(cachedSource);
    if(cachedSource->parts.find(analysis)!=cachedSource->parts.end()) return cachedSource->parts[analysis]->outEdgeToAny();
    else return getInputPartEdge();
  } else {
/*  for(map<ComposedAnalysis*, PartEdgePtr>::iterator e = edges.begin(); e!=edges.end(); e++)
  {
    PartEdgePtr es = e->second;
    dbg << "&nbsp;&nbsp;&nbsp;&nbsp;"<<e->first<< " : " << e->first->str() << " : " << es->str() << endl;
  }*/

  //for(map<ComposedAnalysis*, PartEdgePtr>::iterator edge=edges.begin(); edge!=edges.end(); edge
  //map<ComposedAnalysis*, PartEdgePtr>::iterator edge;
    if(edges.find(analysis)!=edges.end()) { /*dbg << "found edge "<<edges[analysis]->str()<<endl;*/ return edges[analysis]; }
    else { /*dbg << "not found. parent="<<getInputPartEdge()->str()<<endl; */return getInputPartEdge(); }
  }
}

// Return the part that intersects the sources of all the sub-edges of this IntersectionPartEdge
PartPtr IntersectionPartEdge::source() const {
  if(isInEdgeFromAny) {
    return NULLPart;
  } else if(isOutEdgeToAny) {
    assert(cachedSource);
    return cachedSource;
  } else {
    map<ComposedAnalysis*, PartPtr> sourceParts;
    bool allNULL=true; // True if all the source parts of the sub-edges are NULL, false otherwise.
    PartPtr srcParent;
    for(map<ComposedAnalysis*, PartEdgePtr>::const_iterator e=edges.begin(); e!=edges.end(); e++) {
      PartPtr s = e->second->source();
      // Make sure that the parents of source parts are consistent
      if(e == edges.begin()) {
        if(s) {
          srcParent = s->getInputPart();
          allNULL=false;
        }
      } else {
        // Either all sources are NULL or none are
        assert((allNULL && !s) || (!allNULL && s));
        if(s)
          // All parents must be consistent
          assert(srcParent == s->getInputPart());
      }

      sourceParts[e->first] = s;
    }
    if(allNULL) return NULLPart;
    else  {
      IntersectionPartPtr newPart = makePtr<IntersectionPart>(sourceParts, srcParent, getAnalysis());
      newPart->init();
      return newPart;
    }
  }
}

// Return the part that intersects the targets of all the sub-edges of this IntersectionPartEdge
PartPtr IntersectionPartEdge::target() const {
  if(isInEdgeFromAny) {
    assert(cachedTarget);
    return cachedTarget;
  } else if(isOutEdgeToAny) {
    return NULLPart;
  } else {
    map<ComposedAnalysis*, PartPtr> targetParts;
    bool allNULL=true; // True if all the target parts of the sub-edges are NULL, false otherwise.
    PartPtr tgtParent;
    for(map<ComposedAnalysis*, PartEdgePtr>::const_iterator e=edges.begin(); e!=edges.end(); e++) {
      PartPtr t = e->second->target();
      // Make sure that the parents of source parts are consistent
      if(e == edges.begin()) {
        if(t) {
          tgtParent = t->getInputPart();
          allNULL=false;
        }
      } else {
        // Either all sources are NULL or none are
        assert((allNULL && !t) || (!allNULL && t));
        if(t)
          // All parents must be consistent
          assert(tgtParent == t->getInputPart());
      }

      targetParts[e->first] = t;
    }
    if(allNULL) return NULLPart;
    else {
      IntersectionPartPtr newPart = makePtr<IntersectionPart>(targetParts, tgtParent, getAnalysis());
      newPart->init();
      return newPart;
    }
  }
}


// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    PartEdge that denotes a subset of A (the function is called on this PartEdge),
//    it returns a list of PartEdges that partition O.
/*std::list<PartEdgePtr> IntersectionPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand)
{
  scope s(txt()<<"IntersectionPartEdge::getOperandPartEdge(isInEdgeFromAny="<<isInEdgeFromAny<<")");
  if(isInEdgeFromAny) {
    list<PartEdgePtr> in = cachedTarget->inEdges();
    set<PartEdgePtr> resS;
    for(list<PartEdgePtr>::iterator i=in.begin(); i!=in.end(); ++i) {
      scope s2(txt()<<"InEdge "<<(*i)->str());
      list<PartEdgePtr> ope = (*i)->getOperandPartEdge(anchor, operand);
      for(list<PartEdgePtr>::iterator j=ope.begin(); j!=ope.end(); ++j)
        resS.insert(*j);
    }

    list<PartEdgePtr> resL;
    for(set<PartEdgePtr>::iterator k=resS.begin(); k!=resS.end(); ++k)
      resL.push_back(*k);

    return resL;
  }
  if(isOutEdgeToAny)  assert(0);

   // For each part in parts, maps the parent part of each operand part edge to the set of parts that share this parent
  map<PartEdgePtr, map<ComposedAnalysis*, set<PartEdgePtr> > > parent2OPE;
  for(map<ComposedAnalysis*, PartEdgePtr>::iterator edge=edges.begin(); edge!=edges.end(); edge++) {
    scope s3(txt()<<"edge "<<edge->second->str());
    // Get this part's outgoing edges
    list<PartEdgePtr> ope = edge->second->getOperandPartEdge(anchor, operand);

    // Group these edges according to their common parent edge
    for(list<PartEdgePtr>::iterator e=ope.begin(); e!=ope.end(); e++)
      parent2OPE[(*e)->getInputPartEdge()][edge->first].insert(*e);
  }

  // Create a cross-product of the edges in parent2Out, one parent edge at a time
  std::list<PartEdgePtr> edges;
  for(map<PartEdgePtr, map<ComposedAnalysis*, set<PartEdgePtr> > >::iterator par=parent2OPE.begin();
      par!=parent2OPE.end(); par++) {
    map<ComposedAnalysis*, PartEdgePtr> opePartEdges;
    intersectEdges(par->first, par->second.begin(), par->second, opePartEdges, edges, analysis);
  }
  return edges;
}*/


/*std::list<PartEdgePtr> IntersectionPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand)
{
  list<PartEdgePtr> accumOperandPartEdges;
  list<PartEdgePtr> allPartEdges;
  getOperandPartEdge_rec(anchor, operand, edges.begin(), accumOperandPartEdges, allPartEdges);
  return allPartEdges;
}

// Recursive computation of the cross-product of the getOperandParts of all the sub-part edges of this Intersection part edge.
// Hierarchically builds a recursion tree that contains more and more combinations of PartEdgePtrs from the results of
// getOperandPart of different sub-part edges. When the recursion tree reaches its full depth (one level per edge in edges),
// it creates an intersection the current combination of edges.
// edgeI - refers to the current edge in edges
// accumOperandPartEdges - the list of incoming edgesof the current combination of this IntersectionPartEdges's sub-Edges,
//         upto edgeI
void IntersectionPartEdge::getOperandPartEdge_rec(SgNode* anchor, SgNode* operand,
                                                  list<PartEdgePtr>::iterator edgeI, list<PartEdgePtr> accumOperandPartEdges,
                                                  list<PartEdgePtr>& allPartEdges)
{
  // If we've reached the last edge in edges and accumOperandPartEdges contains all the edges for the current combination
  if(edgeI == edges.end())
    allPartEdges.push_back(makePtr<IntersectionPartEdge>(accumOperandPartEdges, analysis));
  // If we haven't yet reached the end, recurse on all the incoming edges of the current edge
  else {
    // Get this edge's incoming edges
    list<PartEdgePtr> operandPartEdges = (*edgeI)->getOperandPartEdge(anchor, operand);

    // Advance to the next edge in edges
    edgeI++;

    // Recurse on the cross product of the ingoing edges of this edge and the incoming edges of subsequent edges
    for(list<PartEdgePtr>::iterator opP=operandPartEdges.begin(); opP!=operandPartEdges.end(); opP++){
      accumOperandPartEdges.push_back(*opP);
      getOperandPartEdge_rec(anchor, operand, edgeI, accumOperandPartEdges, allPartEdges);
      accumOperandPartEdges.pop_back();
    }
  }
}*/

// If the source Part corresponds to a conditional of some sort (if, switch, while test, etc.)
// it must evaluate some predicate and depending on its value continue, execution along one of the
// outgoing edges. The value associated with each outgoing edge is fixed and known statically.
// getPredicateValue() returns the value associated with this particular edge. Since a single
// Part may correspond to multiple CFGNodes getPredicateValue() returns a map from each CFG node
// within its source part that corresponds to a conditional to the value of its predicate along
// this edge.
std::map<CFGNode, boost::shared_ptr<SgValueExp> > IntersectionPartEdge::getPredicateValue()
{
  if(source()) {
    // The set of CFGNodes for which we'll create a value mapping since these nodes exist
    // in all the sub-edges of this IntersectionPartEdge
    set<CFGNode> srcNodes = source()->CFGNodes();

    map<CFGNode, boost::shared_ptr<SgValueExp> > pv;
    // Consider the predicate->value mappings of all the sub-edges
    for(map<ComposedAnalysis*, PartEdgePtr>::iterator e=edges.begin(); e!=edges.end(); e++) {
      map<CFGNode, boost::shared_ptr<SgValueExp> > epv = e->second->getPredicateValue();
      // Consider the values mapped under all the CFG nodes of this sub-edge's source part
      for(map<CFGNode, boost::shared_ptr<SgValueExp> >::iterator v=epv.begin(); v!=epv.end(); v++) {
        // Skip CFGNodes that are not shared but all the sources of all the sub-edges
        if(srcNodes.find(v->first) == srcNodes.end()) { continue; }

        // If a value mapping for the current CFGNode of the current sub-edge has already
        // been observed from another sub-edge, make sure that the mapped values are the same
        if(pv.find(v->first) != pv.end())
          assert(ValueObject::equalValueExp(pv[v->first].get(), v->second.get()));
        else
          pv[v->first] = v->second;
      }
    }

    return pv;
  } else {
    std::map<CFGNode, boost::shared_ptr<SgValueExp> > empty;
    return empty;
  }
}

// Two IntersectionPartEdges are equal of all their constituent sub-parts are equal
bool IntersectionPartEdge::equal(const PartEdgePtr& o) const
{
  /*scope s("IntersectionPartEdge::equal()");
  dbg << "this="<<str()<<endl;
  dbg << "o="<<o->str()<<endl;*/
  IntersectionPartEdgePtr that = dynamicPtrCast<IntersectionPartEdge>(o);
/*  if(isInEdgeFromAny != that->isInEdgeFromAny) return false;
  if(isOutEdgeToAny  != that->isOutEdgeToAny)  return false;

  if(isInEdgeFromAny) {
    assert(cachedTarget);
    assert(that->cachedTarget);
    return cachedTarget == that->cachedTarget;
  }

  if(isOutEdgeToAny) {
    assert(cachedSource);
    assert(that->cachedSource);
    return cachedSource == that->cachedSource;
  }
*/
  /*IntersectionPartEdge copy(edges, getInputPartEdge(), analysis);
  dbg << "IntersectionPartEdge::equal("<<copy.str()<<", "<<that->str()<<")"<<endl;*/
  // Two intersection parts with different numbers of sub-parts are definitely not equal
  if(edges.size() != that->edges.size()) { /*dbg << "NOT EQUAL: size\n"; */return false; }

  // Two intersections with different parents are definitely not equal
  if(getInputPartEdge() != that->getInputPartEdge()) { /*dbg << "NOT EQUAL: parents\n"; */return false; }

  for(map<ComposedAnalysis*, PartEdgePtr>::const_iterator thisIt=edges.begin(), thatIt=that->edges.begin();
      thisIt!=edges.end(); thisIt++) {
    if(*thisIt != *thatIt) { /*dbg << "NOT EQUAL\n"; */return false; }
  }

  //dbg << "EQUAL\n";
  return true;
}

// Lexicographic ordering: This IntersectionPartEdge is < that IntersectionPartEdge if this has fewer edges than that or
// there exists an index i in this.edges and that.edges s.t. forall j<i. this.edges[j]==that.edges[j] and
// this.edges[i] < that.edges[i].
bool IntersectionPartEdge::less(const PartEdgePtr& o) const
{
  IntersectionPartEdgePtr that = dynamicPtrCast<IntersectionPartEdge>(o);
  /*IntersectionPartEdge copy(edges, getInputPartEdge(), analysis);
  dbg << "IntersectionPartEdge::less("<<copy.str()<<", "<<that->str()<<")"<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;getInputPartEdge()="<<getInputPartEdge()->str()<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;that->getInputPartEdge()="<<that->getInputPartEdge()->str()<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;< "<<(getInputPartEdge() < that->getInputPartEdge())<<", == "<<(getInputPartEdge() == that->getInputPartEdge())<<", > "<<(getInputPartEdge() > that->getInputPartEdge())<<endl;
  */
  // If parents are properly ordered, use their ordering
  if(getInputPartEdge() < that->getInputPartEdge()) { /*dbg << "LESS-THAN: parent\n";*/ return true; }
  if(getInputPartEdge() > that->getInputPartEdge()) { /*dbg << "GREATER-THAN: parent\n";*/ return false; }

  // If this has fewer edges than that, it is ordered before it
  if(edges.size() < that->edges.size()) { /*dbg << "LESS-THAN: size\n";*/ return true; }
  // If greater number of edges, it is order afterwards
  if(edges.size() > that->edges.size()) { /*dbg << "GREATER-THAN: size\n";*/ return false; }

  // Keep iterating for as long as the sub-edges are equal and declare this < that if we find
  // a sub-edge in this < the corresponding sub-edge in that
  for(map<ComposedAnalysis*, PartEdgePtr>::const_iterator thisIt=edges.begin(), thatIt=that->edges.begin();
      thisIt!=edges.end(); thisIt++) {
    if(*thisIt < *thatIt) { /*dbg << "LESS-THAN\n"; */return true; }
    else if(*thisIt > *thatIt) { /*dbg << "GREATER-THAN\n"; */return false; }
  }

  // If the lexicographic < condition was not met then this is not < than that
  /*dbg << "NOT LESS THAN\n"; */
  return false;
}

// Recursive function that uses a binary tree to check the equality of the remapping functors.
// Returns whether the functors between iterators start and end are equal to each other.
// numElts is the total number of map elements between start and end (not including end)
bool IntersectionPartEdge::isEqualRemap
                 (map<ComposedAnalysis*, PartEdgePtr>::const_iterator start,
                  map<ComposedAnalysis*, PartEdgePtr>::const_iterator end,
                  int numElts) const
{
  if(numElts<=1) return true;
  else if(numElts==2) {
    map<ComposedAnalysis*, PartEdgePtr>::const_iterator next = start; next++;
    return start->second->getRemap() == next->second->getRemap();
  } else {
    // Divide the current scope of the map into two almost equal halves
    map<ComposedAnalysis*, PartEdgePtr>::const_iterator cur = start;
    // Advance cur to the midpoint of the scope [start, end)
    for(int i=0; i<numElts/2; i++, cur++) {}

    return isEqualRemap(start, cur, numElts/2) &&
           isEqualRemap(cur, end, numElts-numElts/2);
  }
}

// Remaps the given Lattice as needed to take into account any function call boundaries.
// Remapping is performed both in the forwards and backwards directions.
// Returns the resulting Lattice object, which is freshly allocated.
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    some MemLocs are in scope on one side of Part, while others are in scope on the other side.
//    fromPEdge is the edge from which control is passing and the current PartEdge (same as the PartEdge of
//    the Lattice) is the one to which control is passing.
/*Lattice* IntersectionPartEdge::forwardRemapML(Lattice* lat, PartEdgePtr fromPEdge, ComposedAnalysis* client)
{
  if(isInEdgeFromAny) assert(0);
  if(isOutEdgeToAny)  assert(0);

  assert(edges.size()>0);
  // Confirm that all the sub-edges use the same remapping functors
  assert(isEqualRemap(edges.begin(), edges.end(), edges.size()));
  assert(makePtrFromThis(shared_from_this()) == lat->getPartEdge());

  // Since all the edges have the same remapper, call the remapper of the first one
  //return edges.begin()->second->forwardRemapML(lat, fromPEdge, client);
  return edges.begin()->second->forwardRemapML(lat, fromPEdge, client);
}

Lattice* IntersectionPartEdge::backwardRemapML(Lattice* lat, PartEdgePtr fromPEdge, ComposedAnalysis* client)
{
  if(isInEdgeFromAny) assert(0);
  if(isOutEdgeToAny)  assert(0);

  assert(edges.size()>0);
  // Confirm that all the sub-edges use the same remapping functors
  assert(isEqualRemap(edges.begin(), edges.end(), edges.size()));
  assert(makePtrFromThis(shared_from_this()) == lat->getPartEdge());

  // Since all the edges have the same remapper, call the remapper of the first one
  return edges.begin()->second->backwardRemapML(lat, fromPEdge, client);
}*/

std::string IntersectionPartEdge::str(std::string indent) const
{
  ostringstream oss;
  oss << "[IntersectionPartEdge:";
  if(isInEdgeFromAny)      { assert(cachedTarget); oss << "* ==> "<<cachedTarget->str()<<"]"; }
  else if(isOutEdgeToAny)  { assert(cachedSource); oss << cachedSource->str()<<" ==> *]"; }
  else {
    if(edges.size() > 1) oss << endl;
    for(map<ComposedAnalysis*, PartEdgePtr>::const_iterator edge=edges.begin(); edge!=edges.end(); ) {
      if(edges.size() > 1) oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
      oss << edge->second->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;");
      edge++;
      if(edge!=edges.end()) oss << endl;
    }
    //oss << "]"; //", parent="<<(getInputPartEdge()? getInputPartEdge()->str(): "NULL")<<", analysis="<<analysis<<"]";
    oss << ", parent="<<(getInputPartEdge()? getInputPartEdge()->str(): "NULL")<<", analysis="<<analysis->str()<<"]";
  }

  return oss.str();
}

/*
 !!! These ended up being useless and are now deprecated !!!

/ **************************************
 ***** Identity Part and PartEdge *****
 ************************************** /

/ ************************
 ***** IdentityPart *****
 ************************ /

IdentityPart::IdentityPart(PartPtr base):
    Part(base->getAnalysis(), NULLPart),
    base(base)
{ assert(base); }
IdentityPart::IdentityPart(const IdentityPart& that): Part(that) {
  base = that.base;//->copy();
}

// Function that will always be called after a Part is created and before it is returned
// to the caller. It is called outside of the Part constructor, which makes it possible to
// place code inside that registers a shared pointer to this Part with a directory of some kind.
void IdentityPart::init() { base->init(); }

IdentityPart::~IdentityPart() {}

// Returns true if this Part comes from the same analysis as that Part and false otherwise
bool IdentityPart::compatible(const Part& that_arg) {
  const IdentityPart& that = dynamic_cast<const IdentityPart&>(that_arg);
  return base->compatible(that.base);
}
bool IdentityPart::compatible(PartPtr that_arg) {
  IdentityPartPtr that = dynamicPtrCast<IdentityPart>(that_arg);
  assert(that);
  return base->compatible(that->base);
}

ComposedAnalysis* IdentityPart::getAnalysis() const { return base->getAnalysis(); }

// Returns the Part from which this Part refines (it is this part's superset). This function documents
// the hierarchical descent of this Part and makes it possible to find the common parent of parts
// derived from different analyses.
// Returns NULLPart if this part has no supersets (i.e. it is implemented by the syntactic analysis)
PartPtr IdentityPart::getInputPart() const { return ((IdentityPart*)this)->shared_from_this(); }

// Sets this Part's superset
void IdentityPart::setInputPart(PartPtr inputPart) { assert(0); }

// Returns the superset Part of this Part, while guaranteeing that the superset is not identical
// to this Part. When we wrap Parts with IdentityParts it is convenient to say that the superset
// of the identity part is itself. This method must return the parent of the identity part, not
// the identity part itself.
PartPtr IdentityPart::getInputPart() const { return base; }

// Returns the Part where the lattices computed by the analysis that implements this Part may
// be found. This Part should never be NULL.
PartPtr IdentityPart::getNodeStateLocPart() const { return ((IdentityPart*)this)->shared_from_this();/ *base;* / }

// Sets this Part's NodeStateLocPart
void IdentityPart::setNodeStateLocPart(PartPtr NodeStateLocPart) { assert(0); }

// Returns the list of this Part's outgoing edges. These edges may be computed incrementally
// as the analysis runs and thus, this function should not be called by analyses that implement
// the ATS inside their transfer function, since they may end up calling the outEdges() function
// before they've computed these edges
std::list<PartEdgePtr> IdentityPart::outEdges() {
  list<PartEdgePtr> baseEdges = base->outEdges();
  list<PartEdgePtr> wrappedEdges;
  for(list<PartEdgePtr>::iterator be=baseEdges.begin(); be!=baseEdges.end(); ++be) {
    if(*be) wrappedEdges.push_back(makePtr<IdentityPartEdge>(*be));
    else    wrappedEdges.push_back(NULLPartEdge);
  }
  return wrappedEdges;
}

std::list<PartEdgePtr> IdentityPart::inEdges() {
  list<PartEdgePtr> baseEdges = base->inEdges();
  list<PartEdgePtr> wrappedEdges;
  for(list<PartEdgePtr>::iterator be=baseEdges.begin(); be!=baseEdges.end(); ++be) {
    if(*be) wrappedEdges.push_back(makePtr<IdentityPartEdge>(*be));
    else    wrappedEdges.push_back(NULLPartEdge);
  }
  return wrappedEdges;
}

std::set<CFGNode> IdentityPart::CFGNodes() const { return base->CFGNodes(); }

// If this Part corresponds to a function call/return, returns the set of Parts that contain
// its corresponding return/call, respectively.
std::set<PartPtr> IdentityPart::matchingCallParts() const {
  set<PartPtr> baseParts = base->matchingCallParts();
  set<PartPtr> wrappedParts;
  for(set<PartPtr>::iterator bp=baseParts.begin(); bp!=baseParts.end(); ++bp) {
    if(*bp) wrappedParts.insert(makePtr<IdentityPart>(*bp));
    else    wrappedParts.insert(NULLPart);
  }
  return wrappedParts;
}

// Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
PartEdgePtr IdentityPart::inEdgeFromAny() {
  PartEdgePtr pe = base->inEdgeFromAny();
  if(pe) return makePtr<IdentityPartEdge>(pe);
  else   return NULLPartEdge;
}

// Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
PartEdgePtr IdentityPart::outEdgeToAny() {
  PartEdgePtr pe = base->outEdgeToAny();
  if(pe) return makePtr<IdentityPartEdge>(pe);
  else   return NULLPartEdge;
}

// Returns the context that includes this Part and its ancestors.
ContextPtr IdentityPart::getContext()
{ return base->getContext(); }

// Returns the specific context of this Part. Can return the NULLPartContextPtr if this
// Part doesn't implement a non-trivial context.
PartContextPtr IdentityPart::getPartContext() const
{ return base->getPartContext(); }

// Returns whether both this and that parts have the same context and their CFGNode lists consist
// exclusively of matching pairs of outgoing and incoming function calls (for each outgoing call in one
// list there's an incoming call in the other and vice versa).
bool IdentityPart::mustMatchFuncCall(PartPtr that_arg) {
  IdentityPartPtr that = dynamicPtrCast<IdentityPart>(that_arg);
  assert(that);
  return base->mustMatchFuncCall(that);
}

// Returns whether both this and that parts have the same context and their CFGNode lists include some
// matching pairs of outgoing and incoming function calls.
bool IdentityPart::mayMatchFuncCall(PartPtr that_arg) {
  IdentityPartPtr that = dynamicPtrCast<IdentityPart>(that_arg);
  assert(that);
  return base->mayMatchFuncCall(that);
}

// The the base equality and comparison operators are implemented in Part and these functions
// call the equality and inequality test functions supplied by derived classes as needed

// If this and that come from the same analysis, call the type-specific equality test implemented
// in the derived class. Otherwise, these Parts are not equal.
bool IdentityPart::equal(const PartPtr& that_arg) const {
  / *scope s("IdentityPart::equal");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that_arg->str()<<endl;* /
  IdentityPartPtr that = dynamicPtrCast<IdentityPart>(that_arg);
  //dbg << "equal="<<base->equal(that->base)<<endl;
  assert(that);
  return base->equal(that->base);
}

// If this and that come from the same analysis, call the type-specific inequality test implemented
// in the derived class. Otherwise, determine inequality by comparing the analysis pointers.
bool IdentityPart::less(const PartPtr& that_arg) const
{
  / *scope s("IdentityPart::less");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that_arg->str()<<endl;* /
  IdentityPartPtr that = dynamicPtrCast<IdentityPart>(that_arg);
  assert(that);
  //dbg << "less="<<base->less(that->base)<<endl;
  return base->less(that->base);
}

std::string IdentityPart::str(std::string indent) const
{ return string(txt()<<"[IdentityPart:"<<base->str(indent)<<"]"); }


/ ****************************
 ***** IdentityPartEdge *****
 **************************** /

IdentityPartEdge::IdentityPartEdge(PartEdgePtr base) :
    PartEdge(base->getAnalysis(), NULLPartEdge, NULLPartEdge),
    base(base)
{ assert(base); }
IdentityPartEdge::IdentityPartEdge(const IdentityPartEdge& that): PartEdge(that) {
  base = that.base;//->copy();
}

// Function that will always be called after a PartEdge is created and before it is returned
// to the caller. It is called outside of the PartEdge constructor, which makes it possible to
// place code inside that registers a shared pointer to this PartEdge with a directory of some kind.
void IdentityPartEdge::init()
{ base->init(); }

// Returns true if this PartEdge comes from the same analysis as that PartEdge and false otherwise
bool IdentityPartEdge::compatible(const PartEdge& that_arg) {
  const IdentityPartEdge& that = dynamic_cast<const IdentityPartEdge&>(that_arg);
  return base->compatible(that.base);
}
bool IdentityPartEdge::compatible(PartEdgePtr that_arg) {
  IdentityPartEdgePtr that = dynamicPtrCast<IdentityPartEdge>(that_arg);
  assert(that);
  return base->compatible(that->base);
}

// Returns the PartEdge this PartEdge refines (it is thie PartEdge's superset). This function documents
// the hierarchical descent of this PartEdge and makes it possible to find the common parent of parts
// derived from different analyses.
// Returns NULLPartEdge if this part has no parents (i.e. it is implemented by the syntactic analysis)
PartEdgePtr IdentityPartEdge::getInputPartEdge() const { return ((IdentityPartEdge*)this)->shared_from_this();/ *base;* / }

// Sets this PartEdge's parent
void IdentityPartEdge::setInputPartEdge(PartEdgePtr inputPartEdge) { assert(0); }

// Returns the PartEdge where the lattices computed by the analysis that implements this PartEdge may
// be found. This PartEdge should never be NULL.
PartEdgePtr IdentityPartEdge::getNodeStateLocPartEdge() const { return ((IdentityPartEdge*)this)->shared_from_this();/ *base;* / }

// Sets this PartEdge's NodeStateLocPartEdge
void IdentityPartEdge::setNodeStateLocPartEdge(PartEdgePtr NodeStateLocPartEdge) { assert(0); }

PartPtr IdentityPartEdge::source() const {
  PartPtr p = base->target();
  if(p) return makePtr<IdentityPart>(p);
  else  return p;
}

PartPtr IdentityPartEdge::target() const {
  PartPtr p = base->target();
  if(p) return makePtr<IdentityPart>(p);
  else  return p;
}

// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    PartEdge that denotes a subset of A (the function is called on this PartEdge),
//    it returns a list of PartEdges that partition O.
// A default implementation that walks the server analysis-provided graph backwards to find
//    matching PartEdges is provided.
std::list<PartEdgePtr> IdentityPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand) {
  list<PartEdgePtr> baseEdges = base->getOperandPartEdge(anchor, operand);
  list<PartEdgePtr> wrappedEdges;
  for(list<PartEdgePtr>::iterator be=baseEdges.begin(); be!=baseEdges.end(); ++be) {
    if(*be) wrappedEdges.push_back(makePtr<IdentityPartEdge>(*be));
    else    wrappedEdges.push_back(NULLPartEdge);
  }
  return wrappedEdges;
}

// If the source Part corresponds to a conditional of some sort (if, switch, while test, etc.)
// it must evaluate some predicate and depending on its value continue, execution along one of the
// outgoing edges. The value associated with each outgoing edge is fixed and known statically.
// getPredicateValue() returns the value associated with this particular edge. Since a single
// Part may correspond to multiple CFGNodes getPredicateValue() returns a map from each CFG node
// within its source part that corresponds to a conditional to the value of its predicate along
// this edge.
std::map<CFGNode, boost::shared_ptr<SgValueExp> > IdentityPartEdge::getPredicateValue()
{ return base->getPredicateValue(); }

// The the base equality and comparison operators are implemented in Part and these functions
// call the equality and inequality test functions supplied by derived classes as needed

// If this and that come from the same analysis, call the type-specific equality test implemented
// in the derived class. Otherwise, these Parts are not equal.
bool IdentityPartEdge::equal(const PartEdgePtr& that_arg) const
{
  / *scope s("IdentityPartEdge::equal");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that_arg->str()<<endl;* /
  IdentityPartEdgePtr that = dynamicPtrCast<IdentityPartEdge>(that_arg);
  assert(that);
  //dbg << "equal="<<base->equal(that->base)<<endl;
  return base->equal(that->base);
}

// If this and that come from the same analysis, call the type-specific inequality test implemented
// in the derived class. Otherwise, determine inequality by comparing the analysis pointers.
bool IdentityPartEdge::less(const PartEdgePtr& that_arg) const
{
  / *scope s("IdentityPartEdge::less");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that_arg->str()<<endl;* /
  IdentityPartEdgePtr that = dynamicPtrCast<IdentityPartEdge>(that_arg);
  assert(that);
  //dbg << "less="<<base->less(that->base)<<endl;
  return base->less(that->base);
}

// Remaps the given Lattice as needed to take into account any function call boundaries.
// Remapping is performed both in the forwards and backwards directions.
// Returns the resulting Lattice object, which is freshly allocated.
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    some MemLocs are in scope on one side of Part, while others are in scope on the other side.
//    fromPEdge is the edge from which control is passing and the current PartEdge (same as the PartEdge of
//    the Lattice) is the one to which control is passing.
Lattice* IdentityPartEdge::forwardRemapML(Lattice* lat, PartEdgePtr fromPEdge_arg, ComposedAnalysis* client)
{
  IdentityPartEdgePtr fromPEdge = dynamicPtrCast<IdentityPartEdge>(fromPEdge_arg);
  return base->forwardRemapML(lat, fromPEdge, client);
}
Lattice* IdentityPartEdge::backwardRemapML(Lattice* lat, PartEdgePtr fromPEdge_arg, ComposedAnalysis* client)
{
  IdentityPartEdgePtr fromPEdge = dynamicPtrCast<IdentityPartEdge>(fromPEdge_arg);
  return base->backwardRemapML(lat, fromPEdge, client);
}

std::string IdentityPartEdge::str(std::string indent) const
{ return string(txt()<<"[IdentityPartEdge:"<<base->str(indent)<<"]"); }
*/


/**********************
 ****** Utilities *****
 **********************/

}; // namespace fuse
