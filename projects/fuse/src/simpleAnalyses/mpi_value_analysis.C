/*****************************************
 * author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "sage3basic.h"
#include "mpi_value_analysis.h"
#include "const_prop_analysis.h"

namespace fuse {

  DEBUG_LEVEL(mpiValueAnalysisDebugLevel, 2);

  /******************
   * MPIValueObject *
   ******************/

  MPIValueObject::MPIValueObject(PartEdgePtr pedge) :
    Lattice(pedge), FiniteLattice(pedge),
    ValueObject(NULL) {
    kind = boost::make_shared<CPUninitializedKind>();
  }

  MPIValueObject::MPIValueObject(CPValueKindPtr _kind, PartEdgePtr pedge)
    : Lattice(pedge), FiniteLattice(pedge),
      ValueObject(NULL),
      kind(_kind) {
  }
  
  // Do we need th copy constructor?
  MPIValueObject::MPIValueObject(const MPIValueObject & that) :
    Lattice(that.getPartEdge()), FiniteLattice(that.getPartEdge()),
    ValueObject(that),
    kind(that.kind) {
  }
  
  // Access functions.
  CPValueKindPtr MPIValueObject::getKind() const {
    return kind;
  }
  // Sets this object's kind to the given kind, returning true if this causes the MPIValueObject to change
  bool MPIValueObject::setKind(CPValueKindPtr _kind) {
    bool modified = (kind->getKind() == _kind->getKind() &&
                     kind->equalSetV(_kind));
    kind = _kind;
    return modified;
  }
  
  void MPIValueObject::initialize() {
  }
  
  // returns a copy of this lattice
  Lattice* MPIValueObject::copy() const {
    return new MPIValueObject(*this);
  }
  
  // overwrites the state of "this" Lattice with "that" Lattice
  void MPIValueObject::copy(Lattice* that) {
    Lattice::copy(that);
    MPIValueObject* thatMVO = dynamic_cast<MPIValueObject*>(that);
    ROSE_ASSERT(thatMVO);
    kind = thatMVO->getKind();
  }
  
  // compare the kind enum first
  // check if the two kind denote same set of values
  bool MPIValueObject::operator==(Lattice* that) {
    MPIValueObject* thatMVO = dynamic_cast<MPIValueObject*>(that);
    ROSE_ASSERT(thatMVO);
    return (kind->getKind() == thatMVO->getKind()->getKind() &&
            kind->equalSetV(thatMVO->getKind()));
  }
  
  bool MPIValueObject::meetUpdate(Lattice* that) {
    MPIValueObject* thatMVO = dynamic_cast<MPIValueObject*>(that);
    return meetUpdate(thatMVO);
  }

  // computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool MPIValueObject::meetUpdate(MPIValueObject* that) {
    std::pair<bool, CPValueKindPtr> result = kind->meetUpdateV(that->getKind());
    kind = result.second;
    return result.first;
  }
  
  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  bool MPIValueObject::setToFull() {
    if(kind->getKind() != CPValueKind::unknown) {
      kind = boost::make_shared<CPUnknownKind>();
      return true;
    }
    return false;
  }
  
  // Set this Lattice object to represent the of no execution prefixes (empty set)
  // Return true if this causes the object to change and false otherwise.
  bool MPIValueObject::setToEmpty() {
    if(kind->getKind() != CPValueKind::uninitialized) {
      kind = boost::make_shared<CPUninitializedKind>();
      return true;
    }
    return false;
  }
  
  // Returns whether this lattice denotes the set of all possible execution prefixes.
  bool MPIValueObject::isFullLat() {
    return isFullV(getPartEdge());
  }
  // Returns whether this lattice denotes the empty set.
  bool MPIValueObject::isEmptyLat() {
    return isEmptyV(getPartEdge());
  }

  bool MPIValueObject::setMLValueToFull(MemLocObjectPtr ml) {
    // since this is value object do nothing
    return false;
  }
       
  bool MPIValueObject::mayEqualV(ValueObjectPtr o, PartEdgePtr pedge) {
    MPIValueObjectPtr mvo_p = boost::dynamic_pointer_cast<MPIValueObject>(o);
    ROSE_ASSERT(mvo_p);
    return kind->mayEqualV(mvo_p->getKind());
  }

  bool MPIValueObject::mustEqualV(ValueObjectPtr o, PartEdgePtr pedge) {
    MPIValueObjectPtr mvo_p = boost::dynamic_pointer_cast<MPIValueObject>(o);
    ROSE_ASSERT(mvo_p);
    return kind->mustEqualV(mvo_p->getKind());

  }
 
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool MPIValueObject::equalSetV(ValueObjectPtr o, PartEdgePtr pedge) {
    MPIValueObjectPtr mvo_p = boost::dynamic_pointer_cast<MPIValueObject>(o);
    ROSE_ASSERT(mvo_p);
    return kind->equalSetV(mvo_p->getKind());
  }
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool MPIValueObject::subSetV(ValueObjectPtr o, PartEdgePtr pedge) {
    MPIValueObjectPtr mvo_p = boost::dynamic_pointer_cast<MPIValueObject>(o);
    ROSE_ASSERT(mvo_p);
    return kind->subSetV(mvo_p->getKind());
  }
  
    // Computes the meet of this and that and returns the resulting kind
  bool MPIValueObject::meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge) {
    MPIValueObjectPtr mvo_p = boost::dynamic_pointer_cast<MPIValueObject>(that);
    ROSE_ASSERT(mvo_p);
    return meetUpdate(mvo_p.get());
  }
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool MPIValueObject::isFullV(PartEdgePtr pedge) {
    return kind->isFullV(pedge);
  }

  // Returns whether this AbstractObject denotes the empty set.
  bool MPIValueObject::isEmptyV(PartEdgePtr pedge) {
    return kind->isEmptyV(pedge);
  }
  
  // Allocates a copy of this object and returns a pointer to it
  ValueObjectPtr MPIValueObject::copyV() const {
    return boost::make_shared<MPIValueObject>(*this);
  }

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool MPIValueObject::isConcrete() {
    return kind->getKind() == CPValueKind::concrete;
  }

  SgType* MPIValueObject::getConcreteType() {
    ROSE_ASSERT(isConcrete());
    return kind->getConcreteType();
  }
  
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  set<boost::shared_ptr<SgValueExp> > MPIValueObject::getConcreteValue() {
    ROSE_ASSERT(isConcrete());
    return kind->getConcreteValue();
  }

  // pretty print for the object
  std::string MPIValueObject::str(std::string indent) const {
    return sight::txt()<<"MPIValueObject: " << kind->str() << "]";
  }

  std::string MPIValueObject::strp(PartEdgePtr pedge, std::string indent) const {
    return str(indent);
  }
  
  /**********************
   * MVATransferVisitor *
   **********************/

  MVATransferVisitor::MVATransferVisitor(PartPtr _part,
                                         CFGNode _cfgn,
                                         NodeState& _state,
                                         std::map<PartEdgePtr, std::vector<Lattice*> >& _dfInfo,
                                         Composer* _composer,
                                         MPIValueAnalysis* _analysis)
    : VariableStateTransfer<MPIValueObject, MPIValueAnalysis> (_state, _dfInfo,
                                                               boost::make_shared<MPIValueObject>(_part->inEdgeFromAny()),
                                                               _composer, _analysis, _part, _cfgn,
                                                               mpiValueAnalysisDebugLevel, "mpiValueAnalysisDebugLevel"),
                                               part(_part),
                                               cfgn(_cfgn),
                                               state(_state),
                                               dfInfo(_dfInfo),
                                               modified(false),
                                               composer(_composer) {
  }


  void  MVATransferVisitor::visit(SgFunctionCallExp* sgn) {
    // if the function call exp is a MPI operation
    // and if the abstract state (part) denotes entry back to caller
    // set the mpi value for the variables
    Function func(sgn);
    if(isMPIFuncCall(func) && part->isIncomingFuncCall(cfgn)) {
      string func_name = func.get_name().getString();
      if(func_name == "MPI_Comm_rank") {
        transferCommRank(sgn);
      }
      else if(func_name == "MPI_Comm_size") {
        transferCommSize(sgn);
      }
    }
  }

  // transfer function for MPI_Comm_rank(comm, &rank) 
  // first argument is the communicator(MPI_Comm), second argument is the pid variable
  // MPI_Comm_rank assigns integer value to the second argument from MPI runtime
  // transferCommRank assigns MPIValueObject to the second argument by
  // executing MPI_Comm_rank on the first argument
  void MVATransferVisitor::transferCommRank(SgFunctionCallExp* sgn) {
    // get the argument list
    SgExpressionPtrList& args_list = sgn->get_args()->get_expressions();
    // strip cast and get the comm expression
    SgExpression* commExpr = isSgCastExp(args_list[0])->get_operand();
    // MPI_Comms are integer values assigned by MPI runtime
    // before issuing MPI_Comm_rank the integer value for the comm should be known
    // query the composer for the value of comm expr
    // MPI_COMM_WORLD is a special integer and the composer returns this special integer
    // MPI_Comm variables on the other hand are like regular int variables assigned a value
    // by calls such as MPI_Comm_split
    // NOTE: values for MPI_Comm variables can be obtained by implementing 
    // MPIValueAnalsis as a tight compositional analysis 
    ValueObjectPtr commVO = composer->Expr2Val(commExpr, part->inEdgeFromAny());
    ROSE_ASSERT(commVO->isConcrete());
    set<boost::shared_ptr<SgValueExp> > commVals = commVO->getConcreteValue();
    // there should be only one value
    ROSE_ASSERT(commVals.size()==1);
    // get the value expression
    boost::shared_ptr<SgIntVal> commValue = boost::dynamic_pointer_cast<SgIntVal>(*commVals.begin());
    // SgIntVal& commValue = dynamic_cast<SgIntVal&>((*commVals.begin()).get());
    MPI_Comm comm = commValue->get_value();
    // issue the MPI_Comm_rank operation
    int pid;
    MPI_Comm_rank(comm, &pid);
    // create MPIValueObject for the pid
    // boost::shared_ptr<SgValueExp> pidVal = boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(pid));
    MPIValueObjectPtr pidMVO = boost::make_shared<MPIValueObject> (boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(pid))),
                                                                   part->inEdgeFromAny());
    // set the lattice for the second argument
    setLatticeOperand(args_list[1],
                      isSgAddressOfOp(args_list[1])->get_operand(),
                      pidMVO);

  }

  void MVATransferVisitor::transferCommSize(SgFunctionCallExp* sgn) {
  }

  bool MVATransferVisitor::isMPIFuncCall(const Function& func) {
    if(func.get_name().getString().find("MPI_") == 0)
      return true;
    return false;
  }

  bool MVATransferVisitor::finish() {
    return modified;
  }

  /*******************
   * MPIValueAnalysis *
   *******************/

  MPIValueAnalysis::MPIValueAnalysis()
    : FWDataflow() {
  }
  
  ComposedAnalysisPtr MPIValueAnalysis::copy() {
    return boost::make_shared<MPIValueAnalysis>();
  }

  void MPIValueAnalysis::genInitLattice(PartPtr part, 
                                       PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    AbstractObjectMap* aMap = new AbstractObjectMap(boost::make_shared<MPIValueObject>(pedge),
                                                    pedge,
                                                    getComposer(),
                                                    this);
    initLattices.push_back(aMap);
  }

  bool MPIValueAnalysis::transfer(PartPtr part, 
                                 CFGNode cfgn, 
                                 NodeState& state, 
                                 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    assert(0);
    return false;
  }

  boost::shared_ptr<DFTransferVisitor>
  MPIValueAnalysis::getTransferVisitor(PartPtr part, 
                                      CFGNode cfgn, 
                                      NodeState& state, 
                                      std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    return boost::shared_ptr<DFTransferVisitor>(new MVATransferVisitor(part, cfgn, state, dfInfo, getComposer(), this));
  }

  bool MPIValueAnalysis::implementsExpr2Val() {
    return true;
  }

  ValueObjectPtr MPIValueAnalysis::Expr2Val(SgNode* e, PartEdgePtr pedge) {
    return NULLValueObject;
  }

  std::string MPIValueAnalysis::str(std::string indent="") const {
    return "MPIValueAnalysis";
  }
      
}; // end namespace
