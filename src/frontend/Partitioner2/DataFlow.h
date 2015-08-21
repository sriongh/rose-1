#ifndef ROSE_Partitioner2_DataFlow_H
#define ROSE_Partitioner2_DataFlow_H

#include <BinaryDataFlow.h>
#include <Partitioner2/BasicBlock.h>
#include <Partitioner2/ControlFlowGraph.h>
#include <Partitioner2/Function.h>
#include <sawyer/Graph.h>

namespace rose {
namespace BinaryAnalysis {
namespace Partitioner2 {

/** Dataflow utilities. */
namespace DataFlow {


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Control Flow Graph
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** CFG vertex for dataflow analysis.
 *
 *  @sa DfCfg */
class DfCfgVertex {
public:
    /** Vertex type. */
    enum Type {
        BBLOCK,                                         /**< Vertex represents a basic block. */
        FAKED_CALL,                                     /**< Represents a whole called function. */
        FUNCRET,                                        /**< Vertex represents returning to the caller. */
        INDET,                                          /**< Indeterminate basic block where no information is available. */
    };

private:
    Type type_;
    BasicBlock::Ptr bblock_;                            // attached to BBLOCK vertices
    Function::Ptr callee_;                              // function represented by FAKED_CALL

public:
    /** Construct a basic block vertex.  The basic block pointer should not be a null pointer. */
    explicit DfCfgVertex(const BasicBlock::Ptr &bblock): type_(BBLOCK), bblock_(bblock) {
        ASSERT_not_null(bblock);
    }

    /** Construct a faked call vertex. The function may be null if indeterminate. */
    explicit DfCfgVertex(const Function::Ptr &function): type_(FAKED_CALL), callee_(function) {}
    
    /** Construct a vertex of specified type that takes no auxiliary data. */
    explicit DfCfgVertex(Type type): type_(type) {
        ASSERT_require2(BBLOCK!=type && FAKED_CALL!=type, "use a different constructor");
    }

    /** Type of the vertex.
     *
     *  Vertex types are immutable, defined when the vertex is created. Every vertex has a type. */
    Type type() const { return type_; }

    /** Basic block.
     *
     *  The basic block for a vertex is immutable, defined when the vertex is created.  Only basic block vertices have a basic
     *  block; other vertex types will return a null pointer. */
    const BasicBlock::Ptr& bblock() const { return bblock_; }

