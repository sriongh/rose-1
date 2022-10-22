#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "rose.h"
#include "graphIterator.h"
#include "cfgUtils.h"
#include "CallGraphTraverse.h"
#include "sight.h"

namespace fuse {
class Analysis;
};

//#include "lattice.h"
#include "nodeState.h"
#include "partitions.h"
#include <vector>
#include <set>
#include <map>
#include <list>
#include <iostream>

namespace fuse {

// Returns a boost shared pointers to variables that should be
// deallocated when the shared_ptr's reference count drops. Useful
// for passing unmanaged memory to code that takes boost shared
// pointers as arguments.
void dummy_dealloc(int *);
#ifdef ANALYSIS_C
void dummy_dealloc(int *) {} // Deallocator that does nothing
#endif
template <class RefType>
boost::shared_ptr<RefType> shared_ptr_for_static(RefType& o)
{
  return boost::shared_ptr<RefType>(&o, dummy_dealloc);
}

class Analysis
{
  public:
    // a filter function to decide which raw CFG node to show (if return true) or hide (otherwise)
    // This is required to support custom filters of virtual CFG
    // Custom filter is set inside the analysis.
    // Inter-procedural analysis will copy the filter from its analysis during the call to its constructor.
    bool (*filter) (CFGNode cfgn);
    Analysis(bool (*f)(CFGNode) = defaultFilter):filter(f) {}

    // Runs the analysis. Returns true if the function's NodeState gets modified as a result and false otherwise
    virtual void runAnalysis();

    ~Analysis();
};

/********************************
 *** UnstructuredPassAnalyses ***
 ********************************/
class ComposedAnalysis;

// A driver class which simply iterates through all CFG nodes of a specified function
class UnstructuredPassAnalysis : virtual public Analysis
{
  public:
  ComposedAnalysis* analysis;

  UnstructuredPassAnalysis(ComposedAnalysis* analysis) : analysis(analysis) {}

  // Runs the analysis. Returns true if the function's NodeState gets modified as a result and false otherwise
  void runAnalysis();

  virtual void visit(PartPtr p, NodeState& state)=0;
};

/*************************
 *** Dataflow Analyses ***
 *************************/

// Class that encapsulates all the various types of information analyses need
// to know about ATS Parts and PartEdges while they execute
template <typename T>
class AnalysisATS {
  protected:
  // The Part or PartEdge from which analysis can fetch its NodeState
  T NodeState_;
  // Flag that indicates whether this object contains a valid NodeState Part or PartEdge
  bool NodeState_valid_;

  // The superset Part or PartEdge of NodeState. Information within NodeState mapped to NodeState
  // is maintained separately for different edges of NodeState's superset ATS, which is built
  // from Part or PartEdge that are the supersets of NodeState and other Parts or PartEdges from its ATS).
  T index_;
  // Flag that indicates whether this object contains a valid index Part or PartEdge
  bool index_valid_;

  // The Part or PartEdge from the ATS that this analysis takes as input. This may be the ATS of NodeState
  // when running under loose composition. However, when composed tightly, the ATS of NodeState
  // is being constructed as the analysis runs and thus, its nodes and edges cannot be used as
  // input by analysis transfer functions. In that case this Part or PartEdge comes from the ATS the nodes
  // of which are guaranteed to have already been constructed.
  T input_;
  // Flag that indicates whether this object contains a valid input Part or PartEdge
  bool input_valid_;

  public:
  AnalysisATS() {}

  AnalysisATS(T NodeState, T index, T input) :
    NodeState_(NodeState), NodeState_valid_(true),
    index_(index),         index_valid_(true),
    input_(input),         input_valid_(true)
  {
    // At least one Part or PartEdge must be valid
    assert(NodeState_valid_ || index_valid_ || input_valid_);
  }

  AnalysisATS(T NodeState, bool NodeState_valid, T index, bool index_valid, T input, bool input_valid) :
    NodeState_(NodeState), NodeState_valid_(NodeState_valid),
    index_(index),         index_valid_(index_valid),
    input_(input),         input_valid_(input_valid)
  {
    // At least one Part or PartEdge must be valid
    assert(NodeState_valid_ || index_valid_ || input_valid_);
  }

  AnalysisATS(const AnalysisATS& that) :
    NodeState_(that.NodeState_), NodeState_valid_(that.NodeState_valid_),
    index_(that.index_),         index_valid_(that.index_valid_),
    input_(that.input_),         input_valid_(that.input_valid_)
  {
    // At least one Part or PartEdge must be valid
    assert(NodeState_valid_ || index_valid_ || input_valid_);
  }

  T NodeState() const { assert(NodeState_valid_); return NodeState_; }
  T index()     const { assert(index_valid_);     return index_; }
  T input()     const { assert(input_valid_);     return input_; }

  bool NodeState_valid() const { return NodeState_valid_; }
  bool index_valid()     const { return index_valid_; }
  bool input_valid()     const { return input_valid_; }

