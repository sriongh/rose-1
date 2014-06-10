#include "heapSSA.h"
#include "nodeState.h"
#include "analysis.h"
#include "uniqueNameTraversal.h"
#include "defsAndUsesTraversal.h"
#include "iteratedDominanceFrontier.h"
#include "controlDependence.h"
#include "heapDefsAndUsesTraversal.h"

/**  
 * This is a intra-procedural heap SSA constructor 
 *           
 * author: jisheng zhao (jz10@rice.edu)   
 */
 
#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

using namespace std;
using namespace boost;
using namespace ssa_private;
using namespace hssa_private;

HeapReachingDefPtr HeapSSA::emptyHeapReachingDefPtr;
SSAMemLocPtr HeapSSA::emptySSAMemLocPtr;
      
void HeapSSA::build(SgProject* proj, bool interprocedural, bool treatPointersAsStructures) {
  if (!alreadyBuild)
    alreadyBuild = true;
  else
    // No need to rebuild
    return;

  std::cout << "ssa build " << proj << std::endl;
  project = proj;
  // Construct Heap SSA
  build(interprocedural, treatPointersAsStructures);
}

void HeapSSA::build(bool interprocedural, bool treatPointersAsStructures) {
  // Clear the internal tables
  clearTables();
  
  // Perform the unique name traversal
  uniqueNameTraversal(treatPointersAsStructures);

  // Collect the functions that need to be processed
  unordered_set<SgFunctionDefinition*> interestingFunctions;
  vector<SgFunctionDefinition*> funcs 
    = SageInterface::querySubTree<SgFunctionDefinition > (project, V_SgFunctionDefinition);
  FunctionFilter functionFilter;
  foreach (SgFunctionDefinition* f, funcs) {
    if (functionFilter(f->get_declaration()))
      interestingFunctions.insert(f);
  }
  
  DefsAndUsesTraversal defUseTrav(this, treatPointersAsStructures);
  // Identify the heap use/def operations
  HeapDefsAndUsesTraversal heapDefUseTrav(this, treatPointersAsStructures);
  //Generate all local information before doing interprocedural analysis. This is so we know      
  //what variables are directly modified in each function body before we do interprocedural propagation
  foreach(SgFunctionDefinition* func, interestingFunctions) {
    defUseTrav.traverse(func->get_declaration());
    
    hv_setCurrentFunction(func);
    heapDefUseTrav.traverse(func->get_declaration());

    // Collect Heap operations and set the correspond abstract memory objects
    hv_buildAMOs(func->get_declaration());
    
    //Expand any member variable definition to also define its parents at the same node         
    // expandParentMemberDefinitions(func->get_declaration());
    expandParentMemberDefs(func->get_declaration());
   
    //Expand any member variable uses to also use the parent variables (e.g. a.x also uses a)   
    expandParentMemberUses(func->get_declaration());
    
    insertDefsForChildMemberUses(func->get_declaration());
  }
  
  //Interprocedural iterations. We iterate on the call graph until all interprocedural defs are propagated 
  if (interprocedural)
    interproceduralDefPropagation(interestingFunctions);

  //Now we have all local information, including interprocedural defs. Propagate the defs along control-flow 
  foreach(SgFunctionDefinition* func, interestingFunctions) {
    vector<FilteredCfgNode> functionCfgNodesPostorder = getCfgNodesInPostorder(func);
   
    hv_setCurrentFunction(func);
    //Insert definitions at the SgFunctionDefinition for external variables whose values flow inside the function
    insertDefsForExternalVariables(func->get_declaration());
    // Insert the dummy def for the heap variable
    hv_getDummyDefForHeapVar(originalDefTable);

    //Create all ReachingDef objects:                                                           
    //Create ReachingDef objects for all original definitions                                   
    populateLocalDefsTable(func->get_declaration());
    
    //Insert phi functions at join points                                                       
    multimap< FilteredCfgNode, pair<FilteredCfgNode, FilteredCfgEdge> > controlDependencies =
      insertPhiFunctions(func, functionCfgNodesPostorder);
 
    // Renumber all instantiated ReachingDef objects  
    renumberAllDefinitions(func, functionCfgNodesPostorder);

    runDefUseDataFlow(func);
    
    //We have all the propagated defs, now update the use table
    buildUseTable(functionCfgNodesPostorder);
    
    //Annotate phi functions with dependencies 
    //annotatePhiNodeWithConditions(func, controlDependencies);
  }
}

void HeapSSA::clearTables() {
  originalDefTable.clear();
  expandedDefTable.clear();
  reachingDefsTable.clear();
  localUsesTable.clear();
  useTable.clear();
  ssaLocalDefTable.clear();
}
 
void HeapSSA::uniqueNameTraversal(bool treatPointersAsStructures) {
  #ifdef DISPLAY_TIMINGS
  timer time;
#endif
  // if (getDebug())
    cout << "Running UniqueNameTraversal...\n";
    const std::vector<SgInitializedName*>& allNames 
      = SageInterface::querySubTree<SgInitializedName > (project, V_SgInitializedName);
    cout << " unique name 1\n"; 
  UniqueNameTraversal uniqueTrav(allNames, treatPointersAsStructures);
  uniqueTrav.traverse(project);
  // if (getDebug())
    cout << "Finished UniqueNameTraversal." << endl;
#ifdef DISPLAY_TIMINGS
  printf("-- Timing: UniqueNameTraversal took %.2f seconds.\n", time.elapsed());
  fflush(stdout);
  time.restart();
#endif

}

