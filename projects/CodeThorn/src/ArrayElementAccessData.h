#ifndef ARRAY_ELEMENT_ACCESS_DATA
#define ARRAY_ELEMENT_ACCESS_DATA

#include <set>
#include "VariableIdMapping.h"

struct ArrayElementAccessData {
  SPRAY::VariableId varId;
  std::vector<int> subscripts;
  SPRAY::VariableId getVariable() const;
  int getSubscript(int numOfDimension) const;
  int getDimensions() const;
  ArrayElementAccessData();
  ArrayElementAccessData(SgPntrArrRefExp* ref, SPRAY::VariableIdMapping* variableIdMapping);
  std::string toString(SPRAY::VariableIdMapping* variableIdMapping) const;
  //! checks validity of data. The default value is not valid (does not correspond to any array) but can be used when creating STL containers.
  bool isValid() const;
};

bool operator==(const ArrayElementAccessData& a,const ArrayElementAccessData& other);
bool operator!=(const ArrayElementAccessData& a, const ArrayElementAccessData& other);
bool operator<(const ArrayElementAccessData& a, const ArrayElementAccessData& other);

typedef std::set<ArrayElementAccessData> ArrayElementAccessDataSet;

#endif