  bool operator==(const AnalysisATS& that) const {
    // This and that must have consistent validity
    assert(NodeState_valid_ == that.NodeState_valid_);
    assert(index_valid_     == that.index_valid_);
    assert(input_valid_     == that.input_valid_);

    // At least one Part or PartEdge must be valid
    assert(NodeState_valid_ || index_valid_ || input_valid_);

    if(NodeState_valid_) {
      if(NodeState_==that.NodeState_) return true;
      else                            return false;
    }

    if(index_valid_) {
      if(index_==that.index_) return true;
      else                    return false;
    }

    if(input_valid_) {
      if(input_<that.input_) return true;
      else                   return false;
    }

    assert(false);
  }

  bool operator<(const AnalysisATS& that) const {
    // This and that must have consistent validity
    assert(NodeState_valid_ == that.NodeState_valid_);
    assert(index_valid_     == that.index_valid_);
    assert(input_valid_     == that.input_valid_);

    // At least one Part or PartEdge must be valid
    assert(NodeState_valid_ || index_valid_ || input_valid_);

    if(NodeState_valid_) {
      if(NodeState_<that.NodeState_) return true;
      else if(NodeState_>that.NodeState_) return false;
    }

    if(index_valid_) {
      if(index_<that.index_) return true;
      else if(index_>that.index_) return false;
    }

    if(input_valid_) {
      if(input_<that.input_) return true;
      else if(input_>that.input_) return false;
    }

    // this == that, so !(this < that)
    return true;
  }