void HeapSSA::expandParentMemberDefs(SgFunctionDeclaration* function)
{
  class ExpandDefsTraversal : public AstSimpleProcessing
  {
  public:
    HeapSSA* hssa;

    void visit(SgNode* node)
    {
      if (hssa->originalDefTable.count(node) == 0)
	return;

      //We want to iterate the vars defined on this node, and expand them
      foreach(const VarName& definedVar, hssa->originalDefTable[node]) {

	if (getDebugExtra())
	  {
	    cout << "Checking [" << varnameToString(definedVar) << "]" << endl;
	  }
	
	//Check if the variableName has multiple parts
	if (definedVar.size() == 1)
	  {
	    continue;
	  }
	
	//We are dealing with a multi-part variable, loop the entry and expand it           
	//Start at one so we don't get the same defs in the original and expanded defs      
	for (unsigned int i = 1; i < definedVar.size(); i++) {
	  //Create a new varName vector that goes from beginning to end - i               
	  VarName newName;
	  newName.assign(definedVar.begin(), definedVar.end() - i);
	  
	  if (getDebugExtra())
	    {
	      cout << "Testing for presence of [" << varnameToString(newName) << "]" << endl;
	    }
	  
	  //Only insert the new definition if it does not already exist in the original def table  
	  if (hssa->originalDefTable[node].count(newName) == 0)
	    {
	      //Insert the new name as being defined here.                                
	      hssa->expandedDefTable[node].insert(newName);
	      
	      if (getDebugExtra())
		{
		  cout << "Inserted new name [" << varnameToString(newName) << "] into defs." << endl;
		}
	    }
	}
      }
    }
  };

  ExpandDefsTraversal trav;
  trav.hssa = this;
  trav.traverse(function, preorder);
}

void HeapSSA::expandParentMemberUses(SgFunctionDeclaration* function)
{

  class ExpandUsesTraversal : public AstSimpleProcessing
  {
  public:
    HeapSSA* hssa;

    void visit(SgNode* node)
    {
      if (hssa->localUsesTable.count(node) == 0)
	return;

      //We want to iterate the vars used on this node, and expand them 
      foreach(const VarName& usedVar, hssa->localUsesTable[node]) {
	  if (getDebugExtra()) {
	      cout << "Checking [" << varnameToString(usedVar) << "]" << endl;
	    }

	  //Check if the variableName has multiple parts 
	  if (usedVar.size() == 1) {
	    continue;
	  }

	  //We are dealing with a multi-part variable, loop the entry and expand it           
	  //Start at one so we don't reinsert same use                                        
	  for (unsigned int i = 1; i < usedVar.size(); i++)
	    {
	      //Create a new varName vector that goes from beginning to end - i  
	      VarName newName;
	      newName.assign(usedVar.begin(), usedVar.end() - i);

	      if (getDebugExtra())
		{
		  cout << "Testing for presence of [" << varnameToString(newName) << "]" << endl;
		}

	      //Only insert the new definition if it does not already exist                   
	      if (hssa->localUsesTable[node].count(newName) == 0)
		{
		  //Insert the new name as being used here.                                   
		  hssa->localUsesTable[node].insert(newName);

		  if (getDebugExtra())
		    {
		      cout << "Inserted new name [" << varnameToString(newName) << "] into uses." << endl;
		    }
		}
	    }
	}
    }
  };

  ExpandUsesTraversal trav;
  trav.hssa = this;
  trav.traverse(function, preorder);
}

void HeapSSA::insertDefsForChildMemberUses(SgFunctionDeclaration* function)
{
  ROSE_ASSERT(function->get_definition() != NULL);

  set<VarName> usedNames = getVarsUsedInSubtree(function);

  //Map each varName to all used names for which it is a prefix      
  map<VarName, set<VarName> > nameToChildNames;

  foreach(const VarName& rootName, usedNames)
    {

      foreach(const VarName& childName, usedNames)
        {
	  if (childName.size() <= rootName.size())
	    continue;

	  if (isPrefixOfName(childName, rootName))
            {
	      nameToChildNames[rootName].insert(childName);
            }
        }
    }

  //Now that we have all the used names, we iterate the definitions.          
  //If there is a definition and a child of it is used, we have to insert a definition for the child also
  class InsertExpandedDefsTraversal : public AstSimpleProcessing
  {
  public:
    HeapSSA* hssa;
    map<VarName, set<VarName> >* nameToChildNames;
    
    void visit(SgNode* node)
    {
      LocalDefUseTable::const_iterator childDefs = hssa->originalDefTable.find(node);
      
      if (childDefs == hssa->originalDefTable.end())
	return;
      
      foreach(const VarName& definedVar, childDefs->second)
	{
	map<VarName, set<VarName> >::iterator childVars = nameToChildNames->find(definedVar);
	if (childVars == nameToChildNames->end())
	  continue;
	
	//Go over all the child names and define them here also  
	foreach(const VarName& childName, childVars->second)
	  {
	    ROSE_ASSERT(childName.size() > definedVar.size());
	    for (size_t i = 0; i < (childName.size() - definedVar.size()); i++)
	      {
		//Create a new varName vector that goes from beginning to end - i   
		VarName newName;
		newName.assign(childName.begin(), childName.end() - i);
		
		if (hssa->expandedDefTable[node].count(newName) == 0 
		    && hssa->originalDefTable[node].count(newName) == 0)
		  {
		    hssa->expandedDefTable[node].insert(newName);
		  }
	      }
	  }
	}
    }
  };
  
 InsertExpandedDefsTraversal trav;
 trav.hssa = this;
 trav.nameToChildNames = &nameToChildNames;
 trav.traverse(function, preorder);
}

