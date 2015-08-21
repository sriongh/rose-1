#ifndef UDASTATTRIBUTE
#define UDASTATTRIBUTE

// Author: Markus Schordan, 2013.

#include "Labeler.h"
#include "VariableIdMapping.h"

#include <set>
#include <string>

#include "RDAstAttribute.h"
#include "AnalysisAbstractionLayer.h"

namespace SPRAY {
typedef std::pair<VariableId,SPRAY::LabelSet> VariableIdLabelSetPair;
typedef std::set<VariableIdLabelSetPair> UseDefInfo;

/*! 
  * \author Markus Schordan
  * \date 2013.
 */
class UDAstAttribute : public AstSgNodeAttribute {
 public:
  UDAstAttribute(RDAstAttribute* rdAttr, SgNode* nodeForUseVarQuery);
  VariableIdSet useVariables(VariableIdMapping& vidm);
  SPRAY::LabelSet definitionsOfVariable(VariableId var);
  void toStream(std::ostream& os, VariableIdMapping* vim);
 private:
  RDAstAttribute* _rdAttr;
  SgNode* _node;
};

} // end of namespace SPRAY

#endif