  // Returns a human-readable string representation of this object
  std::string str() const  {
    // At least one Part or PartEdge must be valid
    assert(NodeState_valid_ || index_valid_ || input_valid_);

    std::ostringstream oss;

    oss << "[";
    if(NodeState_valid_) oss << "NodeState="<<(NodeState_? NodeState_->str() : "NULL")<<std::endl;
    if(index_valid_)     oss << "index="    <<(index_?     index_->str()     : "NULL")<<std::endl;
    if(input_valid_)     oss << "input="    <<(input_?     input_->str()     : "NULL")<<std::endl;
    oss << "]";

    return oss.str();
  }
}; // class AnalysisATS

/* // Class that encapsulates all the various types of information analyses need
// to know about sets of ATS Parts or PartEdges while they execute
template <class ContainerT, class T, class DerivedT>
class AnalysisATSContainers {
  // For each container type, a container and a flag that records whether this iterator is valid.
  // Same types as in AnalysisATS
  ContainerT NodeStates_;
  bool NodeStates_valid_;

  ContainerT indexes_;
  bool indexes_valid_;

  ContainerT inputs_;
  bool inputs_valid_;

  public:
  AnalysisATSContainers() : NodeStates_valid_(true), indexes_valid_(true), inputs_valid_(true) {}

  // Initializes this container with valid containers for NodeStates, indexes and inputs
  AnalysisATSContainers(const ContainerT& NodeStates, const ContainerT& indexes, const ContainerT& inputs) :
    NodeStates_(NodeStates), NodeStates_valid_(true),
    indexes_(indexes),       indexes_valid_(true),
    inputs_(inputs),         inputs_valid_(true)
  {}

  // Initializes this container with containers for NodeStates, indexes and inputs, some of which may not be
  // valid and indicates which are valid and which aren't
  AnalysisATSContainers(const ContainerT& NodeStates, bool NodeStates_valid,
                        const ContainerT& indexes,    bool indexes_valid,
                        const ContainerT& inputs,     bool inputs_valid) :
    NodeStates_(NodeStates), NodeStates_valid_(NodeStates_valid),
    indexes_(indexes),       indexes_valid_(indexes_valid),
    inputs_(inputs),         inputs_valid_(inputs_valid)
  {
    // At least one list must be valid
    assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);
  }

  AnalysisATSContainers(const AnalysisATSContainers& that) :
    NodeStates_(that.NodeStates_), indexes_(that.indexes_), inputs_(that.inputs_) {}

  // Return a reference to each of the sets inside this object
  const ContainerT& NodeStates() const { assert(NodeStates_valid_); return NodeStates_; }
  const ContainerT& indexes()    const { assert(indexes_valid_);    return indexes_; }
  const ContainerT& inputs()     const { assert(inputs_valid_);     return inputs_; }

  size_t size() const {
    // Compute the size of the lists from the lists that are valid, ensuring that all valid
    // lists have the same size.

    // At least one list must be valid
    assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);

    bool sizeKnown=false;
    size_t ret;
    if(NodeStates_valid_) {
      ret = NodeStates_.size();
      sizeKnown = true;
    }

    if(indexes_valid_) {
      if(sizeKnown) assert(ret == indexes_.size());
      else {
        ret = indexes_.size();
        sizeKnown = true;
      }
    }

    if(inputs_valid_) {
      if(sizeKnown) assert(ret == inputs_.size());
      else {
        ret = inputs_.size();
        sizeKnown = true;
      }
    }

    // The size of the list must be known at this point
    assert(sizeKnown);

    return ret;
  }

  // Returns a human-readable string representation of this object
  std::string str() const {
    std::ostringstream oss;

    // At least one list must be valid
    assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);

    oss << "[";
    if(NodeStates_valid_) {
      oss << "NodeStates="<<std::endl;
      for(typename ContainerT::const_iterator p=NodeStates.begin(); p!=NodeStates.end(); ++p)
        oss << "  "<<(*p)->str()<<std::endl;
    }

    if(indexes_valid_) {
      oss << "indexes="<<std::endl;
      for(typename ContainerT::const_iterator p=indexes.begin(); p!=indexes.end(); ++p)
        oss << "  "<<(*p)->str()<<std::endl;
    }

    if(inputs_valid_) {
      oss << "inputs="<<std::endl;
      for(typename ContainerT::const_iterator p=inputs.begin(); p!=inputs.end(); ++p)
        oss << "  "<<(*p)->str()<<std::endl;
    }
    oss << "]";

    return oss.str();
  }

  // Returns the set that contains the superset s of the s in partsSet
  static ContainerT getInputs(const ContainerT& s) {
    ContainerT out;
    for(typename ContainerT::const_iterator p=s.begin(); p!=s.end(); ++p) {
      out.insert(DerivedT::getInput(*p));
    }
    return out;
  }

  // Set iteration
  class iterator {
    // For each container type, an iterator and a flag that records whether this iterator is valid
    typename ContainerT::const_iterator NodeStatesIter_;
    bool NodeStates_valid_;

    typename ContainerT::const_iterator indexesIter_;
    bool indexes_valid_;

    typename ContainerT::const_iterator inputsIter_;
    bool inputs_valid_;

    public:

    // Initializes this container with iterators for NodeStates, indexes and inputs, some of which may not be
    // valid and indicates which are valid and which aren't
    iterator(typename ContainerT::const_iterator NodeStatesIter, bool NodeStates_valid,
             typename ContainerT::const_iterator indexesIter,    bool indexes_valid,
             typename ContainerT::const_iterator inputsIter,     bool inputs_valid) :
        NodeStatesIter_(NodeStatesIter), NodeStates_valid_(NodeStates_valid),
        indexesIter_   (indexesIter),    indexes_valid_(indexes_valid),
        inputsIter_    (inputsIter),     inputs_valid_(inputs_valid)
    {
      // At least one iterator must be valid
      assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);
    }

    iterator(const iterator& that) :
      NodeStatesIter_(that.NodeStatesIter_), NodeStates_valid_(that.NodeStates_valid_),
      indexesIter_   (that.indexesIter_),    indexes_valid_(that.indexes_valid_),
      inputsIter_    (that.inputsIter_),     inputs_valid_(that.inputs_valid_)
    {
      // At least one iterator must be valid
      assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);
    }


    // Methods to get the value of each type of item at the current iterator location
    T NodeState() { assert(NodeStates_valid_); return *NodeStatesIter_; }
    T index()     { assert(indexes_valid_);    return *indexesIter_; }
    T input()     { assert(inputs_valid_);     return *inputsIter_; }

    // Method to get all the items at the current iterator location
    AnalysisATS<T> cur() const {
      // At least one iterator must be valid
      assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);

      return AnalysisATS<T>(NodeStates_valid_? *NodeStatesIter_: T(),
                            indexes_valid_?    *indexesIter_:    T(),
                            inputs_valid_?     *inputsIter_:     T());
    }

    // Pre-increment
    iterator& operator++() {
      // At least one iterator must be valid
      assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);

      if(NodeStates_valid_) ++NodeStatesIter_;
      if(indexes_valid_)    ++indexesIter_;
      if(inputs_valid_)     ++inputsIter_;
      return *this;
    }

    // Post-increment
    iterator operator++(int) {
      // At least one iterator must be valid
      assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);

      iterator old(*this);
      if(NodeStates_valid_) ++NodeStatesIter_;
      if(indexes_valid_)    ++indexesIter_;
      if(inputs_valid_)     ++inputsIter_;
      return old;
    }

    // Relational operators
    bool operator==(const iterator& that) {
      // At least one iterator must be valid
      assert(NodeStates_valid_ || indexes_valid_ || inputs_valid_);

      // Ensure that the validity state of this and that iterators is the same
      assert(NodeStates_valid_ == that.NodeStates_valid_);
      assert(indexes_valid_    == that.indexes_valid_);
      assert(inputs_valid_     == that.inputs_valid_);

      return (NodeStates_valid_? NodeStatesIter_ == that.NodeStatesIter_: true) &&
             (indexes_valid_?    indexesIter_    == that.indexesIter_ :   true)&&
             (inputs_valid_?     inputsIter_     == that.inputsIter_:     true);
    }
    bool operator!=(const iterator& that) {
      return !(*this == that);
    }
  }; // class iterator

  iterator begin() const {
    return iterator(NodeStates_.begin(), NodeStates_valid_,
                    indexes_.begin(),    indexes_valid_,
                    inputs_.begin(),     inputs_valid_);
  }

  iterator end() const {
    return iterator(NodeStates_.end(), NodeStates_valid_,
                    indexes_.end(),    indexes_valid_,
                    inputs_.end(),     inputs_valid_);
  }
}; // class AnalysisATSContainers

// Class that encapsulates all the various types of information analyses need
// to know about sets of ATS s while they execute
template<class ContainerT>
class AnalysisPartContainers: public AnalysisATSContainers<ContainerT, PartPtr, AnalysisPartContainers<ContainerT> > {
  public:
  AnalysisPartContainers() : AnalysisATSContainers<ContainerT, PartPtr, AnalysisPartContainers<ContainerT> >() {}
  AnalysisPartContainers(const ContainerT NodeStates, const ContainerT& indexes, const ContainerT& inputs) :
    AnalysisATSContainers<ContainerT, PartPtr, AnalysisPartContainers<ContainerT> >(NodeStates, indexes, inputs) {}
  AnalysisPartContainers(const ContainerT NodeStates, bool NodeStates_valid,
                         const ContainerT& indexes,    bool indexes_valid,
                         const ContainerT& inputs,     bool inputs_valid) :
    AnalysisATSContainers<ContainerT, PartPtr, AnalysisPartContainers<ContainerT> >(NodeStates, NodeStates_valid,
                                                                                    indexes,    indexes_valid,
                                                                                    inputs,     inputs_valid) {}
  AnalysisPartContainers(const AnalysisPartContainers& that) :
    AnalysisATSContainers<ContainerT, PartPtr, AnalysisPartContainers<ContainerT> >(that) {}

  // Returns the iterm that denotes the superset of the given item
  static PartPtr getInput(const PartPtr& item) { return item->getInputPart(); }
};

typedef AnalysisPartContainers<std::set<PartPtr> >    AnalysisPartSets;
typedef AnalysisPartContainers<std::list<PartPtr> >   AnalysisPartLists;
typedef AnalysisPartContainers<std::vector<PartPtr> > AnalysisPartVectors;


// Class that encapsulates all the various types of information analyses need
// to know about sets of ATS PartEdges while they execute
template<class ContainerT>
class AnalysisPartEdgeContainers: public AnalysisATSContainers<ContainerT, PartEdgePtr, AnalysisPartEdgeContainers<ContainerT> > {
  public:
  AnalysisPartEdgeContainers() : AnalysisATSContainers<ContainerT, PartEdgePtr, AnalysisPartEdgeContainers<ContainerT> >() {}
  AnalysisPartEdgeContainers(const ContainerT NodeStates, const ContainerT& indexes, const ContainerT& inputs) :
    AnalysisATSContainers<ContainerT, PartEdgePtr, AnalysisPartEdgeContainers<ContainerT> >(NodeStates, indexes, inputs) {}
  AnalysisPartEdgeContainers(const ContainerT NodeStates, bool NodeStates_valid,
                             const ContainerT& indexes,    bool indexes_valid,
                             const ContainerT& inputs,     bool inputs_valid) :
    AnalysisATSContainers<ContainerT, PartEdgePtr, AnalysisPartEdgeContainers<ContainerT> >(NodeStates, NodeStates_valid,
                                                                                            indexes,    indexes_valid,
                                                                                            inputs,     inputs_valid) {}
  AnalysisPartEdgeContainers(const AnalysisPartEdgeContainers& that) :
    AnalysisATSContainers<ContainerT, PartEdgePtr, AnalysisPartEdgeContainers<ContainerT> >(that) {}

  // Returns the iterm that denotes the Input of the given item
  static PartEdgePtr getInput(const PartEdgePtr& item) { return item->getInputPartEdge(); }
};

typedef AnalysisPartEdgeContainers<std::set<PartEdgePtr> >    AnalysisPartEdgeSets;
typedef AnalysisPartEdgeContainers<std::list<PartEdgePtr> >   AnalysisPartEdgeLists;
typedef AnalysisPartEdgeContainers<std::vector<PartEdgePtr> > AnalysisPartEdgeVectors;*/


// Class that encapsulates all the various types of information analyses need
// to know about containers of ATS Parts or PartEdges (sets, lists or vector)
// while they execute. It is organized as a container of AnalysisParts or AnalysisPartEdges
template <class ContainerT, class ContainerATS_T, class T>
class AnalysisATSContainers {
  protected:
  // For each container type, a container and a flag that records whether this iterator is valid.
  // Same types as in AnalysisATS
  ContainerATS_T cont;