void HeapSSA::insertDefsForExternalVariables(SgFunctionDeclaration* function)
{
  ROSE_ASSERT(function->get_definition() != NULL);

  set<VarName> usedNames = getVarsUsedInSubtree(function);

  set<VarName>& originalVarsAtFunctionEntry = originalDefTable[function->get_definition()];
  set<VarName>& expandedVarsAtFunctionEntry = expandedDefTable[function->get_definition()];

  //Iterate over each used variable and check it it is declared outside of the function scope 
  foreach(const VarName& usedVar, usedNames)
    {
      VarName rootName;
      rootName.assign(1, usedVar[0]);

      SgScopeStatement* varScope = SageInterface::getScope(rootName[0]);
      SgScopeStatement* functionScope = function->get_definition();

      //If it is a local variable, there should be a def somewhere inside the function    
      if (varScope == functionScope || SageInterface::isAncestor(functionScope, varScope))
        {
	  //We still need to insert defs for compiler-generated variables (e.g. __func__), since they don't have defs in the AST           
	  if (!isBuiltinVar(rootName))
	    continue;
        }
      else if (isSgGlobal(varScope))
        {
	  //Handle the case of declaring "extern int x" inside the function                   
	  //Then, x has global scope but it actually has a definition inside the function so we don't need to insert one                   
	  if (SageInterface::isAncestor(function->get_definition(), rootName[0]))
            {
	      //When else could a var be declared inside a function and be global?     
	      SgVariableDeclaration* varDecl = isSgVariableDeclaration(rootName[0]->get_parent());
	      ROSE_ASSERT(varDecl != NULL);
	      ROSE_ASSERT(varDecl->get_declarationModifier().get_storageModifier().isExtern());
	      continue;
            }
        }

      //Are there any other types of external vars?   
      ROSE_ASSERT(isBuiltinVar(rootName) || isSgClassDefinition(varScope) || isSgNamespaceDefinitionStatement(varScope)
		  || isSgGlobal(varScope));

      //The variable is not in local scope; we need to insert a def for it at the function definition                                      
      for (size_t i = 0; i < usedVar.size(); i++)
        {
	  //Create a new varName vector that goes from beginning to end - i      
	  VarName newName;
	  newName.assign(usedVar.begin(), usedVar.end() - i);
	  originalVarsAtFunctionEntry.insert(newName);
	  
	  ROSE_ASSERT(expandedVarsAtFunctionEntry.count(newName) == 0);
        }
    }
}

void HeapSSA::populateLocalDefsTable(SgFunctionDeclaration* function)
{
  ROSE_ASSERT(function->get_definition() != NULL);

  struct InsertDefs : public AstSimpleProcessing
  {
    HeapSSA* hssa;

    void visit(SgNode * node)
    {
      // Short circuit to prevent creating empty entries in the local def table when we don't need them    
      if ((hssa->originalDefTable.count(node) == 0 || hssa->originalDefTable[node].empty()) &&
	  (hssa->expandedDefTable.count(node) == 0 || hssa->expandedDefTable[node].empty()))
	{
	  return;
	}

      //This is the table of local definitions at the current node     
      NodeReachingDefTable& localDefs = hssa->ssaLocalDefTable[node];

      if (hssa->originalDefTable.count(node) > 0)
	{

	  foreach(const VarName& definedVar, hssa->originalDefTable[node])
	    {
	      localDefs[definedVar] = ReachingDefPtr(new ReachingDef(node, ReachingDef::ORIGINAL_DEF));
	    }
	}

      if (hssa->expandedDefTable.count(node) > 0)
	{

	  foreach(const VarName& definedVar, hssa->expandedDefTable[node])
	    {
	      localDefs[definedVar] = ReachingDefPtr(new ReachingDef(node, ReachingDef::EXPANDED_DEF));
	    }
	}
    }
  };

  InsertDefs trav;
  trav.hssa = this;
  trav.traverse(function, preorder);
}

multimap< StaticSingleAssignment::FilteredCfgNode, pair<StaticSingleAssignment::FilteredCfgNode, StaticSingleAssignment::FilteredCfgEdge> > HeapSSA::insertPhiFunctions(SgFunctionDefinition* function, const std::vector<FilteredCfgNode>& cfgNodesInPostOrder)
{
  if (getDebug())
    printf("Inserting phi nodes in function %s...\n", function->get_declaration()->get_name().str());
  ROSE_ASSERT(function != NULL);

  //First, find all the places where each name is defined        
  map<VarName, vector<FilteredCfgNode> > nameToDefNodesMap;

  foreach(const FilteredCfgNode& cfgNode, cfgNodesInPostOrder)
    {
      SgNode* node = cfgNode.getNode();

      //Don't visit the sgFunctionDefinition node twice        
      if (isSgFunctionDefinition(node) && cfgNode != FilteredCfgNode(node->cfgForBeginning()))
	continue;

      //Check the definitions at this node and add them to the map        
      LocalDefUseTable::const_iterator defEntry = originalDefTable.find(node);
      if (defEntry != originalDefTable.end())
        {

	  foreach(const VarName& definedVar, defEntry->second)
            {
	      nameToDefNodesMap[definedVar].push_back(cfgNode);
            }
        }

      defEntry = expandedDefTable.find(node);
      if (defEntry != expandedDefTable.end())
        {

	  foreach(const VarName& definedVar, defEntry->second)
            {
	      nameToDefNodesMap[definedVar].push_back(cfgNode);
            }
        }
    }

  //Build an iterated dominance frontier for this function    
  map<FilteredCfgNode, FilteredCfgNode> iPostDominatorMap;
  map<FilteredCfgNode, set<FilteredCfgNode> > domFrontiers =
    calculateDominanceFrontiers<FilteredCfgNode, FilteredCfgEdge > (function, NULL, &iPostDominatorMap);

  //Calculate control dependencies (for annotating the phi functions)     
  multimap< FilteredCfgNode, pair<FilteredCfgNode, FilteredCfgEdge> > controlDependencies =
    calculateControlDependence<FilteredCfgNode, FilteredCfgEdge > (function, iPostDominatorMap);

  //Find the phi function locations for each variable      
  VarName var;
  vector<FilteredCfgNode> definitionPoints;

  foreach(tie(var, definitionPoints), nameToDefNodesMap)
    {
      ROSE_ASSERT(!definitionPoints.empty() && "We have a variable that is not defined anywhere!");

      //Calculate the iterated dominance frontier  
      set<FilteredCfgNode> phiNodes = calculateIteratedDominanceFrontier(domFrontiers, definitionPoints);

      if (getDebug())
	printf("Variable %s has phi nodes inserted at\n", varnameToString(var).c_str());

      foreach(FilteredCfgNode phiNode, phiNodes)
        {
	  SgNode* node = phiNode.getNode();
	  ROSE_ASSERT(reachingDefsTable[node].first.count(var) == 0);

	  //We don't want to insert phi defs for functions that have gone out of scope  
	  if (!isVarInScope(var, node))
	    continue;

	  reachingDefsTable[node].first[var] = ReachingDefPtr(new ReachingDef(node, ReachingDef::PHI_FUNCTION));

	  if (getDebug())
	    printf("\t\t%s\n", phiNode.toStringForDebugging().c_str());
        }
    }

  return controlDependencies;
}

