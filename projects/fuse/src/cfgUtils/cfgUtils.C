#include "sage3basic.h"
#include "cfgUtils.h"
#include "VirtualCFGIterator.h"
#include "CallGraphTraverse.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <iostream>
#include <boost/make_shared.hpp>


using namespace std;

#ifndef DISABLE_SIGHT
#include "sight.h"
using namespace sight;
#else
#include "sight-disable.h"
#endif


namespace fuse
{

// Liao 10/7/2010, made a few functions' namespace explicit
// pulls off all the SgCastExps that may be wrapping the given expression, returning the expression that is being wrapped
SgExpression* unwrapCasts(SgExpression* e)
{
  if(isSgCastExp(e))
  {
    return unwrapCasts(isSgCastExp(e)->get_operand());
  }
  else return e;
}

// Creates the SgValueExp that denotes the contents of the given expression, returning NULL if this is not possible.
// The caller must deallocate the returned object
SgValueExp* getSGValueExp(SgExpression* e) {
  SgTreeCopy tc;
  if(isSgValueExp(e))
    return isSgValueExp(e->copy(tc));
  if(isSgCastExp(e))
    return getSGValueExp(isSgCastExp(e)->get_operand());
  if(isSgUnaryAddOp(e))
    return getSGValueExp(isSgUnaryAddOp(e)->get_operand());
  if(isSgMinusOp(e)) {
    SgValueExp* val = getSGValueExp(isSgMinusOp(e)->get_operand());
    switch(val->variantT()) {
      case V_SgBoolValExp: break;
      case V_SgCharVal:                isSgCharVal(val)->set_value(0 - isSgCharVal(val)->get_value()); break;
      case V_SgDoubleVal:              isSgDoubleVal(val)->set_value(0 - isSgDoubleVal(val)->get_value()); break;
      case V_SgEnumVal:                isSgEnumVal(val)->set_value(0 - isSgEnumVal(val)->get_value()); break;
      case V_SgFloatVal:               isSgFloatVal(val)->set_value(0 - isSgFloatVal(val)->get_value()); break;
      case V_SgIntVal:                 isSgIntVal(val)->set_value(0 - isSgIntVal(val)->get_value()); break;
      case V_SgLongDoubleVal:          isSgLongDoubleVal(val)->set_value(0 - isSgLongDoubleVal(val)->get_value()); break;
      case V_SgLongIntVal:             isSgLongIntVal(val)->set_value(0 - isSgLongIntVal(val)->get_value()); break;
      case V_SgLongLongIntVal:         isSgLongLongIntVal(val)->set_value(0 - isSgLongLongIntVal(val)->get_value()); break;
      case V_SgShortVal:               isSgShortVal(val)->set_value(0 - isSgShortVal(val)->get_value()); break;
      case V_SgUnsignedCharVal:        isSgUnsignedCharVal(val)->set_value(0 - isSgUnsignedCharVal(val)->get_value()); break;
      case V_SgUnsignedIntVal:         isSgUnsignedIntVal(val)->set_value(0 - isSgUnsignedIntVal(val)->get_value()); break;
      case V_SgUnsignedLongLongIntVal: isSgUnsignedLongLongIntVal(val)->set_value(0 - isSgUnsignedLongLongIntVal(val)->get_value()); break;
      case V_SgUnsignedLongVal:        isSgUnsignedLongVal(val)->set_value(0 - isSgUnsignedLongVal(val)->get_value()); break;
      case V_SgUnsignedShortVal:       isSgUnsignedShortVal(val)->set_value(0 - isSgUnsignedShortVal(val)->get_value()); break;
      case V_SgWcharVal:               isSgWcharVal(val)->set_value(0 - isSgWcharVal(val)->get_value()); break;
      case V_SgStringVal:   cerr << "ERROR: cannot negate strings!"; break;
      case V_SgComplexVal:  cerr << "ERROR: don't know how to negate complex numbers!"; break;
      case V_SgUpcMythread: cerr << "ERROR: don't know how to negate UPC myThread numbers!"; break;
      case V_SgUpcThreads:  cerr << "ERROR: don't know how to negate UPC Threads!"; break;
      default: cout << "ERROR: Unknown value type in negation!";
    }
    return val;
  }

  return NULL;
}

// returns the CFGNode that represents that start of the CFG of the given function's body
CFGNode getFuncStartCFG(SgFunctionDefinition* func)
{
  //return CFGNode(func->cfgForBeginning(), f);

  // Find the SgFunctionParameterList node by walking the CFG forwards from the function's start
  CFGNode funcCFGStart(func->cfgForBeginning());
  for(CFGIterator it(funcCFGStart); it!=CFGIterator::end(); it++)
  {
    if(isSgFunctionParameterList((*it).getNode()))
      return (*it);
  }
  // We should never get here
  assert(0);

  /*ROSE_STL_Container<SgNode*> funcParamL = NodeQuery::querySubTree(SageInterface::getSageInterface::getProject()(), V_SgFunctionParameterList);
  assert(funcParamL.size()==1);
  return CFGNode(*funcParamL.begin(), 0);*/
}

// returns the CFGNode that represents that end of the CFG of the given function's body
CFGNode getFuncEndCFG(SgFunctionDefinition* func)
{
  //return (CFGNode) func->cfgForEnd();
  //return boost::make_shared<CFGNode>(func->cfgForEnd(), f);
  return func->cfgForEnd();
}

// returns a string containing a unique name that is not otherwise used inside this project
string genUniqueName()
{
  string name = "temp_";

  Rose_STL_Container<SgNode*> initNames = NodeQuery::querySubTree(SageInterface::getProject(), V_SgInitializedName);
  for(Rose_STL_Container<SgNode*>::iterator it = initNames.begin(); it!= initNames.end(); it++)
  {
    SgInitializedName *curName;
    assert(curName = isSgInitializedName(*it));
    // while our chosen "unique" name conflicts with the current SgInitializedName
    // keep adding random numbers to the end of the the "unique" name until it becomes unique
    //          printf("SgInitializedName: name<%s> == curName->get_name().getString()<%s> = %d\n", name.c_str(), curName->get_name().getString().c_str(), name == curName->get_name().getString());
    while(name == curName->get_name().getString())
    {
      char num[2];
      num[0] = '0'+rand()%10;
      num[1] = 0;
      name = name + num;
    }
  }

  Rose_STL_Container<SgNode*> funcDecls = NodeQuery::querySubTree(SageInterface::getProject(), V_SgFunctionDeclaration);
  for(Rose_STL_Container<SgNode*>::iterator it = funcDecls.begin(); it!= funcDecls.end(); it++)
  {
    SgFunctionDeclaration *curDecl;
    assert(curDecl = isSgFunctionDeclaration(*it));
    // while our chosen "unique" name conflicts with the current SgFunctionDeclaration
    // keep adding random numbers to the end of the the "unique" name until it becomes unique
    //          printf("SgFunctionDeclaration: name<%s> == curDecl->get_name().getString()<%s> = %d\n", name.c_str(), curDecl->get_name().getString().c_str(), name == curDecl->get_name().getString());
    while(name == curDecl->get_name().getString())
    {
      char num[2];
      snprintf(num, 2, "%s", (char*)(rand()%10));
      name = name + num;
    }
  }
  return name;
}

// Returns the type of the given node or NULL if none is defined for this node type
SgType* getType(SgNode* n) {
  if(SgExpression* expr = isSgExpression(n))
    return expr->get_type();
  else if(SgInitializedName* iname = isSgInitializedName(n))
    return iname->get_type();

  // This SgNode doesn't have a get_type field
  return NULL;
}

// returns the SgFunctionDeclaration for the function with the given name
SgFunctionDeclaration* getFuncDecl(string name)
{
  Rose_STL_Container<SgNode*> funcDecls = NodeQuery::querySubTree(SageInterface::getProject(), V_SgFunctionDeclaration);
  for(Rose_STL_Container<SgNode*>::iterator it = funcDecls.begin(); it!= funcDecls.end(); it++)
  {
    SgFunctionDeclaration *curDecl;
    assert(curDecl = isSgFunctionDeclaration(*it));
    // if we've found our function
    while(name == curDecl->get_name().getString())
    {
      return curDecl;
    }
  }
  return NULL;
}

// given a function's declaration, returns the function's definition.
// handles the case where decl->get_definition()==NULL
SgFunctionDefinition* funcDeclToDef(SgFunctionDeclaration* decl)
{
  if(decl->get_definition())
    return decl->get_definition();
  else
  {
    Rose_STL_Container<SgNode*> funcDefs = NodeQuery::querySubTree(SageInterface::getProject(), V_SgFunctionDefinition);
    for(Rose_STL_Container<SgNode*>::iterator it = funcDefs.begin(); it!= funcDefs.end(); it++)
    {
      assert(isSgFunctionDefinition(*it));
      // if the current definition has the same mangled name as the function declaration, we've found it
      if(isSgFunctionDefinition(*it)->get_mangled_name() == decl->get_mangled_name())
        return isSgFunctionDefinition(*it);
    }
  }
  return NULL;
}

// Returns a string representation of this node's key information
std::string SgNode2Str(SgNode* sgn)
{
  ostringstream oss;
  if(!sgn)
    oss << "NULL";
  else if(isSgClassType(sgn))
    oss << "[" << isSgClassType(sgn)->unparseToString() << " | " << isSgClassType(sgn)->class_name() << " | decl="<<SgNode2Str(isSgClassDeclaration(isSgClassType(sgn)->get_declaration())->get_definition())<<"]";
  else if(isSgNullStatement(sgn))
    oss << "[" << sgn->class_name() << "]";
  else if(isSgStringVal(sgn))
    oss << "[" << isSgStringVal(sgn)->get_value()<<" | "<<sgn->class_name() << "]";
  else if(isSgFunctionParameterList(sgn)) {
    Function func = Function::getEnclosingFunction(sgn);
    oss << "["<<func.get_name().getString()<<"(";
    SgInitializedNamePtrList args = isSgFunctionParameterList(sgn)->get_args();
    for(SgInitializedNamePtrList::iterator a=args.begin(); a!=args.end(); a++) {
      if(a!=args.begin()) oss << ", ";
#ifndef DISABLE_SIGHT
      oss << common::escape((*a)->unparseToString());
#else
      oss << (*a)->unparseToString();
#endif
    }
    oss << ") | " << sgn->class_name() << "]";
  } else if(isSgVariableSymbol(sgn)) {
#ifndef DISABLE_SIGHT
    oss << "[" << common::escape(isSgVariableSymbol(sgn)->get_name().getString()) << " | " << sgn->class_name() << "]";
#else
#endif
  } else if(isSgInitializedName(sgn)) {
#ifndef DISABLE_SIGHT
    oss << "[" << common::escape(isSgInitializedName(sgn)->get_qualified_name().getString()) << " | " << sgn->class_name() << "]";
#else
    oss << "[" << isSgInitializedName(sgn)->get_qualified_name().getString() << " | " << sgn->class_name() << "]";
#endif   
  } else
#ifndef DISABLE_SIGHT
    oss << "[" << sgn->unparseToString() << " | " << sgn->class_name() << "]";
#else
  oss << "[" << sgn->unparseToString() << " | " << sgn->class_name() << "]";
#endif
  return oss.str();
}

// Returns a string representation of this CFG node's key information
std::string CFGNode2Str(const CFGNode& n)
{
  ostringstream oss;
  if(n.getNode()==NULL)
    oss << "[NULL]";
  else if(isSgClassType(n.getNode()))
    oss << "[" << isSgClassType(n.getNode())->unparseToString() << " | " << isSgClassType(n.getNode())->class_name() << " | decl="<<SgNode2Str(isSgClassDeclaration(isSgClassType(n.getNode())->get_declaration())->get_definition())<<"]";
  else if(isSgNullStatement(n.getNode()))
    oss << "[" << n.getNode()->class_name() << " | " << n.getIndex() << "]";
  else if(isSgStringVal(n.getNode()))
    oss << "[" << isSgStringVal(n.getNode())->get_value()<<" | "<<n.getNode()->class_name() << " | " << n.getIndex() << "]";
  else if(isSgFunctionParameterList(n.getNode())) {
    Function func = Function::getEnclosingFunction(n.getNode());
    oss << "["<<func.get_name().getString()<<"(";
    SgInitializedNamePtrList args = isSgFunctionParameterList(n.getNode())->get_args();
    for(SgInitializedNamePtrList::iterator a=args.begin(); a!=args.end(); a++) {
      if(a!=args.begin()) oss << ", ";
#ifndef DISABLE_SIGHT
      oss << common::escape((*a)->unparseToString());
#else
      oss << (*a)->unparseToString();
#endif
    }
    oss << ") | " << n.getNode()->class_name() << " | " << n.getIndex() << "]";
  } else if(isSgVariableSymbol(n.getNode())) {
#ifndef DISABLE_SIGHT
    oss << "[" << common::escape(isSgVariableSymbol(n.getNode())->get_name().getString()) << " | " << n.getNode()->class_name() << " | " << n.getIndex() << "]";
#else
    oss << "[" << isSgVariableSymbol(n.getNode())->get_name().getString() << " | " << n.getNode()->class_name() << " | " << n.getIndex() << "]";
#endif
  } else if(isSgInitializedName(n.getNode())) {
#ifndef DISABLE_SIGHT
    oss << "[" << common::escape(isSgInitializedName(n.getNode())->get_name().getString()) << " | " << n.getNode()->class_name() << " | " << n.getIndex() << "]";
#else
    oss << "[" << isSgInitializedName(n.getNode())->get_name().getString() << " | " << n.getNode()->class_name() << " | " << n.getIndex() << "]";
#endif
  } else
#ifndef DISABLE_SIGHT
    oss << "[" << common::escape(n.getNode()->unparseToString()) << " | " << n.getNode()->class_name() << " | " << n.getIndex() << "]";
#else
  oss << "[" << n.getNode()->unparseToString() << " | " << n.getNode()->class_name() << " | " << n.getIndex() << "]";
#endif
  return oss.str();
}

// Returns a string representation of this CFG edge's key information
std::string CFGEdge2Str(const CFGEdge& e)
{
  ostringstream oss;
  oss << "[" << CFGNode2Str(e.source()) << " ==&gt; " << CFGNode2Str(e.target())<<"]";
  return oss.str();
}

// Returns a string representation of this CFG paths's key information
std::string CFGPath2Str(const CFGPath& p)
{
  ostringstream oss;
  const std::vector<CFGEdge>& edges = p.getEdges();
  oss << "[";
  for(std::vector<CFGEdge>::const_iterator e=edges.begin(); e!=edges.end(); ) {
    oss << CFGEdge2Str(*e);
    e++;
    if(e!=edges.end()) oss << endl;
  }
  oss << "]";
  return oss.str();
}

/**********************
 ***** comparable *****
 **********************/
bool comparable::operator==(const comparable& that) const {
  /*scope s("operator==(comparable)");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that.str()<<endl;*/

  // First try applying the equal method of this
  try{
    return equal(that);
  } catch (std::bad_cast bc) {
    // The types of this and that are not compatible and this::equal cannot deal with
    // this. Try calling the equal method of that.
    try{
      return that.equal(*this);
    } catch (std::bad_cast bc) {
      // Neither method could deal with the type incompatibility
      cerr << "ERROR in comparable::operator==: types of comparable objects are not compatible!"<<endl;
      cerr << "this="<<str()<<endl;
      cerr << "that="<<that.str()<<endl;
      ROSE_ASSERT(0);
    }
  }
}

bool comparable::operator<(const comparable& that) const {
  /*scope s("operator<(comparable)");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that.str()<<endl;*/

  // First try applying the less method of this
  try{
    return less(that);
  } catch (std::bad_cast bc) {
    // The types of this and that are not compatible and this::less cannot deal with
    // this. Try calling the less method of that. Since we know that this!=that because
    // they're not type compatible we can just negate the return of that.less(this)
    try{
      return !that.less(*this);
    } catch (std::bad_cast bc) {
      // Neither method could deal with the type incompatibility
      cerr << "ERROR in comparable::operator<: types of comparable objects are not compatible!"<<endl;
      cerr << "this="<<str()<<endl;
      cerr << "that="<<that.str()<<endl;
      ROSE_ASSERT(0);
    }
  }
}

// Comparison operations on lists of comparable objects
bool operator==(const std::list<comparablePtr>& leftKey, const std::list<comparablePtr>& rightKey) {
  std::list<comparablePtr>::const_iterator left=leftKey.begin(), right=rightKey.begin();
  for(; left!=leftKey.end() && right!=rightKey.end(); left++, right++) {
    if(*left != *right) return false;
  }
  return true;
}

bool operator<(const std::list<comparablePtr>& leftKey, const std::list<comparablePtr>& rightKey) {
  /*scope s("operator<(list<comparable>)");
  dbg << "leftKey: "<<leftKey<<endl;
  dbg << "rightKey: "<<rightKey<<endl;*/

  std::list<comparablePtr>::const_iterator left=leftKey.begin(), right=rightKey.begin();
  for(; left!=leftKey.end() && right!=rightKey.end(); left++, right++) {
    // Less-than
    // Greater-than (rephrased since the > implementation calls < redundantly)
    if(*left != *right) return false;
  }
  // leftKey == rightKey
  return false;
}

// Stringification of comparable lists
std::ostream& operator<<(std::ostream& s, const std::list<comparablePtr>& l) {
  /*for(list<comparablePtr>::const_iterator k=l.begin(); k!=l.end(); k++) {
    if(k!=l.begin()) s << ", ";
    s << (*k)->str();
  }*/
  s << "<table border=1><tr><td>";
  for(list<comparablePtr>::const_iterator k=l.begin(); k!=l.end(); k++) {
    if(k!=l.begin()) s << "</td><td>";
    s << (*k)->str();
  }
  s << "</td></tr></table>";
  return s;
}

/****************************
 ***** comparableSgNode *****
 ****************************/

comparableSgNode::comparableSgNode(SgNode* n): n(n) {}
// This == That
bool comparableSgNode::equal(const comparable& that_arg) const {
  //try{
    const comparableSgNode& that = dynamic_cast<const comparableSgNode&>(that_arg);
    return n == that.n;
  /*} catch (std::bad_cast bc) {
    ROSE_ASSERT(0);
  }*/
}

// This < That
bool comparableSgNode::less(const comparable& that_arg) const {
  //try{
    const comparableSgNode& that = dynamic_cast<const comparableSgNode&>(that_arg);
    return n < that.n;
  /*} catch (std::bad_cast bc) {
    ROSE_ASSERT(0);
  }*/
}
std::string comparableSgNode::str(std::string indent) const { return SgNode2Str(n); }

} /* namespace fuse */