  public:
  AnalysisATSContainers() {}

  // Verifies that the validity information (NodeState_valid, index_valid and input_valid)
  // of the object being added is consistent with the validity of all the other objects in cont
  void validateObjToAdd(const AnalysisATS<T>& newObj) {
    // If this is not the first object to be added, ensure that its validity info is the
    // same as that of the first object.
    if(cont.size()>0) {
      assert(cont.begin()->NodeState_valid() == newObj.NodeState_valid());
      assert(cont.begin()->index_valid()     == newObj.index_valid());
      assert(cont.begin()->input_valid()     == newObj.input_valid());
    }
  }

  // Generic function to add a new element to the given container subCont. It is used by the NodeStates(),
  // indexes() and inputs() methods to create new containers of sub-objects while iterating through
  // cont.
  virtual void add(ContainerT& subCont, T obj) const=0;

  // Returns the number of elements in this container
  size_t size() const { return cont.size(); }

  // Return a reference to each of the sets inside this object
  ContainerT NodeStates() const {
    if(cont.size()==0) return ContainerT();
    else {
      assert(cont.begin()->NodeState_valid());
      ContainerT subCont;
      for(typename ContainerATS_T::const_iterator i=cont.begin(); i!=cont.end(); ++i)
        add(subCont, i->NodeState());
      return subCont;
    }
  }

