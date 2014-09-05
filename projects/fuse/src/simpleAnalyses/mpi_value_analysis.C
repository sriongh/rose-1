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
    : DFTransferVisitor(_part, _cfgn, _state, _dfInfo),
      part(_part),
      cfgn(_cfgn),
      state(_state),
      dfInfo(_dfInfo),
      analysis(_analysis),
      modified(false),
      composer(_composer) {
    // Incoming dfInfo is associated with inEdgeFromAny
    assert(dfInfo.size()==1);    
    assert(dfInfo[part->inEdgeFromAny()].size()==1);
    assert(*dfInfo[part->inEdgeFromAny()].begin());

    Lattice *l = *dfInfo[part->inEdgeFromAny()].begin();
    mpiValuesMap = (dynamic_cast<AbstractObjectMap*>(l));
    assert(mpiValuesMap);
  }

  bool MVATransferVisitor::setLattice(SgExpression* expr, MPIValueObjectPtr mvo_p) {
    MemLocObjectPtr ml_p = composer->Expr2MemLoc(expr, part->inEdgeFromAny(), analysis);
    return mpiValuesMap->insert(ml_p, boost::dynamic_pointer_cast<Lattice>(mvo_p));
  }

  SgExpression* MVATransferVisitor::getOpCastExpr(SgExpression* exp) {
    ROSE_ASSERT(isSgCastExp(exp));
    return isSgCastExp(exp)->get_operand();
  }

  SgExpression* MVATransferVisitor::getOpAddrOfExpr(SgExpression* exp) {
    ROSE_ASSERT(isSgAddressOfOp(exp));
    return isSgAddressOfOp(exp)->get_operand();
  }

  bool MVATransferVisitor::isMPIFuncCall(const Function& func) {
    if(func.get_name().getString().find("MPI_") == 0)
      return true;
    return false;
  }

  void  MVATransferVisitor::visit(SgFunctionCallExp* sgn) {
    // if the function call exp is a MPI operation
    // and if the abstract state (part) denotes entry back to caller
    // and if the call site is not inside MPI library
    // set the mpi value for the variables
    Function func(sgn);
    if(isMPIFuncCall(func) && 
       part->isIncomingFuncCall(cfgn)) {
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
    // assert it has only two arguments
    ROSE_ASSERT(args_list.size() == 2);
    // strip cast and get the comm expression
    SgExpression* commExpr = getOpCastExpr(args_list[0]);

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
    MPI_Comm comm = commValue->get_value();

    // issue the MPI_Comm_rank operation
    int pid;
    MPI_Comm_rank(comm, &pid);

    // create MPIValueObject for the pid
    CPConcreteKindPtr pidValKind = boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(pid)));
    MPIValueObjectPtr pidMVO = boost::make_shared<MPIValueObject>(pidValKind,
                                                                  part->inEdgeFromAny());
    // set the lattice for the second argument
    modified = setLattice(getOpAddrOfExpr(args_list[1]), pidMVO);
  }

  void MVATransferVisitor::transferCommSize(SgFunctionCallExp* sgn) {
    // get the argument list
    SgExpressionPtrList& args_list = sgn->get_args()->get_expressions();
    // assert it has only two arguments
    ROSE_ASSERT(args_list.size() == 2);
    // strip cast and get the comm expression
    SgExpression* commExpr = getOpCastExpr(args_list[0]);    
    ValueObjectPtr commVO = composer->Expr2Val(commExpr, part->inEdgeFromAny());
    ROSE_ASSERT(commVO->isConcrete());
    set<boost::shared_ptr<SgValueExp> > commVals = commVO->getConcreteValue();
    // there should be only one value
    ROSE_ASSERT(commVals.size()==1);
    // get the value expression
    boost::shared_ptr<SgIntVal> commValue = boost::dynamic_pointer_cast<SgIntVal>(*commVals.begin()); 
    MPI_Comm comm = commValue->get_value();

    // issue the MPI_Comm_size operation
    int size;
    MPI_Comm_size(comm, &size);

    // create MPIValueObject for the pid
    CPConcreteKindPtr sizeValKind = boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(size)));
    MPIValueObjectPtr sizeMVO = boost::make_shared<MPIValueObject>(sizeValKind,
                                                                   part->inEdgeFromAny());
    // set the lattice for the second argument
    modified = setLattice(getOpAddrOfExpr(args_list[1]), sizeMVO);
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

  ValueObjectPtr MPIValueAnalysis::Expr2Val(SgNode* n, PartEdgePtr pedge) {
    scope s(sight::txt()<<"MPIValueAnalysis::Expr2Val(n="<<SgNode2Str(n)<<
            ", pedge="<<pedge->str()<<")", scope::medium, attrGE("mpiValueAnalysisDebugLevel", 2));

    MemLocObjectPtr ml = getComposer()->Expr2MemLoc(n, pedge, this);
    if(mpiValueAnalysisDebugLevel()>=1) dbg << "ml="<<(ml? ml->str(): "NULL")<<endl;

    // if the edge is not a wildcard
    if(pedge->source()) {
      NodeState* state = NodeState::getNodeState(this, pedge->source());
      // specific edges are exposed to transfer functions
      // in a fwd analysis edge specific information are associated below
      Lattice* l = state->getLatticeBelow(this, pedge, 0);
      AbstractObjectMap* mvMap = dynamic_cast<AbstractObjectMap*>(l);
      ROSE_ASSERT(mvMap);

      // We currently can only handle requests for the SgNode that corresponds to the current Part
      set<CFGNode> nodes = pedge->source()->CFGNodes();
      assert(nodes.size()==1);
      assert(nodes.begin()->getNode() == n);

      // print debug info
      if(mpiValueAnalysisDebugLevel() >= 2) {
        dbg << "mvMap Below=" << mvMap << "=" << mvMap->str() << endl;
        dbg << "state=" << state->str() << endl;
      }

      MPIValueObjectPtr mvo = boost::dynamic_pointer_cast<MPIValueObject>(mvMap->get(ml));
      ROSE_ASSERT(mvo);
      return mvo->copyV();
    }
    else if(pedge->target()) {
      NodeState* state = NodeState::getNodeState(this, pedge->target());
      Lattice* l = state->getLatticeAbove(this, pedge, 0);
      AbstractObjectMap* mvMap = dynamic_cast<AbstractObjectMap*>(l);
      ROSE_ASSERT(mvMap);

      // We currently can only handle requests for the SgNode that corresponds to the current Part
      set<CFGNode> nodes = pedge->target()->CFGNodes();
      assert(nodes.size()==1);
      assert(nodes.begin()->getNode() == n);

      // print debug info
      if(mpiValueAnalysisDebugLevel() >= 2) {
        dbg << "mvMap Above=" << mvMap << "=" << mvMap->str() << endl;
        dbg << "state=" << state->str() << endl;
      }

      MPIValueObjectPtr mvo = boost::dynamic_pointer_cast<MPIValueObject>(mvMap->get(ml));
      ROSE_ASSERT(mvo);
      return mvo->copyV();
    }
    else ROSE_ASSERT(false);
  }

  std::string MPIValueAnalysis::str(std::string indent="") const {
    return "MPIValueAnalysis";
  }
      
}; // end namespace
