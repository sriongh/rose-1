// Example ROSE Translator: used within ROSE/tutorial

#include "rose.h"
#include <list>
#include <limits.h>
#include "stx_analysis_utils.h"
using namespace std;



idx=0;
std::pair<StxMemLocObject*, StxValueObject*> AnalyzeExprTree(SgExpression* e, string indent) {
  if(SgVarRefExp* ref=isSgVarRefExp(e)) {
//    cout << indent << "SgVarRefExp: "<<SgNode2Str(e)<<endl;
    return make_pair(new StxMemLocObject(new StxMemRegionSpecMem(ref->get_symbol()->get_declaration()), 
                                         new StxValueObject(new StxConcreteValueKind(SageBuilder::buildIntVal(0)))),
                     new StxValueObject(new StxValueObject(new StxUnknownValueKind());
  } else if(e->variantT() == V_SgDotExp) {
    pair<StxMemLocObject*, StxValueObject*> core = AnalyzeExprTree(isSgDotExp(e)->get_lhs_operand(), indent+"    ");
//    cout << indent << "SgDotExp: "<<SgNode2Str(e)<<" core=<"<<SgNode2Str(core.first)<<", "<<core.second<<">"<<endl;

    // If the core of this dot expression denotes all memory regions, any of its constituents will be the same.
    if(core.first->getRegion()->getKind() == StxMemRegionObject::any) {
      assert(core.second->getKind() == StxMemValueObject::unknown);
      return core;
    }

    // It is not possible for the core to be an expression memory region since the core must be a dot
    // expression, which must originate in some named memory location
    assert(core.first->getRegion()->getKind() == StxMemRegionObject::specMem);
    
    // This constituent of the core expression is a member of the same named memory region but with
    // a different index

    //ostringstream fname; fname << "type."<<idx;
    //generateDOT(core.first->get_type(), fname.str());
//    cout << indent << "rhs="<<SgNode2Str(isSgDotExp(e)->get_rhs_operand())<<endl;
//    cout << indent << "lhs="<<SgNode2Str(isSgDotExp(e)->get_lhs_operand())<<endl;
      if(isSgVarRefExp(isSgDotExp(e)->get_rhs_operand())) {
        if(SgClassType *type = isSgClassType(isSgDotExp(e)->get_lhs_operand()->get_type())) {
          SgClassDeclaration* decl = isSgClassDeclaration(type->get_declaration()->get_definingDeclaration());
          assert(decl);
          
/*          cout << indent << "  type="<<SgType2Str(type)<<endl;
          cout << indent << "  decl="<<SgNode2Str(type->get_declaration())<<", classdecl="<<isSgClassDeclaration(type->get_declaration()->get_definingDeclaration())<<endl;
          cout << indent << "  def="<<SgNode2Str(isSgClassDeclaration(type->get_declaration()->get_definingDeclaration())->get_definition())<<endl;*/
          SgClassDefinition* def = decl->get_definition();
          assert(def);

          const SgDeclarationStatementPtrList& members = def->get_members();
          int memberIdx=0;
//          cout << indent<<"    RHS="<<SgNode2Str(isSgVarRefExp(isSgDotExp(e)->get_rhs_operand()))<<", symbol="<<SgNode2Str(isSgVarRefExp(isSgDotExp(e)->get_rhs_operand())->get_symbol())<<", decl="<<SgNode2Str(isSgVarRefExp(isSgDotExp(e)->get_rhs_operand())->get_symbol()->get_declaration())<<endl;
          SgInitializedName* rhsDecl = isSgVarRefExp(isSgDotExp(e)->get_rhs_operand())->get_symbol()->get_declaration();
//          cout << indent << "rhsDecl="<<SgNode2Str(rhsDecl)<<endl;
          assert(rhsDecl);
          
          for(SgDeclarationStatementPtrList::const_iterator m=members.begin(); m!=members.end(); m++) {
//          cout << indent<<memberIdx<<":    member ="<<SgNode2Str(*m)<<endl;
            if(isSgVariableDeclaration(*m)) {
              const SgInitializedNamePtrList& decls = isSgVariableDeclaration(*m)->get_variables();
              for(SgInitializedNamePtrList::const_iterator d=decls.begin(); d!=decls.end(); d++) {
//                cout << indent<<"        decl "<<memberIdx<<"="<<SgNode2Str(*d)<<" type="<<SgType2Str((*d)->get_type())<<endl;
                if(*d == rhsDecl) { 
//                  cout << indent << "    idx="<<(core.second+memberIdx)<<endl;
                  return make_pair(core.first, core.second+memberIdx);
                  return make_pair(core.first,
                                   core.second->
                                   new StxValueObject(new StxConcreteValueKind(SageBuilder::buildIntVal(core.second+memberIdx
                }
                if(SgClassType* varClassType = isSgClassType((*d)->get_type())) {
                  SgClassDeclaration* varClassDecl = isSgClassDeclaration(varClassType->get_declaration()->get_definingDeclaration());
                  memberIdx += getNumClassMembers(varClassDecl->get_definition());
                } else {
                 memberIdx++;
                }  
              }
            } /*else if(isSgClassDeclaration(*m)) {
              memberIdx += getNumClassMembers(isSgClassDeclaration(isSgClassDeclaration(*m)->get_definingDeclaration())->get_definition());
            }*/
          }
          assert(0); // There must be at least one match

        }
/*        cout << indent << "   rhs_symbol="<<SgNode2Str(isSgVarRefExp(isSgDotExp(e)->get_rhs_operand())->get_symbol())<<", "<<
                             "rhs_decl="<<SgNode2Str(isSgVarRefExp(isSgDotExp(e)->get_rhs_operand())->get_symbol()->get_declaration())<<", "<<
                             "rhs_decl="<<SgNode2Str(isSgVarRefExp(isSgDotExp(e)->get_rhs_operand())->get_symbol()->get_declaration()->get_declaration())<<", "<<
                             "rhs_def="<<SgNode2Str(isSgVarRefExp(isSgDotExp(e)->get_rhs_operand())->get_symbol()->get_declaration()->get_definition())<<", "<<endl;*/
      }
      assert(0);
    }
  } else if(e->variantT() == V_SgPntrArrRefExp) {
     SgPntrArrRefExp* ref = isSgPntrArrRefExp(e);

     pair<SgExpression*, int> core = AnalyzeExprTree(ref->get_lhs_operand(), indent+":   ");
//    cout << indent << "SgPntrArrRefExp: "<<SgNode2Str(e)<<" core=<"<<SgNode2Str(core.first)<<", "<<core.second<<">"<<endl;

//    cout << indent << "rhs="<<SgNode2Str(ref->get_rhs_operand())<<endl;
//    cout << indent << "lhs="<<SgNode2Str(ref->get_lhs_operand())<<endl;
    SgArrayType* arrType = isSgArrayType(ref->get_lhs_operand()->get_type());
    assert(arrType);
/*    cout << indent << "   index="<<SgNode2Str(arrType->get_index())<<endl;
    cout << indent << "   dim="<<SgNode2Str(arrType->get_dim_info())<<"="<<arrType->get_dim_info()<<endl;
    cout << indent << "   rank="<<arrType->get_rank()<<endl;
    cout << indent << "   eltCount="<<SageInterface::getArrayElementCount(arrType)<<endl;*/
    
    // If the core of this dot expression is aliased, any of its constituents will be just as aliased
    if(core.second < 0) 
      return core;
    // If the core is named then this constituent is a member of the same named memory region but with
    // a different index
    else {
      unsigned long long coreSize;
      
/*      if(isSgPntrArrRefExp(ref->get_lhs_operand())) {
        cout << indent << "ref->get_lhs_operand()="<<SgNode2Str(ref->get_lhs_operand())<<endl;
        cout << indent << "isSgPntrArrRefExp(ref->get_lhs_operand())->get_lhs_operand()="<<SgNode2Str(isSgPntrArrRefExp(ref->get_lhs_operand())->get_lhs_operand())<<endl;
        
        SgArrayType* coreArrType = isSgArrayType(isSgPntrArrRefExp(ref->get_lhs_operand())->get_lhs_operand()->get_type());
        assert(coreArrType);
        
        coreSize = SageInterface::getArrayElementCount(arrType);
      } else {
        coreSize = 1;*/
      // Compute the number of entries in the array of the current sub-level in the SgArrayType by 
      // dividing the number of total entries in the current SgArrayType by the number of sub-arrays
      // in the next dimension.
      assert(isSgValueExp(arrType->get_index()));
      assert(isIntegerConstantValue(isSgValueExp(arrType->get_index())));
      unsigned long long subArraySize = SageInterface::getArrayElementCount(arrType) / 
                                        getIntegerConstantValue(isSgValueExp(arrType->get_index()));
      
      assert(isSgValueExp(ref->get_rhs_operand()));
      assert(isIntegerConstantValue(isSgValueExp(ref->get_rhs_operand())));
      unsigned long long index = core.second + 
                                 subArraySize * 
                                 getIntegerConstantValue(isSgValueExp(ref->get_rhs_operand()));
//      cout << indent << "returning "<<index<<", coreSize="<<coreSize<<endl;
      return make_pair(core.first, index);
    }
  } else
    return make_pair((SgExpression*)NULL, -2);
}

class functor
{
  public:
  typedef void* result_type;

  void* operator()(SgNode* n) {
    if(isSgAssignOp(n)) {
      pair<SgNode*, int> lhs = AnalyzeExprTree(isSgAssignOp(n)->get_lhs_operand(), "");
      cout << "AssignOp LHS "<<SgNode2Str(isSgAssignOp(n)->get_lhs_operand())<<": <"<<SgNode2Str(lhs.first)<<", "<<lhs.second<<">"<<endl;
      pair<SgNode*, int> rhs = AnalyzeExprTree(isSgAssignOp(n)->get_rhs_operand(), "");
      cout << "AssignOp RHS "<<SgNode2Str(isSgAssignOp(n)->get_rhs_operand())<<": <"<<SgNode2Str(rhs.first)<<", "<<rhs.second<<">"<<endl;
    }

    return NULL;
  }
};

int main( int argc, char * argv[] )
   {
  // Build the AST used by ROSE
    SgProject* project = frontend(argc,argv);

    NodeQuery::querySubTree(project, functor());
  // Generate a DOT file to use in visualizing the AST graph.
//     generateDOT ( *project );

     return 0;
   }