  ContainerT indexes() {
    if(cont.size()==0) return ContainerT();
    else {
      assert(cont.begin()->index_valid());
      ContainerT subCont;
      for(typename ContainerATS_T::const_iterator i=cont.begin(); i!=cont.end(); ++i)
        add(subCont, i->index());
      return subCont;
    }
  }

  ContainerT inputs()  {
    if(cont.size()==0) return ContainerT();
    else {
      assert(cont.begin()->input_valid());
      ContainerT subCont;
      for(typename ContainerATS_T::const_iterator i=cont.begin(); i!=cont.end(); ++i)
        add(subCont, i->input());
      return subCont;
    }
  }

  // Returns a human-readable string representation of this object
  std::string str() const {
    std::ostringstream oss;

    oss << "[";
    for(typename ContainerATS_T::const_iterator i=cont.begin(); i!=cont.end(); ++i)
      oss << "    "<<i->str()<<std::endl;
    oss << "]";

    return oss.str();
  }

  // Iterator methods
  typename ContainerATS_T::iterator       begin()       { return cont.begin(); }
  typename ContainerATS_T::const_iterator begin() const { return cont.begin(); }
  typename ContainerATS_T::iterator       end()         { return cont.end(); }
  typename ContainerATS_T::const_iterator end() const   { return cont.end(); }

  typedef typename ContainerATS_T::iterator       iterator;
  typedef typename ContainerATS_T::const_iterator const_iterator;

  // Returns the set that contains the superset s of the s in partsSet
  /*static ContainerT getInputs(const ContainerT& s) {
    ContainerT out;
    for(typename ContainerT::const_iterator p=s.begin(); p!=s.end(); ++p) {
      out.insert(DerivedT::getInput(*p));
    }
    return out;
  }*/
}; // class AnalysisATSContainers

template<class T>
class AnalysisATSSets: public AnalysisATSContainers<std::set<T>, std::set<AnalysisATS<T> >, T> {
  public:
  AnalysisATSSets() : AnalysisATSContainers<std::set<T>, std::set<AnalysisATS<T> >, T>() {}

  // Creates a set of AnalysisATS objects from just the NodeStateObjs
  /*AnalysisATSSets(const std::set<T> NodeStateObjs, NodeState2AllMapper& mapper) :
    AnalysisATSContainers<std::set<T>, std::set<AnalysisATS<T> >, T>()
  {
    for(std::set<T>::const_iterator i=NodeStateObjs.begin(); i!=NodeStateObjs.end(); ++i) {
      insert(mapper.NodeState2All(*i));
    }
  }*/

  void insert(const AnalysisATS<T>& newObj) {
    this->validateObjToAdd(newObj);
    AnalysisATSContainers<std::set<T>, std::set<AnalysisATS<T> >, T>::cont.insert(newObj);
  }

  // Generic function to add a new element to the given container subCont. It is used by the NodeStates(),
  // indexes() and inputs() methods to create new containers of sub-objects while iterating through
  // cont.
  void add(std::set<T>& subCont, T obj) const {
    subCont.insert(obj);
  }
};

template<class T>
class AnalysisATSLists: public AnalysisATSContainers<std::list<T>, std::list<AnalysisATS<T> >, T> {
  public:
  AnalysisATSLists() : AnalysisATSContainers<std::list<T>, std::list<AnalysisATS<T> >, T>() {}

/*  // Creates a list of AnalysisATS objects from just the NodeStateObjs
  AnalysisATSLists(const std::list<T> NodeStateObjs, NodeState2AllMapper& mapper) :
    AnalysisATSContainers<std::list<T>, std::list<AnalysisATS<T> >, T>()
  {
    for(std::list<T>::const_iterator i=NodeStateObjs.begin(); i!=NodeStateObjs.end(); ++i) {
      push_back(mapper.NodeState2All(*i));
    }
  }*/

  void push_front(const AnalysisATS<T>& newObj) {
    this->validateObjToAdd(newObj);
    AnalysisATSContainers<std::list<T>, std::list<AnalysisATS<T> >, T>::cont.push_front(newObj);
  }

  void push_back(const AnalysisATS<T>& newObj) {
    this->validateObjToAdd(newObj);
    AnalysisATSContainers<std::list<T>, std::list<AnalysisATS<T> >, T>::cont.push_back(newObj);
  }