void HeapSSA::renumberAllDefinitions(SgFunctionDefinition* func, const vector<FilteredCfgNode>& cfgNodesInPostOrder)
{
  //Map from each name to the next index. Not in map means 0                 
  map<VarName, int> nameToNextIndexMap;

  //The SgFunctionDefinition node is special. reachingDefs INTO the function definition node are actually
  //The definitions that reach the *end* of the function              
  //reachingDefs OUT of the function definition node are the ones that come externally into the function       
  FilteredCfgNode functionStartNode = FilteredCfgNode(func->cfgForBeginning());
  FilteredCfgNode functionEndNode = FilteredCfgNode(func->cfgForEnd());

  //We process nodes in reverse postorder; this provides a natural numbering for definitions 
  reverse_foreach(const FilteredCfgNode& cfgNode, cfgNodesInPostOrder) {
    SgNode* astNode = cfgNode.getNode();
    
    //Iterate over all the phi functions inserted at this node. We skip the SgFunctionDefinition entry node,   
    //since those phi functions actually belong to the bottom of the CFG    
    if (cfgNode != functionStartNode) {
      
      foreach(NodeReachingDefTable::value_type& varDefPair, reachingDefsTable[astNode].first) {
	const VarName& definedVar = varDefPair.first;
	ReachingDefPtr reachingDef = varDefPair.second;
	
	if (!reachingDef->isPhiFunction())
	    continue;
	
	//Give an index to the variable        
	int index = 0;
	if (nameToNextIndexMap.count(definedVar) > 0) {
	  index = nameToNextIndexMap[definedVar];
	}
	nameToNextIndexMap[definedVar] = index + 1;
	  
	reachingDef->setRenamingNumber(index);
      }
    }
    
    //Local defs at the function end actually occur at the very beginning of the function  
    if (cfgNode != functionEndNode) {
      //Iterate over all the local definitions at the node
      foreach(NodeReachingDefTable::value_type& varDefPair, ssaLocalDefTable[astNode]) {
	const VarName& definedVar = varDefPair.first;
	ReachingDefPtr reachingDef = varDefPair.second;
	
	//Give an index to the variable                                                
	int index = 0;
	if (nameToNextIndexMap.count(definedVar) > 0) {
	  index = nameToNextIndexMap[definedVar];
	}
	nameToNextIndexMap[definedVar] = index + 1;
	
	reachingDef->setRenamingNumber(index);
      
	// Hack the dphi reaching def and renumbering them
	if (hv_isHeapVar(definedVar) && hv_hasDPhi(astNode)) {
	  // Renumbering the dphi function here
	  ReachingDefPtr dphiReachingDef = hv_getDPhi(astNode);
	  //Give an index to the variable  
	  int dphiInd = 0;
	  if (nameToNextIndexMap.count(definedVar) > 0) {
	    dphiInd = nameToNextIndexMap[definedVar];
	  }
	  nameToNextIndexMap[definedVar] = dphiInd + 1;

	  dphiReachingDef->setRenamingNumber(dphiInd);
	}
      }
    }
  }
}

void HeapSSA::runDefUseDataFlow(SgFunctionDefinition* func)
{
  if (getDebug())
    printOriginalDefTable();
  //Keep track of visited nodes                                                                  
  unordered_set<SgNode*> visited;
  
  set<FilteredCfgNode> worklist;
  
  FilteredCfgNode current = FilteredCfgNode(func->cfgForBeginning());
  worklist.insert(current);
  
  while (!worklist.empty())
    {
      if (getDebugExtra())
	cout << "-------------------------------------------------------------------------" << endl;
      //Get the node to work on                                                                  
      current = *worklist.begin();
      worklist.erase(worklist.begin());
      
      //Propagate defs to the current node                                                       
      bool changed = propagateDefs(current);

      //For every edge, add it to the worklist if it is not seen or something has changed        
      
      reverse_foreach(const FilteredCfgEdge& edge, current.outEdges())
        {
	  FilteredCfgNode nextNode = edge.target();
	  
	  //Insert the child in the worklist if the parent is changed or it hasn't been visited yet                                                                                                
	  if (changed || visited.count(nextNode.getNode()) == 0)
	    {
	      //Add the node to the worklist                                                     
	      bool insertedNew = worklist.insert(nextNode).second;
	      if (insertedNew && getDebugExtra())
		{
		  if (changed)
		    cout << "Defs Changed: Added " << nextNode.getNode()->class_name() << nextNode.getNode() << " to the worklist." << endl;
		  else
		    cout << "Next unvisited: Added " << nextNode.getNode()->class_name() << nextNode.getNode() << " to the worklist." << endl;
		}
	    }
	}
      
      //Mark the current node as seen                                                            
      visited.insert(current.getNode());
    }
}

