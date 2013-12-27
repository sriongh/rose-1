#ifndef CODETHORN_H
#define CODETHORN_H

/*************************************************************
 * Copyright: (C) 2012 by Markus Schordan                    *
 * Author   : Markus Schordan                                *
 * License  : see file LICENSE in the CodeThorn distribution *
 *************************************************************/

#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <sstream>

#include "CFAnalyzer.h"
#include "Analyzer.h"
#include "Visualizer.h"
#include "LanguageRestrictor.h"
#include "LanguageRestrictorCollection.h"
#include "Miscellaneous.h"

using namespace std;

namespace CodeThorn {

class CodeThornLanguageRestrictor : public LanguageRestrictorCppSubset1 {
public:
    void initialize();
};

} // end of namespace CodeThorn

#endif