  // Generic function to add a new element to the given container subCont. It is used by the NodeStates(),
  // indexes() and inputs() methods to create new containers of sub-objects while iterating through
  // cont.
  void add(std::list<T>& subCont, T obj) const {
    subCont.push_back(obj);
  }
};

template<class T>
class AnalysisATSVectors: public AnalysisATSContainers<std::vector<T>, std::vector<AnalysisATS<T> >, T> {
  public:
  AnalysisATSVectors() : AnalysisATSContainers<std::vector<T>, std::vector<AnalysisATS<T> >, T>() {}

/*  // Creates a vector of AnalysisATS objects from just the NodeStateObjs
  AnalysisATSvectors(const std::vector<T> NodeStateObjs, NodeState2AllMapper& mapper) :
    AnalysisATSContainers<std::vector<T>, std::vector<AnalysisATS<T> >, T>()
  {
    for(std::vector<T>::const_iterator i=NodeStateObjs.begin(); i!=NodeStateObjs.end(); ++i) {
      push_back(mapper.NodeState2All(*i));
    }
  }*/

  void push_front(const AnalysisATS<T>& newObj) {
    this->validateObjToAdd(newObj);
    AnalysisATSContainers<std::vector<T>, std::vector<AnalysisATS<T> >, T>::cont.push_front(newObj);
  }

  void push_back(const AnalysisATS<T>& newObj) {
    this->validateObjToAdd(newObj);
    AnalysisATSContainers<std::vector<T>, std::vector<AnalysisATS<T> >, T>::cont.push_back(newObj);
  }

  // Generic function to add a new element to the given container subCont. It is used by the NodeStates(),
  // indexes() and inputs() methods to create new containers of sub-objects while iterating through
  // cont.
  void add(std::vector<T>& subCont, T obj) const {
    subCont.push_back(obj);
  }
};

typedef AnalysisATSSets   <PartPtr> AnalysisPartSets;
typedef AnalysisATSLists  <PartPtr> AnalysisPartLists;
typedef AnalysisATSVectors<PartPtr> AnalysisPartVectors;
typedef AnalysisATSSets   <PartEdgePtr> AnalysisPartEdgeSets;
typedef AnalysisATSLists  <PartEdgePtr> AnalysisPartEdgeLists;
typedef AnalysisATSVectors<PartEdgePtr> AnalysisPartEdgeVectors;

class NodeState2AllMapper;

// Class that encapsulates all the various types of information analyses need
// to know about ATS Parts while they execute
class AnalysisParts : public AnalysisATS<PartPtr>
{
  public:
  AnalysisParts() {}

  AnalysisParts(PartPtr NodeState, PartPtr index, PartPtr input) :
    AnalysisATS<PartPtr>(NodeState, index, input)
  {}
  AnalysisParts(PartPtr NodeState, bool NodeState_valid,
                PartPtr index,     bool index_valid,
                PartPtr input,     bool input_valid) :
    AnalysisATS<PartPtr>(NodeState, NodeState_valid,
                         index,     index_valid,
                         input,     input_valid)
  {}
  AnalysisParts(const AnalysisParts& that) : AnalysisATS<PartPtr>(that) {}
  AnalysisParts(const AnalysisATS<PartPtr>& that) : AnalysisATS<PartPtr>(that) {}

  /*// Returns the list of this Part's outgoing edges.
  AnalysisPartEdgeLists outEdges() {
    return AnalysisPartEdgeLists(NodeState()->outEdges(), index()->outEdges(), input()->outEdges());
  }

  AnalysisPartEdgeLists inEdges() {
    return AnalysisPartEdgeLists(NodeState()->inEdges(), index()->inEdges(), input()->inEdges());
  }*/

  /* // Returns the list of this Part's outgoing edges, focusing on all or a subset of the edge types
  AnalysisPartEdgeLists outEdges(bool NodeState_valid=true, bool indexes_valid=true, bool inputs_valid=true) {
    //sight::structure::scope s("outEdges");
    cout << "outEdges"<<endl<<"-------------------"<<endl;
    cout << "NodeState_valid="<<NodeState_valid<<", indexes_valid="<<indexes_valid<<", inputs_valid="<<inputs_valid<<endl;
    cout << "NodeState="<<(NodeState_valid_? NodeState()->str(): "NULL")<<endl;
    cout << "index()="<<(index_valid_? index()->str(): "NULL")<<endl;
    cout << "input()="<<(input_valid_? NodeState()->str(): "NULL")<<endl;
    return AnalysisPartEdgeLists(NodeState_valid? NodeState()->outEdges(): std::list<PartEdgePtr>(), NodeState_valid,
                                 indexes_valid?   index()->outEdges():     std::list<PartEdgePtr>(), indexes_valid,
                                 inputs_valid?    input()->outEdges():     std::list<PartEdgePtr>(), inputs_valid);
  }

  AnalysisPartEdgeLists inEdges(bool NodeState_valid=true, bool indexes_valid=true, bool inputs_valid=true) {
    return AnalysisPartEdgeLists(NodeState_valid? NodeState()->inEdges(): std::list<PartEdgePtr>(), NodeState_valid,
                                 indexes_valid?   index()->inEdges():     std::list<PartEdgePtr>(), indexes_valid,
                                 inputs_valid?    input()->inEdges():     std::list<PartEdgePtr>(), inputs_valid);
  }

  // Returns a wildcard edge that denotes an incoming edge from any source Part, focusing on all or a subset of the edge types
  AnalysisATS<PartEdgePtr> inEdgeFromAny(bool NodeState_valid=true, bool indexes_valid=true, bool inputs_valid=true) {
    return AnalysisATS<PartEdgePtr>(NodeState_valid? NodeState()->inEdgeFromAny(): NULLPartEdge, NodeState_valid,
                                    indexes_valid?   index()->inEdgeFromAny():     NULLPartEdge, indexes_valid,
                                    inputs_valid?    input()->inEdgeFromAny():     NULLPartEdge, inputs_valid);
  }

  // Returns a wildcard edge that denotes an outgoing edge to any target Part, focusing on all or a subset of the edge types
  AnalysisATS<PartEdgePtr> outEdgeToAny(bool NodeState_valid=true, bool indexes_valid=true, bool inputs_valid=true) {
    return AnalysisATS<PartEdgePtr>(NodeState_valid? NodeState()->outEdgeToAny(): NULLPartEdge, NodeState_valid,
                                    indexes_valid?   index()->outEdgeToAny():     NULLPartEdge, indexes_valid,
                                    inputs_valid?    input()->outEdgeToAny():     NULLPartEdge, inputs_valid);
  }*/