bool HeapSSA::propagateDefs(FilteredCfgNode cfgNode)
{
  SgNode* node = cfgNode.getNode();

  //This updates the IN table with the reaching defs from previous nodes                         
  updateIncomingPropagatedDefs(cfgNode);

  //Special Case: the OUT table at the function definition node actually denotes definitions at the function entry                                                                                 
  //So, if we're propagating to the *end* of the function, we shouldn't update the OUT table     
  if (isSgFunctionDefinition(node) && cfgNode == FilteredCfgNode(node->cfgForEnd()))
    {
      return false;
    }
  
  //Create a staging OUT table. At the end, we will check if this table                          
  //Was the same as the currently available one, to decide if any changes have occurred          
  //We initialize the OUT table to the IN table                                                  
  NodeReachingDefTable outDefsTable = reachingDefsTable[node].first;
  
  //Special case: the IN table of the function definition node actually denotes                  
  //definitions reaching the *end* of the function. So, start with an empty table to prevent def initions                                                                                           
  //from the bottom of the function from propagating to the top.                                 
  if (isSgFunctionDefinition(node) && cfgNode == FilteredCfgNode(node->cfgForBeginning()))
    {
      outDefsTable.clear();
    }
  
  //Now overwrite any local definitions:                                                         
  if (ssaLocalDefTable.count(node) > 0)
    {
      
      foreach(const NodeReachingDefTable::value_type& varDefPair, ssaLocalDefTable[node])
	{
	  const VarName& definedVar = varDefPair.first;
	  ReachingDefPtr localDef = varDefPair.second;
	  
	  outDefsTable[definedVar] = localDef;
	}
    }
  
  //Compare old to new OUT tables                                                                
  bool changed = (reachingDefsTable[node].second != outDefsTable);
  if (changed)
    {
      reachingDefsTable[node].second = outDefsTable;
    }
  
  return changed;
}

void HeapSSA::updateIncomingPropagatedDefs(FilteredCfgNode cfgNode)
{
  //Get the previous edges in the CFG for this node   
  vector<FilteredCfgEdge> inEdges = cfgNode.inEdges();
  SgNode* astNode = cfgNode.getNode();

  NodeReachingDefTable& incomingDefTable = reachingDefsTable[astNode].first;

  //Iterate all of the incoming edges   
  for (unsigned int i = 0; i < inEdges.size(); i++) {
    SgNode* prev = inEdges[i].source().getNode();
    
    const NodeReachingDefTable& previousDefs = reachingDefsTable[prev].second;
    
    //Merge all the previous defs into the IN table of the current node 
    foreach(const NodeReachingDefTable::value_type& varDefPair, previousDefs) {
      const VarName& var = varDefPair.first;
      const ReachingDefPtr previousDef = varDefPair.second;
      
      //Here we don't propagate defs for variables that went out of scope    
      //(built-in vars are body-scoped but we inserted the def at the SgFunctionDefinition node, so we make an exception)              
      if (!isVarInScope(var, astNode) && !isBuiltinVar(var))
	continue;
      
      //If this is the first time this def has propagated to this node, just copy it over   
      if (incomingDefTable.count(var) == 0) {
	incomingDefTable[var] = previousDef;
 
	// Process the heap variable and dphi function  
	if (hv_isHeapVar(var)) {
	  SgInitializedName* prevSgn = isSgInitializedName(previousDef->getDefinitionNode());
	  if (prevSgn != NULL) {
	    SgType* nodeType = prevSgn->get_type();
	  }
	  // Get the prev node's dphi 
          ReachingDefPtr prevDPhi = previousDef;
	  if (!prevDPhi->isPhiFunction() 
	      && hv_hasDPhi(previousDef->getDefinitionNode())) {
	    prevDPhi = hv_getDPhi(previousDef->getDefinitionNode());
	  }
          if (hv_hasHeapUse(astNode)) {
            // Set prev node's dphi as all current node's heap uses' reaching def    
	    hv_updateUseReachingDef(astNode, prevDPhi);
	  }
          if (hv_hasDPhi(astNode)) {
            HeapReachingDefPtr currDPhi = hv_getDPhi(astNode);
            currDPhi->addJoinedDPhi(currDPhi, prevDPhi);
	  } 
        }
      } else {
	ReachingDefPtr existingDef = incomingDefTable[var];
	
	if (existingDef->isPhiFunction() && existingDef->getDefinitionNode() == astNode) {
	  //There is a phi node here. We update the phi function to point to the previous reaching definition 
	  existingDef->addJoinedDef(previousDef, inEdges[i]);
	} else {
	  //If there is no phi node, and we get a new definition, it better be the same as the one previously propagated.                                   
	  if (!(*previousDef == *existingDef)) {
	    printf("ERROR: At node %s@%d, two different definitions reach for variable %s\n", astNode->class_name().c_str(), astNode->get_file_info()->get_line(), varnameToString(var).c_str());
	    ROSE_ASSERT(false);
	  }
	} 

	// Process the heap variable and dphi function
	if (hv_isHeapVar(var)) {
	  SgNode* currNode = astNode;
	  if (SgExprStatement* exprStmt = isSgExprStatement(astNode)) {
	    SgExpression * expr = exprStmt->get_expression();
	    currNode = expr;
	  }
	  // Get the prev node's dphi
	  ReachingDefPtr prevDPhi = existingDef;
	  if (!prevDPhi->isPhiFunction() 
	      && hv_hasDPhi(previousDef->getDefinitionNode())) {
	    prevDPhi = hv_getDPhi(previousDef->getDefinitionNode());
	  } 
	  if (hv_hasHeapUse(currNode)) {
	    // Set prev node's dphi as all current node's heap uses' reaching def
	    hv_updateUseReachingDef(currNode, prevDPhi);
	  }
	  if (hv_hasDPhi(currNode) && existingDef->isPhiFunction()) {
	    HeapReachingDefPtr currDPhi = hv_getDPhi(currNode);
	    currDPhi->addJoinedDPhi(currDPhi, prevDPhi);
	  }
	}
      }
    }
  }
}

