#ifndef PROGRAM_STATS_H
#define PROGRAM_STATS_H

#include <iostream>
#include "VariableIdMapping.h"
#include "Labeler.h"
#include "CFAnalyzer.h"
#include "RDLattice.h"
#include "DFAnalyzer.h"
#include "RDAstAttribute.h"
#include "Miscellaneous.h"

enum CompInfoType {CIT_TOTAL=1, CIT_INT, CIT_FLOAT, CIT_NUM};
class ComputationInfo {
public:
  ComputationInfo():
    numJmp(0),
    numConditionTest(0),
    numLogOp(0),
    numBitOp(0),
    hasInt(false),
    hasFloat(false),
    hasRead(false),
    hasWrite(false)
 {
    for(int i=CIT_TOTAL; i<CIT_NUM;++i) {
      numArithOp[i]=0;
      numReadMemLoc[i]=0;
      numWriteMemLoc[i]=0;
    }
  }
  int numJmp;
  int numConditionTest;
  int numLogOp;
  int numBitOp;
  int numArithOp[CIT_NUM];
  int numReadMemLoc[CIT_NUM];
  int numWriteMemLoc[CIT_NUM];
  bool hasInt;
  bool hasFloat;
  bool hasRead;
  bool hasWrite;
  string toString() {
    stringstream ss;
    ss<<"Jmp:"<<numJmp<<","
      <<"Tst:"<<numConditionTest<<","
      <<"LogOp:"<<numLogOp<<","
      <<"BitOp:"<<numBitOp;
    ss<< " [";
    for(int i=CIT_TOTAL; i<CIT_NUM;++i) {
      string tt;
      switch(i){
      case CIT_TOTAL: tt="total";break;
      case CIT_INT: tt="int";break;
      case CIT_FLOAT: tt="float";break;
      }
      ss<<tt<<"-ArithOp:"<<numArithOp[i]<<",";
      ss<<tt<<"-Read:"<<numReadMemLoc[i]<<",";
      ss<<tt<<"-Write:"<<numWriteMemLoc[i]<<",";
    }
    ss<< "]";
    return ss.str();
  }
  static string coloredHTMLEntry(int cnt,string defaultColor, string color) {
    stringstream ss;
    // only generate numbers != 0 for readability (-> defaultcolor block does never contain a number)
    if(cnt>0)
      ss<<cnt;
    if(cnt==0)
      ss<<"-";
    if(cnt<0)
      ss<<"*";
    string cntstr=ss.str();
    return string("<TD BGCOLOR=")+"\""+string(cnt==0?defaultColor:color)+"\""+">"+cntstr+"</TD>";
  }
 string toDot() {
    stringstream ss;
    string defaultcolor="grey";
    ss<<coloredHTMLEntry((numJmp+numConditionTest)?1:0,defaultcolor,"yellow");
    //    ss<<coloredHTMLEntry(numLogOp,defaultcolor,"darkorchid1");
    //ss<<coloredHTMLEntry(numBitOp,defaultcolor,"darkorchid2");
    int intOpNum=numArithOp[CIT_INT]+numReadMemLoc[CIT_INT]+numWriteMemLoc[CIT_INT];
    if(intOpNum==0 && hasInt==true)
      intOpNum=-1;
    int floatOpNum=numArithOp[CIT_FLOAT]+numReadMemLoc[CIT_FLOAT]+numWriteMemLoc[CIT_FLOAT];
    if(floatOpNum==0 && hasFloat==true)
      floatOpNum=-1;
    int readOpNum=numReadMemLoc[CIT_TOTAL]+numArithOp[CIT_INT]+numArithOp[CIT_FLOAT];
    if(readOpNum==0 && hasRead==true)
      readOpNum=-1;
    int writeOpNum=numWriteMemLoc[CIT_TOTAL];
    if(writeOpNum==0 && hasWrite==true)
      writeOpNum=-1;

    ss<<coloredHTMLEntry(intOpNum,defaultcolor,"darkorchid1");
    ss<<coloredHTMLEntry(floatOpNum,defaultcolor,"dodgerblue1");
    ss<<coloredHTMLEntry(readOpNum,defaultcolor,"chartreuse4");
    ss<<coloredHTMLEntry(writeOpNum,defaultcolor,"crimson");
    return ss.str();
  }
};

class ProgramStatistics {
 public:
  ProgramStatistics(VariableIdMapping* vidm, Labeler* labeler, Flow* icfg, string useDefAstAttributeName);
  void computeStatistics();
  void generateResourceUsageICFGDotFile(string dotfile);
  void printStatistics();
  string generateNodeResourceUsageDotString(Label lab);
  void setGenerateWithSource(bool withsource);
 private:
  void computeOpStats(ComputationInfo& ci, SgNode* node);
  ComputationInfo computeComputationInfo(Label lab, VariableIdMapping* vidm);
  void floatIntStats(SgNode* node, ComputationInfo& cit);
  VariableIdMapping* vidm;
  Labeler* labeler;
  Flow* icfg;
  string useDefAstAttributeName;
  vector<ComputationInfo> computationInfo;
  bool _withSource;
};

#endif
