#ifndef CFGUTILS_H
#define CFGUTILS_H

#include <set>
#include <string>
#include <list>
//#include "sight.h"
#include "comp_shared_ptr.h"
#include <iostream>

namespace fuse
{
  // Returns whether a given AST node that represents a constant is an integer and
  // sets *val to be the numeric value of that integer (all integer types are included
  // but not floating point, characters, etc.)
  bool IsConstInt(SgExpression* rhs, long &val);

  // pulls off all the SgCastExps that may be wrapping the given expression, returning the expression that is being wrapped
  SgExpression* unwrapCasts(SgExpression* e);
  
  // Creates the SgValueExp that denotes the contents of the given expression, returning NULL if this is not possible.
  // The caller must deallocate the returned object
  SgValueExp* getSGValueExp(SgExpression* e);
  
  // returns the DataflowNode that represents that start of the CFG of the given function's body
  CFGNode getFuncStartCFG(SgFunctionDefinition* func);

  // returns the DataflowNode that represents that end of the CFG of the given function's body
  CFGNode getFuncEndCFG(SgFunctionDefinition* func);

  // returns a string containing a unique name that is not otherwise used inside this project
  std::string genUniqueName();

  // Returns the type of the given node or NULL if none is defined for this node type
  SgType* getType(SgNode* n);

  // returns the SgFunctionDeclaration for the function with the given name
  SgFunctionDeclaration* getFuncDecl(std::string name);

  // given a function's declaration, returns the function's definition.
  // handles the case where decl->get_definition()==NULL
  SgFunctionDefinition* funcDeclToDef(SgFunctionDeclaration* decl);
  
  // Returns a string representation of this node's key information
  std::string SgNode2Str(SgNode* sgn);

  // Returns a string representation of this CFG node's key information
  std::string CFGNode2Str(const CFGNode& n);

  // Returns a string representation of this CFG edge's key information
  std::string CFGEdge2Str(const CFGEdge& e);

  // Returns a string representation of this CFG paths's key information
  std::string CFGPath2Str(const CFGPath& p);

  // Base class for objects that can be compared using the == and < operators
  class comparable;
  typedef CompSharedPtr<comparable> comparablePtr;
  class comparable {
    public:
    // Operations that derived classes must implement. Note that it is possible for 
    // the argument of these operations to be not of the same type as the derived
    // object implementing them. This happens only with special objects that can
    // handle type incompatibilities and is thus not an error. As such, implementations
    // should cast that to the type of the implementing object and allow any bad_cast
    // exceptions to propagate to the calling methods.
    // This == That
    virtual bool equal(const comparable& that) const=0;
    // This < That
    virtual bool less(const comparable& that) const=0;
    
    // Implementations of the other comparison operations
    bool operator==(const comparable& that) const;
    bool operator< (const comparable& that) const;
    bool operator!=(const comparable& that) const { return !(*this == that); }
    bool operator<=(const comparable& that) const { return (*this == that)  || (*this < that); }
    bool operator> (const comparable& that) const { return !(*this == that) && !(*this < that); }
    bool operator>=(const comparable& that) const { return (*this == that)  || !(*this < that); }
    
    // Wrappers for comparing with CompSharedPtrs
    bool operator==(const comparablePtr& that) const { return *this == *that.get(); }
    bool operator< (const comparablePtr& that) const { return *this <  *that.get(); }
    bool operator!=(const comparablePtr& that) const { return *this != *that.get(); }
    bool operator<=(const comparablePtr& that) const { return *this <= *that.get(); }
    bool operator> (const comparablePtr& that) const { return *this >  *that.get(); }
    bool operator>=(const comparablePtr& that) const { return *this >= *that.get(); }
    
    // String method
    virtual std::string str(std::string indent="") const { return "comparable"; }
  }; // class comparable
  
  // Comparison operations on lists of comparable objects
  bool operator==(const std::list<comparablePtr>& leftKey, const std::list<comparablePtr>& rightKey);
  bool operator<(const std::list<comparablePtr>& leftKey, const std::list<comparablePtr>& rightKey);
  
  // Generic wrapper for comparing SgNode*'s that implements the comparable interface
  class comparableSgNode : public comparable {
    protected:
    SgNode *n;
    public:
    comparableSgNode(SgNode* n);

    // This == That
    bool equal(const comparable& that_arg) const;

    // This < That
    bool less(const comparable& that_arg) const;

    std::string str(std::string indent="") const;
  };
  typedef boost::shared_ptr<comparableSgNode> comparableSgNodePtr;

  // Stringification of comparable lists
  std::ostream& operator<<(std::ostream& s, const std::list<comparablePtr>& l); 

  // Given a list of elements return a set that contains the same elements
  template <typename Type>
  std::set<Type> list2set(const std::list<Type> l) {
    std::set<Type> s;
    for(typename std::list<Type>::const_iterator i=l.begin(); i!=l.end(); ++i)
      s.insert(*i);
    return s;
  }
} // namespace fuse

#endif