void HeapSSA::buildUseTable(const vector<FilteredCfgNode>& cfgNodes) {
  foreach(const FilteredCfgNode& cfgNode, cfgNodes) {
    SgNode* node = cfgNode.getNode();
    if (localUsesTable.count(node) == 0)
      continue;
    
    foreach(const VarName& usedVar, localUsesTable[node]) {
      //Check the defs that are active at the current node to find the reaching definition 
      //We want to check if there is a definition entry for this use at the current node  
      if (reachingDefsTable[node].first.count(usedVar) > 0) {
	useTable[node][usedVar] = reachingDefsTable[node].first[usedVar];
      } else {
	// There are no defs for this use at this node, this shouldn't happen  
	printf("Error: Found use for the name '%s', but no reaching defs!\n", varnameToString(usedVar).c_str());
	printf("Node is %s:%d in %s\n", node->class_name().c_str(), node->get_file_info()->get_line(),
	       node->get_file_info()->get_filename());
	ROSE_ASSERT(false);
      }
    }
  }
}

/**
 * Check if the two given abstract memory objects must be same or not, now we support two types 
 * of object set: ScalarExprObj and ScalarNamedObj
 */
bool HeapSSA::mustBeSame(SSAMemLocPtr memLoc1, SSAMemLocPtr memLoc2) {
  return memLoc1->mustEqual(memLoc2);
}

bool HeapSSA::mustBeSame(SSAMemLoc* memLoc1, SSAMemLoc* memLoc2) {
  // This function should expire
  /*ScalarExprObj* seObj1 = dynamic_cast<ScalarExprObj* >(objSet1);
  ScalarExprObj* seObj2 = dynamic_cast<ScalarExprObj* >(objSet2);
  
  if (seObj1 != NULL && seObj2 != NULL)
    return mustBeSame(seObj1, seObj2);
  else if (seObj1 != NULL || seObj2 != NULL)
    return false;
  
  ScalarNamedObj* snObj1 = dynamic_cast<ScalarNamedObj* >(objSet1);
  ScalarNamedObj* snObj2 = dynamic_cast<ScalarNamedObj* >(objSet2);
  if (snObj1 != NULL && snObj2 != NULL) {
    return mustBeSame(snObj1, snObj2);
  }
  */
  ROSE_ASSERT(false);

  return false;
}

bool HeapSSA::mustBeSame(SSAMemLoc* memLoc1, SSAMemLoc* memLoc2, bool& mayBeSame) {
  // This function should expire
  /*ScalarExprObj* seObj1 = dynamic_cast<ScalarExprObj* >(objSet1);
  ScalarExprObj* seObj2 = dynamic_cast<ScalarExprObj* >(objSet2);

  if (seObj1 != NULL && seObj2 != NULL)
    return mustBeSame(seObj1, seObj2, mayBeSame);
  else if (seObj1 != NULL || seObj2 != NULL)
    return false;

  ScalarNamedObj* snObj1 = dynamic_cast<ScalarNamedObj* >(objSet1);
  ScalarNamedObj* snObj2 = dynamic_cast<ScalarNamedObj* >(objSet2);
  if (snObj1 != NULL && snObj2 != NULL) {
    return mustBeSame(snObj1, snObj2);
  }
  */
  ROSE_ASSERT(false);

  return false;
}

/**  
 * Check if the two given abstract memory objects may be same or not, now we support two types
 * of object set: ScalarExprObj and ScalarNamedOb
 */
bool HeapSSA::mayBeSame(SSAMemLocPtr memLoc1, SSAMemLocPtr memLoc2) {
  return mustBeSame(memLoc1, memLoc2);
}

bool HeapSSA::mayBeSame(SSAMemLoc* memLoc1, SSAMemLoc* memLoc2) {
  return mustBeSame(memLoc1, memLoc2);
}

/**           
 * Check if the two given abstract memory objects may be different or not, now we support 
 * two types of object set: ScalarExprObj and ScalarNamedObj
 */
bool HeapSSA::mayBeDifferent(SSAMemLocPtr memLoc1, SSAMemLocPtr memLoc2) {
  return !mustBeSame(memLoc1, memLoc2);
}

bool HeapSSA::mayBeDifferent(SSAMemLoc* memLoc1, SSAMemLoc* memLoc2) {
  return !mustBeSame(memLoc1, memLoc2);
}

/**
 * Check if two dot expressions point to same heap address
 */
bool HeapSSA::mustBeSame(SgDotExp* dotExp1, SgDotExp* dotExp2) {
  SgNode* lhs1 = dotExp1->get_lhs_operand();
  SgNode* lhs2 = dotExp2->get_lhs_operand();
  if (hasSameReachingDef(lhs1, lhs2)) {
    SgNode* rhs1 = dotExp1->get_rhs_operand();
    SgNode* rhs2 = dotExp2->get_rhs_operand();

    SgVarRefExp* varRef1 = isSgVarRefExp(rhs1);
    SgVarRefExp* varRef2 = isSgVarRefExp(rhs2);
    if (varRef1 != NULL && varRef2 != NULL)
      return varRef1->get_symbol() == varRef2->get_symbol();
    
    bool res = hasSameReachingDef(rhs1, rhs2);
    return res;
  }
  
  return false;
}

/**
 * Check if two arrow expression point to same heap address 
 */
bool HeapSSA::mustBeSame(SgArrowExp* arrowExp1, SgArrowExp* arrowExp2) {
  SgNode* lhs1 = arrowExp1->get_lhs_operand();
  SgNode* lhs2 = arrowExp2->get_lhs_operand();
  if (hasSameReachingDef(lhs1, lhs2)) {
    SgNode* rhs1 = arrowExp1->get_rhs_operand();
    SgNode* rhs2 = arrowExp2->get_rhs_operand();
    return hasSameReachingDef(rhs1, rhs2);
  }

  return false;
}