  // Returns the list of this Part's outgoing edges, focusing on all of the edge types
  AnalysisPartEdgeLists outEdgesAll(NodeState2AllMapper& mapper);

  AnalysisPartEdgeLists inEdgesAll(NodeState2AllMapper& mapper);

  // Returns the list of this Part's outgoing edges, focusing on index and input edge types
  AnalysisPartEdgeLists outEdgesIndexInput(NodeState2AllMapper& mapper);

  AnalysisPartEdgeLists inEdgesIndexInput(NodeState2AllMapper& mapper);

  // Returns a wildcard edge that denotes an incoming edge from any source Part, focusing on all of the edge types
  AnalysisATS<PartEdgePtr> inEdgeFromAnyAll(NodeState2AllMapper& mapper);

  // Returns a wildcard edge that denotes an outgoing edge to any target Part, focusing on all or a subset of the edge types
  AnalysisATS<PartEdgePtr> outEdgeToAnyAll(NodeState2AllMapper& mapper);

  // Returns a wildcard edge that denotes an incoming edge from any source Part, focusing on all or a subset of the edge types
  AnalysisATS<PartEdgePtr> inEdgeFromAnyIndexInput(NodeState2AllMapper& mapper);

  // Returns a wildcard edge that denotes an outgoing edge to any target Part, focusing on all or a subset of the edge types
  AnalysisATS<PartEdgePtr> outEdgeToAnyIndexInput(NodeState2AllMapper& mapper);

  // Returns the Part that denotes the Input of the given Part
  static PartPtr getInput(const PartPtr& part) { return part->getInputPart(); }
};

// Class that encapsulates all the various types of information analyses need
// to know about ATS PartEdges while they execute
class AnalysisPartEdges : public AnalysisATS<PartEdgePtr>
{
  public:
  AnalysisPartEdges() {}

  AnalysisPartEdges(PartEdgePtr NodeState, PartEdgePtr index, PartEdgePtr input) :
    AnalysisATS<PartEdgePtr>(NodeState, index, input)
  {}

  AnalysisPartEdges(PartEdgePtr NodeState, bool NodeState_valid,
                    PartEdgePtr index,     bool index_valid,
                    PartEdgePtr input,     bool input_valid) :
    AnalysisATS<PartEdgePtr>(NodeState, NodeState_valid,
                             index,     index_valid,
                             input,     input_valid)
  {}

  AnalysisPartEdges(const AnalysisPartEdges& that) : AnalysisATS<PartEdgePtr>(that) {}
  AnalysisPartEdges(const AnalysisATS<PartEdgePtr>& that) : AnalysisATS<PartEdgePtr>(that) {}

  // Return the AnalysisPart that denotes the source of this edge
  AnalysisParts source() { return AnalysisParts(NodeState_->source(), index_->source(), input_->source()); }

  // Return the AnalysisPart that denotes the target of this edge
  AnalysisParts target() { return AnalysisParts(NodeState()->target(), index()->target(), input()->target()); }

  // Returns the PartEdge that denotes the Input of the given PartEdge
  static PartEdgePtr getInput(const PartEdgePtr& pedge) { return pedge->getInputPartEdge(); }
}; // class AnalysisPartEdges


// Interface implemented by entities that are able to generate AnalysisATS objects (AnalysisParts and
// AnalysisPartEdges) from NodeState Parts and PartEdges. This interface is only implemented by Composers.
class NodeState2AllMapper {
public:
  // Given a Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisParts object that contains all the Parts relevant for analysis.
  virtual AnalysisParts NodeState2All(PartPtr part, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true)=0;

  // Given a PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis.
  virtual AnalysisPartEdges NodeState2All(PartEdgePtr pedge, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true)=0;

