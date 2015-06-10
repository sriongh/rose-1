/*****************************************
 * Author: Sriram Aananthakrishnan, 2015 *
 *****************************************/

#include "sage3basic.h"
#include "mpi_comm_analysis.h"
#include "abstract_object_map.h"

using namespace std;
using namespace boost;

namespace fuse {

  /**********************
   * MPICommValueObject *
   **********************/
  MPICommValueObject::MPICommValueObject(PartEdgePtr pedge)
    : Lattice(pedge),
      FiniteLattice(pedge),
      ValueObject() {
  }

  MPICommValueObject::MPICommValueObject(const MPICommValueObject& that)
    : Lattice(that),
      FiniteLattice(that),
      ValueObject(that) {
  }

  void MPICommValueObject::initialize() {
    assert(0);
  }

  Lattice* MPICommValueObject::copy() const {
    assert(0);
  }
   
  bool MPICommValueObject::meetUpdate(Lattice* thatL) {
    assert(0);
  }

  bool MPICommValueObject::operator==(Lattice* thatL) {
    assert(0);
  }

  bool MPICommValueObject::setToFull() {
    assert(0);
  }

  bool MPICommValueObject::setToEmpty()  {
    assert(0);
  }

  bool MPICommValueObject::setMLValueToFull(MemLocObjectPtr ml) {
    assert(0);
  }

  bool MPICommValueObject::isFullLat() {
    assert(0);
  }

  bool MPICommValueObject::isEmptyLat() {
    assert(0);
  }

  bool MPICommValueObject::mayEqualV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::mustEqualV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::equalSetV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::subSetV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::meetUpdateV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::isEmptyV(PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::isFullV(PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::isConcrete() {
    assert(0);
  }

  SgType* MPICommValueObject::getConcreteType() {
    assert(0);
  }

  set<boost::shared_ptr<SgValueExp> > MPICommValueObject::getConcreteValue() {
    assert(0);
  }

  ValueObjectPtr MPICommValueObject::copyV() const {
    assert(0);
  }

  string MPICommValueObject::str(string indent) const  {
    assert(0);
  }

  /*******************
   * MPICommAnalysis *
   *******************/
  MPICommAnalysis::MPICommAnalysis() {
  }

  // void MPICommAnalysis::initAnalysis(set<PartPtr>& startingParts) {
  //   assert(0);
  // }

  void MPICommAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    AbstractObjectMap* aomap = new AbstractObjectMap(boost::make_shared<MPICommValueObject>(pedge), 
                                                     pedge, 
                                                     getComposer(), 
                                                     this);
    initLattices.push_back(aomap);
  }

  bool MPICommAnalysis::transfer(PartPtr part, CFGNode cn, NodeState& state, 
                                 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    assert(0);
  }

  ValueObjectPtr MPICommAnalysis::Expr2Val(SgNode* sgn, PartEdgePtr pedge) {
    assert(0);
  }

  string MPICommAnalysis::str(std::string indent) const {
    return "MPICommAnalysis";
  }
}