/**
 * Check if two pointer de-reference have same pointer value
 */
bool HeapSSA::mustBeSame(SgPointerDerefExp* pdrExp1, SgPointerDerefExp* pdrExp2) {
  return hasSameReachingDef(pdrExp1->get_operand(), pdrExp2->get_operand());
}

bool HeapSSA::mustBeSame(SgDotExp* dotExp, SgPointerDerefExp* pdrExp) {
  return false;
}

/**
 * Check if the two given SgNodes have same reaching def object, i.e. same variable
 */
bool HeapSSA::hasSameReachingDef(SgNode* sgn1, SgNode* sgn2) {
  const StaticSingleAssignment::VarName& varName1 = StaticSingleAssignment::getVarName(sgn1);
  const StaticSingleAssignment::VarName& varName2 = StaticSingleAssignment::getVarName(sgn2);
  const StaticSingleAssignment::NodeReachingDefTable& reachingDefs1
    = getReachingDefsAtNode_(sgn1);
  const StaticSingleAssignment::NodeReachingDefTable& reachingDefs2
    = getReachingDefsAtNode_(sgn2);
  
  // Get SSA look-aside info                                                              
  if (reachingDefs1.find(varName1) != reachingDefs1.end()
      && reachingDefs2.find(varName2) != reachingDefs2.end()) {
    StaticSingleAssignment::ReachingDefPtr reachingDef1 = (* reachingDefs1.find(varName1)).second;
    StaticSingleAssignment::ReachingDefPtr reachingDef2 = (* reachingDefs1.find(varName2)).second;
    return reachingDef1.get() == reachingDef2.get();
  } else
    return false;
}

void HeapSSA::hv_buildAMOs(SgFunctionDeclaration* function) {
  class EvaluateExpressionTraversal : public AstSimpleProcessing {
  public:
    HeapSSA* ssa;

    void visit(SgNode* sgn) {
      if (SgPntrArrRefExp* ptrArr = isSgPntrArrRefExp(sgn)) {
        ssa->hv_buildAMO(ptrArr);
      } else if (SgPointerDerefExp* ptrDeref = isSgPointerDerefExp(sgn)) {
        ssa->hv_buildAMO(ptrDeref);
      } else if (SgDotExp* dotExpr = isSgDotExp(sgn)) {
        ssa->hv_buildAMO(dotExpr);
      } else if (SgArrowExp* arrowExpr = isSgArrowExp(sgn)) {
        ssa->hv_buildAMO(arrowExpr);
      } else if (SgVarRefExp* varRefExp = isSgVarRefExp(sgn)) {
        ssa->hv_buildAMO(varRefExp);
      }
    }
  };

  EvaluateExpressionTraversal trav;
  trav.ssa = this;
  trav.traverse(function, postorder);
}

void HeapSSA::hv_buildAMO(SgNode* sgn) {
  hv_amoTable[sgn] = SSAMemLocPtr(SSAMemLocFactory::createSSAMemLoc(sgn, this));
}

void HeapSSA::hv_getDummyDefForHeapVar(StaticSingleAssignment::LocalDefUseTable& originalDefTable) {
  if (hv_varNames.find(hv_currFunc) != hv_varNames.end()) {
    const StaticSingleAssignment::VarName& heapVar = hv_getCurrentHeapVar();
    set<StaticSingleAssignment::VarName>& originalVarsAtFunctionEntry = originalDefTable[hv_currFunc];
    if (originalVarsAtFunctionEntry.count(heapVar) == 0) {
      originalVarsAtFunctionEntry.insert(heapVar);
    }
  }
}

// Check if the given varName is corresponding to current function
bool HeapSSA::hv_isHeapVar(const StaticSingleAssignment::VarName& varName) {
  if (varName.size() == 1 && hv_varNames.find(hv_currFunc) != hv_varNames.end())
    // Compare if the two var name vector has same SgInitializedName pointer
    return hv_varNames[hv_currFunc]->getKey()[0] == varName[0];
  else
    return false;
}

bool HeapSSA::hv_hasDPhi(SgNode* sgn) {
  return hv_dphiTable.find(sgn) != hv_dphiTable.end();
}

HeapReachingDefPtr HeapSSA::hv_getDPhi(SgNode* sgn) {
  return hv_dphiTable[sgn];
}

bool HeapSSA::hv_hasHeapUse(SgNode* sgn) {
  return hv_localUses.find(sgn) != hv_localUses.end();
}

// Set given node's all use SgNode with the input reaching def which is a dphi
bool HeapSSA::hv_updateUseReachingDef(SgNode* sgn, ReachingDef::ReachingDefPtr reachingDef) {
  ROSE_ASSERT(hv_localUses.find(sgn) != hv_localUses.end());
  foreach (SgNode* useNode, hv_localUses[sgn]) {
    hv_reachingDefTable[useNode] = reachingDef;
  }
}

// Get the heap variable name from a given SgExpression
const StaticSingleAssignment::VarName& HeapSSA::hv_getHeapVarName(SgExpression* sgn) {
  // We only create single heap variable here, so this is not related to the input SgNode
  if (hv_varNames.find(hv_currFunc) == hv_varNames.end()) {
    std::string hv_ = "_hv_";
    SgName name(hv_.c_str());
    SgType* type = sgn->get_type();
    SgInitializedName* initializedName = new SgInitializedName(name, type);
    initializedName->set_scope(hv_currFunc);

    // Create a new unique var name object which take the psudo SgInitializedName object
    VarUniqueName* varName = new VarUniqueName(initializedName);

    hv_varNames[hv_currFunc] = varName;
  }

  return hv_varNames[hv_currFunc]->getKey();
}

bool HeapSSA::hv_addHeapUse(SgNode* useNode, SgNode* heapUse) {
  hv_localUses[useNode].insert(heapUse);
  return true;
}