    /** Function represented by faked call. */
    const Function::Ptr& callee() const { return callee_; }
};

/** Control flow graph used by dataflow analysis.
 *
 *  The CFG used for dataflow is slightly different than the global CFG maintained by the partitioner. The partitioner's global
 *  CFG is tuned for discovering basic blocks and deciding which basic blocks are owned by which functions, whereas a
 *  dataflow's CFG is tuned for performing data flow analysis.  A dataflow CFG is usually constructed from the partitioner's
 *  global CFG, but differs in the following ways:
 *
 *  @li First, dataflow analysis is usually performed on a subset of the partitioner's global CFG. This function uses the @p
 *      startVertex to select some connected subgraph, such as a subgraph corresponding to a single function definition when
 *      given the entry block.
 *
 *  @li Function return blocks (e.g., RET instructions) are handled differently during dataflow.  In the partitioner's global
 *      CFG each return block is marked as a function return and has single successor--the indeterminate vertex.  In a dataflow
 *      CFG the return blocks are not handled specially, but rather all flow into a single special return vertex that has no
 *      instructions.  This allows data to be merged from all the return points.
 *
 *  @li Function call sites are modified.  In the partitioner global CFG a function call (e.g., CALL instruction) has an edge
 *      (or edges) going to the entry block of the called function(s) and a special call-return edge to the return site if
 *      there is one (usually the fall-through address). A data-flow analysis often needs to perform some special action for
 *      the call-return, thus a call-return edge in the global CFG gets transformed to an edge-vertex-edge sequence in the
 *      dataflow CFG where the middle vertex is a special CALLRET vertex with no instructions. */
typedef Sawyer::Container::Graph<DfCfgVertex> DfCfg;

/** Predicate that decides when to use inter-procedural dataflow.
 *
 *  The predicate is invoked with the global CFG and a function call edge and should return true if the called function should
 *  be included into the dataflow graph.  If it returns false then the graph will have a single FAKED_CALL vertex to represent
 *  the called function. */
class InterproceduralPredicate {
public:
    virtual ~InterproceduralPredicate() {}
    virtual bool operator()(const ControlFlowGraph&, const ControlFlowGraph::ConstEdgeNodeIterator&, size_t depth) = 0;
};

/** Predicate that always returns false, preventing interprocedural analysis. */
class NotInterprocedural: public InterproceduralPredicate {
public:
    bool operator()(const ControlFlowGraph&, const ControlFlowGraph::ConstEdgeNodeIterator&, size_t depth) ROSE_OVERRIDE {
        return false;
    }
};
extern NotInterprocedural NOT_INTERPROCEDURAL;

/** Unpacks a vertex into a list of instructions. */
std::vector<SgAsmInstruction*> vertexUnpacker(const DfCfgVertex&);

/** build a cfg useful for dataflow analysis.
 *
 *  The returned CFG will be constructed from the global CFG vertices that are reachable from @p startVertex such that the
 *  reached vertex belongs to the same function as @p startVertex.
 *
 *  @sa DfCfg */
DfCfg buildDfCfg(const Partitioner&, const ControlFlowGraph&, const ControlFlowGraph::ConstVertexNodeIterator &startVertex,
                 InterproceduralPredicate &predicate = NOT_INTERPROCEDURAL);

/** Emit a dataflow CFG as a GraphViz file. */
void dumpDfCfg(std::ostream&, const DfCfg&);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Dataflow State
//
// Any state can be used in the calls to the generic BinaryAnalysis::DataFlow stuff, but we define a state here based on
// symbolic semantics because that's what's commonly wanted.  Users are free to create their own states either from scratch or
// by inheriting from the one defined here.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** A multi-byte variable that appears on the stack. */
struct StackVariable {
    int64_t offset;                                     /**< Offset from initial stack pointer. */
    size_t nBytes;                                      /**< Size of variable in bytes. */
    BaseSemantics::SValuePtr address;                   /**< Starting address, i.e., initial stack pointer + offset. */

    /** Create a new stack variable. The @p offset is the starting (low) address of the variable and @p addr is the symbolic
     *  address for that offset. */
    StackVariable(int64_t offset, size_t nBytes, const BaseSemantics::SValuePtr &address)
        : offset(offset), nBytes(nBytes), address(address) {}
};

/** Multiple stack variables. */
typedef std::vector<StackVariable> StackVariables;

/** State for dataflow. Mostly the same as a semantic state. */
class State: public Sawyer::SharedObject {
public:
    typedef Sawyer::SharedPointer<State> Ptr;

private:
    BaseSemantics::RiscOperatorsPtr ops_;
    BaseSemantics::StatePtr semanticState_;

protected:
    explicit State(const BaseSemantics::RiscOperatorsPtr &ops)
        : ops_(ops) {
        init();
    }

    // Deep copy
    State(const State &other): ops_(other.ops_) {
        semanticState_ = other.semanticState_->clone();
    }


public:
    // Allocating constructor
    static Ptr instance(const BaseSemantics::RiscOperatorsPtr &ops) {
        return Ptr(new State(ops));
    }

    // Copy + allocate constructor
    Ptr clone() const {
        return Ptr(new State(*this));
    }

    void clear() {
        semanticState_->clear();
    }

    BaseSemantics::StatePtr semanticState() const { return semanticState_; }
    
