#ifndef LATTICE_H
#define LATTICE_H

#include "sage3basic.h"
#include <iostream>
#include <string>
#include "VariableIdMapping.h"

namespace SPRAY {

class Lattice {
 public:
  Lattice();
  virtual bool isBot();
  virtual void toStream(std::ostream& os, SPRAY::VariableIdMapping* vim);
  virtual std::string toString(SPRAY::VariableIdMapping* vim=0);
  virtual bool approximatedBy(Lattice& other)=0;
  virtual void combine(Lattice& other);
  virtual ~Lattice();
};

}

#endif