const StaticSingleAssignment::VarName& HeapSSA::hv_getCurrentHeapVar() {
  ROSE_ASSERT(hv_varNames.find(hv_currFunc) != hv_varNames.end());
  return hv_varNames[hv_currFunc]->getKey();
}

bool HeapSSA::hv_addHeapDef(SgNode* defNode, SgNode* heapDef) {
  hv_localDefs[defNode].insert(heapDef);
  return true;
}

// Create a dphi function for the given SgNode
bool HeapSSA::hv_addDPhi(SgNode* sgn) {
  ROSE_ASSERT(hv_dphiTable.find(sgn) == hv_dphiTable.end());
  hv_dphiTable[sgn]  = HeapReachingDefPtr(new HeapReachingDef(sgn, HeapReachingDef::DEF_PHI));
  hv_reachingDefTable[sgn] = hv_dphiTable[sgn];
  return true;
}

/*bool HeapSSA::hv_hasHeapLattice(ReachingDef::ReachingDefPtr reachingDef) {
  return hv_phiHeapLatticeMap.find(reachingDef) != hv_phiHeapLatticeMap.end();
}*/

/*HeapLatticePtr HeapSSA::hv_getHeapLattice(ReachingDef::ReachingDefPtr reachingDef) {
  ROSE_ASSERT(hv_phiHeapLatticeMap.find(reachingDef) != hv_phiHeapLatticeMap.end());
  return hv_phiHeapLatticeMap[reachingDef];
  }*/

/*void HeapSSA::hv_addHeapLattice(ReachingDef::ReachingDefPtr reachingDef) {
  ROSE_ASSERT(hv_phiHeapLatticeMap.find(reachingDef) == hv_phiHeapLatticeMap.end());
  hv_phiHeapLatticeMap[reachingDef] = HeapLatticePtr(new HeapSSALattice());
}*/

// Check if the give object set (i.e. AMO) has corresponding lattice in the heap SSA lattice set
/*SSAMemLocPtr HeapSSA::hv_hasHeapLattice(SSAMemLocPtr memLocPtr, HeapLatticePtr heapLatticePtr) {
  map<SSAMemLocPtr, LatticeArith* >& amoLatticeMap = heapLatticePtr->getAMOLatticeMap();

  for (map<SSAMemLocPtr, LatticeArith* >::iterator it = amoLatticeMap.begin();
       it != amoLatticeMap.end(); it ++) {
    // Using must/may Equal 
    // if (mustBeSame((* it).first.get(), memLocPtr.get()))
    if ((* it).first->mustEqual(memLocPtr))
      return (* it).first;
  }

  return emptySSAMemLocPtr;
}*/

bool HeapSSA::hv_hasHeapReachingDef(SgNode* sgn) {
  return hv_reachingDefTable.find(sgn) != hv_reachingDefTable.end();
}

// Join the two given heap SSA lattices, lattice2 --> lattice1
/*void HeapSSA::hv_joinHeapSSALattice(HeapLatticePtr heapLattice1,
				    HeapLatticePtr heapLattice2) {
  map<SSAMemLocPtr, LatticeArith* >& amoLatticeMap1 = heapLattice1->getAMOLatticeMap();
  map<SSAMemLocPtr, LatticeArith* >& amoLatticeMap2 = heapLattice2->getAMOLatticeMap();

  for (map<SSAMemLocPtr, LatticeArith* >::iterator it2 = amoLatticeMap2.begin();
       it2 != amoLatticeMap2.end(); it2 ++) {
    SSAMemLocPtr memLocPtr2 = (* it2).first;
    bool found = false;
    for (map<SSAMemLocPtr, LatticeArith* >::iterator it1 = amoLatticeMap1.begin();
         it1 != amoLatticeMap1.end(); it1 ++) {
      SSAMemLocPtr memLocPtr1 = (* it1).first;
      // Using must/may Equal
      // if (mustBeSame(memLocPtr2.get(), memLocPtr1.get())) {
      if (memLocPtr2->mustEqual(memLocPtr1)) {
	// Meet update the two same AMOs' lattice and put to the 1st heap SSA lattice
        heapLattice1->meetUpdate(memLocPtr1, memLocPtr1, (* it2).second);
	found = true;
        break;
      }
    }
    if (!found)
      // Add new AMO's lattice to the 1st heap SSA lattices
      heapLattice1->meetUpdate(memLocPtr2, emptySSAMemLocPtr, (* it2).second);
  }
  }*/

ReachingDef::ReachingDefPtr HeapSSA::hv_getHeapReachingDef(SgNode* sgn) {
  ROSE_ASSERT(hv_reachingDefTable.find(sgn) != hv_reachingDefTable.end());
  return hv_reachingDefTable[sgn];
}

bool HeapSSA::hv_hasAMO(SgNode* sgn) {        
  return hv_amoTable.find(sgn) != hv_amoTable.end();       
} 

SSAMemLocPtr HeapSSA::hv_getAMO(SgNode* sgn) { 
  return hv_amoTable[sgn];   
}
                                                                      
// Set the AMO for a given SgNode  
bool HeapSSA::hv_setAMO(SgNode* sgn, SSAMemLocPtr memLocPtr) { 
  if (hv_amoTable.find(sgn) != hv_amoTable.end())  
    return false;         
  else {         
    hv_amoTable[sgn] = memLocPtr;   
    return true;     
  }       
}  

/*bool HeapSSA::hv_hasAlias(SgNode* sgn) {
  return hv_aliasTable.find(sgn) != hv_aliasTable.end();
}

AliasSetPtr HeapSSA::hv_getAlias(SgNode* sgn) {
  return hv_aliasTable[sgn];
  }

void HeapSSA::hv_addAlias(SgNode* sgn, AliasSetPtr another) {
  AliasSet* aliasSet = another->copy();
  hv_aliasTable[sgn] = AliasSetPtr(aliasSet);
  }*/