    /** Returns the list of all known stack variables.  A stack variable is any memory location whose address is a constant
     *  offset from an initial stack pointer.  That is, the address has the form (add SP0 CONSTANT) where SP0 is a variable
     *  supplied as an argument to this function.  When CONSTANT is zero the expression is simplified to SP0, so that also is
     *  accepted. Although memory is byte addressable and values are stored as individual bytes in memory, this function
     *  attempts to sew related addresses back together again to produce variables that are multiple bytes.  There are many
     *  ways to do this, all of which are heuristic. */
    StackVariables findStackVariables(const BaseSemantics::SValuePtr &initialStackPointer) const;

    /** Returns the list of all known local variables.  A local variable is any stack variable whose starting address is less
     *  than the specified stack pointer.  For the definition of stack variable, see @ref findStackVariables. */
    StackVariables findLocalVariables(const BaseSemantics::SValuePtr &initialStackPointer) const;

    /** Returns the list of all known function arguments.  A function argument is any stack variable whose starting address is
     *  greater than or equal to the specified stack pointer.  For the definition of stack variable, see @ref
     *  findStackVariables.   On architectures that pass a return address on the top of the stack, that return address is
     *  considered to be the first argument of the function. */
    StackVariables findFunctionArguments(const BaseSemantics::SValuePtr &initialStackPointer) const;

    /** Returns a list of global variables.  The returned abstract locations all point to memory. The @p wordNBytes is the
     *  maximum size for any returned variable; larger units of memory written to by the same instruction will be broken into
     *  smaller variables. */
    std::vector<AbstractLocation> findGlobalVariables(size_t wordNBytes) const;
    
public:
    bool merge(const Ptr &other);                       // merge other into this, returning true iff changed
    bool mergeDefiners(BaseSemantics::SValuePtr &dstValue /*in,out*/, const BaseSemantics::SValuePtr &srcValue) const;
    bool mergeSValues(BaseSemantics::SValuePtr &dstValue /*in,out*/, const BaseSemantics::SValuePtr &srcValue) const;
    bool mergeRegisterStates(const BaseSemantics::RegisterStateGenericPtr &dstState,
                             const BaseSemantics::RegisterStateGenericPtr &srcState) const;
    bool mergeMemoryStates(const BaseSemantics::MemoryCellListPtr &dstState,
                           const BaseSemantics::MemoryCellListPtr &srcState) const;

private:
    void init();
};

std::ostream& operator<<(std::ostream&, const State &x);

/** List of states, one per dataflow vertex. */
typedef std::vector<State::Ptr> States;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Transfer function
//
// The transfer function is reponsible for taking a CFG vertex and an initial state and producing the next state, the final
// state for that vertex.  Users can use whatever transfer function they want; this one is based on the DfCfg and State types
// defined above.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Dataflow transfer functor. */
class TransferFunction {
    BaseSemantics::DispatcherPtr cpu_;
    BaseSemantics::SValuePtr callRetAdjustment_;
    const RegisterDescriptor STACK_POINTER_REG;
public:
    explicit TransferFunction(const BaseSemantics::DispatcherPtr &cpu, const RegisterDescriptor &stackPointerRegister)
        : cpu_(cpu), STACK_POINTER_REG(stackPointerRegister) {
        size_t adjustment = STACK_POINTER_REG.get_nbits() / 8; // sizeof return address on top of stack
        callRetAdjustment_ = cpu->number_(STACK_POINTER_REG.get_nbits(), adjustment);
    }

    State::Ptr initialState() const;

    // Required by dataflow engine: should return a deep copy of the state
    State::Ptr operator()(const State::Ptr &incomingState) const {
        return incomingState ? incomingState->clone() : State::Ptr();
    }

    // Required by dataflow engine: compute new output state given a vertex and input state.
    State::Ptr operator()(const DfCfg&, size_t vertexId, const State::Ptr &incomingState) const;
};

/** Dataflow engine. */
typedef rose::BinaryAnalysis::DataFlow::Engine<DfCfg, State::Ptr, TransferFunction> Engine;


} // namespace
} // namespace
} // namespace
} // namespace

#endif