  // Given a set of Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartSets object that contains all the sets of Parts relevant for analysis.
  AnalysisPartSets NodeState2All(const std::set<PartPtr>& parts, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a set of PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdgeSets object that contains all the sets of PartEdges relevant for analysis.
  AnalysisPartEdgeSets NodeState2All(const std::set<PartEdgePtr>& pedges, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a list of Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartlists object that contains all the lists of Parts relevant for analysis.
  AnalysisPartLists NodeState2All(const std::list<PartPtr>& parts, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a list of PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdgelists object that contains all the lists of PartEdges relevant for analysis.
  AnalysisPartEdgeLists NodeState2All(const std::list<PartEdgePtr>& pedges, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a Part from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisParts object that contains the input and index the Parts relevant for analysis.
  virtual AnalysisParts Input2Index(PartPtr part, bool indexes_valid=true, bool inputs_valid=true)=0;

  // Given a PartEdge from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisPartEdges object that contains the input and index the Parts relevant for analysis.
  virtual AnalysisPartEdges Input2Index(PartEdgePtr pedge, bool indexes_valid=true, bool inputs_valid=true)=0;

  // Given a set of Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartSets object that contains all the sets of Parts relevant for analysis.
  AnalysisPartSets Input2Index(const std::set<PartPtr>& parts, bool indexes_valid=true, bool inputs_valid=true);

  // Given a set of PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdgeSets object that contains all the sets of PartEdges relevant for analysis.
  AnalysisPartEdgeSets Input2Index(const std::set<PartEdgePtr>& pedges, bool indexes_valid=true, bool inputs_valid=true);

  // Given a list of Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartlists object that contains all the lists of Parts relevant for analysis.
  AnalysisPartLists Input2Index(const std::list<PartPtr>& parts, bool indexes_valid=true, bool inputs_valid=true);

  // Given a list of PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdgelists object that contains all the lists of PartEdges relevant for analysis.
  AnalysisPartEdgeLists Input2Index(const std::list<PartEdgePtr>& pedges, bool indexes_valid=true, bool inputs_valid=true);
};

/// Apply an analysis A's transfer function at a particular AST node type
class DFTransferVisitor : public ROSE_VisitorPatternDefaultBase
{
  protected:
  // Common arguments to the underlying transfer function
  AnalysisParts parts;
  CFGNode cn;
  NodeState &state;
  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo;

  public:
  DFTransferVisitor(const AnalysisParts& parts, CFGNode cn, NodeState &state,
                         std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)
    : parts(parts), cn(cn), state(state), dfInfo(dfInfo)
  { }

  //PartPtr getPart() const { return part; }
  NodeState&  getNodeState()      const { return state; }
  const AnalysisParts& getParts() const { return parts; }
  CFGNode getCFGNode()            const { return cn; }

  virtual bool finish() = 0;
  virtual ~DFTransferVisitor() { }
};

class Lattice;
class Dataflow : virtual public Analysis
{
  public:

  // the transfer function that is applied to every node
  // part - The Part that is being processed
  // state - the NodeState object that describes the state of the node, as established by earlier
  //   analysis passes
  // dfInfo - The Lattices that this transfer function operates on. The function take a map of lattices, one for
  //   each edge that departs from this part (outgoing for forward analyses and incoming for backwards)
  //   as input and overwrites them with the result of the transfer.
  // Returns true if any of the input lattices changed as a result of the transfer function and
  //    false otherwise.
  virtual bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                        std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)=0;

  class DefaultTransfer : public DFTransferVisitor
  {
    bool modified;
    Dataflow *analysis;
    public:
    DefaultTransfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
        std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, Dataflow *a)
      : DFTransferVisitor(parts, cn, state, dfInfo), modified(false), analysis(a)
      { }

    void visit(SgNode *n) { modified = analysis->transfer(parts, cn, state, dfInfo); }
    bool finish() { return modified; }
  };


  // \todo \pp IMO. the function getTransferVisitor is not necessary and can be removed.
  //     Users wanting to write the analysis based on visitors can do so
  //     in the transfer function. (This safes one memory allocation, deallocation,
  //     and boost::shared_pointer management overhead per transfer).
  //     A transfer function using the visitor would look like (if desired this can be
  //     simplified by providing a convenience function taking a visitor as argument):
  // \code
  //     virtual bool transfer(const Function& func, PartPtr p, NodeState& state, const std::vector<Lattice*>& dfInfo, std::vector<Lattice*>** retState, bool fw)
  //     {
  //       MyTransferVisitor visitor(myarguments, func, n, ...);
  //       n.getNode().accept(visitor);
  //       return visitor.finish();
  //     }
  // \endcode
  virtual boost::shared_ptr<DFTransferVisitor> getTransferVisitor(
                AnalysisParts& parts, CFGNode cn,
                NodeState& state,
                std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)
  { return boost::shared_ptr<DFTransferVisitor>(new DefaultTransfer(/*part,supersetPart*/ parts, cn, state, dfInfo, this)); }
};

}; // namespace fuse
#endif
