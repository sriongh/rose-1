// Author: Markus Schordan, 2013.

#ifndef CPASTATTRIBUTEINTERFACE_H
#define CPASTATTRIBUTEINTERFACE_H

#include "Labeler.h"
#include "VariableIdMapping.h"
#include "DFAstAttribute.h"

namespace SPRAY {

/*! 
  * \author Markus Schordan
  * \date 2013.
 */
class LVAstAttributeInterface : public DFAstAttribute {
 public:
  virtual bool isLive(VariableId varId)=0;
  virtual ~LVAstAttributeInterface();
};

}

#endif
