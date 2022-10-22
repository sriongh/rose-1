include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs
#include $(ROSE_WITH_SIGHT_INSTALL_PATH)/Makefile.extern

bin_PROGRAMS = fuse

fuse_CXXFLAGS = -g -O0 -std=c++14 -DDISABLE_SIGHT -DFUSE_START_FROM_MAIN -DFUSE_SKIP_LIB_GLOBAL_DECLS
fuse_LDADD = -lrose
fuse_LDFLAGS = $(ROSE_LIBS) $(LIBS_WITH_RPATH)

AM_CPPFLAGS = \
	$(ROSE_INCLUDES) \
	-I$(srcdir)/analysis \
	-I$(srcdir)/abstraction \
	-I$(srcdir)/cfgUtils \
	-I$(srcdir)/sight/widgets \
	-I$(srcdir)/sight/ \
	-I$(srcdir)/lattice \
	-I$(srcdir)/simpleAnalyses \
	-I$(srcdir)/state \
	-I$(srcdir)/sight

include_HEADERS =\
        fuse.h \
	abstraction/abstract_object.h \
	abstraction/abstract_object_map.h \
	abstraction/abstract_object_set.h \
	abstraction/concrete_values.h \
	analysis/analysis.h \
	analysis/analysis_tester.h \
	analysis/compose.h \
	analysis/tight_composer.h \
	analysis/composed_analysis.h \
	analysis/comp_shared_ptr.h \
        analysis/data_structures.h \
	analysis/factor_trans_system.h \
	analysis/graphIterator.h \
	analysis/partitions.h \
	analysis/ssa.h \
	analysis/stx_analysis.h \
	cfgUtils/CallGraphTraverse.h \
	cfgUtils/cfgUtils.h \
	cfgUtils/DataflowCFG.h \
	cfgUtils/VirtualCFGIterator.h \
	sight/sight.h \
	lattice/lattice.h \
	lattice/latticeFull.h \
	simpleAnalyses/AnalysisAstAttribute.h \
	simpleAnalyses/call_context_sensitivity_analysis.h \
	simpleAnalyses/const_prop_analysis.h \
	simpleAnalyses/const_prop_count.h \
	simpleAnalyses/dead_path_elim_analysis.h \
	simpleAnalyses/live_dead_analysis.h \
	simpleAnalyses/pointsToAnalysis.h \
	simpleAnalyses/printAnalysisStates.h \
	simpleAnalyses/saveDotAnalysis.h \
	simpleAnalyses/taintAnalysis.h \
	simpleAnalyses/virtualMethodAnalysis.h \
	simpleAnalyses/dynamicMonitor.h \
	simpleAnalyses/defsAnalysis.h \
	simpleAnalyses/VariableStateTransfer.h \
	fuseCommandParser.h

# TEST VARIABLES
TESTS_DIR = $(top_srcdir)/projects/fuse/src/tests
TEST_EXIT_STATUS = $(top_srcdir)/scripts/test_exit_status

fuse_SOURCES = \
	abstraction/abstract_object.C \
	abstraction/abstract_object_map.C \
	abstraction/abstract_object_set.C \
	abstraction/concrete_values.C \
	analysis/analysis.C \
	analysis/analysis_tester.C \
	analysis/compose.C \
	analysis/tight_composer.C \
	analysis/composed_analysis.C \
	analysis/factor_trans_system.C \
	analysis/graphIterator.C \
	analysis/partitions.C \
	analysis/ssa_nosight.C \
	analysis/ssa.C \
	analysis/stx_analysis.C \
	cfgUtils/CallGraphTraverse.C \
	cfgUtils/cfgUtils.C \
	cfgUtils/DataflowCFG.C \
	cfgUtils/VirtualCFGIterator.C \
	lattice/lattice.C \
	simpleAnalyses/AnalysisAstAttribute.C \
	simpleAnalyses/call_context_sensitivity_analysis.C \
	simpleAnalyses/const_prop_analysis.C \
	simpleAnalyses/const_prop_count.C \
	simpleAnalyses/dead_path_elim_analysis.C \
	simpleAnalyses/live_dead_analysis.C \
	simpleAnalyses/pointsToAnalysis.C \
	simpleAnalyses/printAnalysisStates.C \
	simpleAnalyses/saveDotAnalysis.C \
	simpleAnalyses/taintAnalysis.C \
	simpleAnalyses/dynamicMonitor.C \
	state/nodeState.C \
	fuseCommandParser.C \
	compositionalTestAnalysis.C

# for .C files
FUSE_C_TESTS = \
	compositionalTestInput1.C \
	compositionalTestInput2.C \
	compositionalTestInput3.C \
	compositionalTestInput4.C \
	compositionalTestInput5.C \
	compositionalTestInput6.C \
	compositionalTestInput7.C \
	compositionalTestInput8.C \
	compositionalTestInput9.C \
	compositionalTestInput10.C \
	compositionalTestInput.jisheng.C \
	cp_test1.C 

#SA (9/13/2013): moving these tests to expensive as they take long time
# if these tests complete within 5 mins we can move them to regular checks
FUSE_C_EXPENSIVE_TESTS = \
	compositionalTestInput1.C \
	compositionalTestInput2.C \
	compositionalTestInput3.C

# for .cpp files
FUSE_CPP_TESTS = \
	simplePtrTest.cpp \
	sas13Fig1Example.cpp

#SA 8/15/2014
# Adding checks for testing tight composition
# Tests are simple pointer manipulations
# Sequential composition requires multiple iterations
# Tight composition requires only single iteration
FUSETIGHT_CP_PT_TESTS=\
	simplePtrTestTight1.cpp \
	simplePtrTestTight2.cpp \
	simplePtrTestTight3.cpp \
	simplePtrTestTight4.cpp \
	simplePtrTestTight5.cpp

FUSE_C_CHECKS = $(addprefix fuse_C_, $(addsuffix .passed, $(FUSE_C_TESTS)))
FUSE_C_EXPENSIVE_CHECKS = $(addprefix fuse_C_EXP_, $(addsuffix .passed, $(FUSE_C_EXPENSIVE_TESTS)))
FUSE_CPP_CHECKS = $(addprefix fuse_cpp_, $(addsuffix .passed, $(FUSE_CPP_TESTS)))
FUSETIGHT_CHECKS = $(addprefix fusetight_, $(addsuffix .passed, $(FUSETIGHT_TESTS)))
FUSE_FLAGS = 
FUSE = ./fuse

$(FUSE_C_CHECKS): fuse_C_%.passed: $(TESTS_DIR)/% $(TEST_EXIT_STATUS) $(FUSE)
	@$(RTH_RUN) CMD="./fuse  -I$(TESTS_DIR) -c $<" $(TEST_EXIT_STATUS) $@

$(FUSE_C_EXPENSIVE_CHECKS): fuse_C_EXP_%.passed: $(TESTS_DIR)/% $(TEST_EXIT_STATUS) $(FUSE)
	@$(RTH_RUN) CMD="./fuse  -I$(TESTS_DIR) -c $<" $(TEST_EXIT_STATUS) $@

$(FUSE_CPP_CHECKS): fuse_cpp_%.passed: $(TESTS_DIR)/% $(TEST_EXIT_STATUS) $(FUSE)
	@$(RTH_RUN) CMD="./fuse  -I$(TESTS_DIR) -c $<" $(TEST_EXIT_STATUS) $@

$(FUSETIGHT_CHECKS): fusetight_%.passed: $(TESTS_DIR)/% $(TEST_EXIT_STATUS) $(FUSE)
	@$(RTH_RUN) CMD="./fuse  -I$(TESTS_DIR) -c $<" $(TEST_EXIT_STATUS) $@

.PHONY: check-fuse check-fuse-expensive

check-fuse: $(FUSE_C_CHECKS) $(FUSE_CPP_CHECKS) $(FUSETIGHT_CHECKS)
check-fuse-expensive: $(FUSE_C_EXPENSIVE_CHECKS)

.PHONY: clean-fuse
clean-fuse:
	rm -rf fuse fuse-*.o

#############
# EXTRADIST #
#############
EXTRA_DIST = \
	src/factor_trans_systemTest.C 

###############
# CXX TESTING #
###############
# Generic cxx testing for robustness

CXX_TESTDIR = $(top_srcdir)/tests/CompileTests/Cxx_tests

CXX_TESTS_REQUIRED_TO_PASS = \
        X.C lexPhase2003_01.C math.C test2001_01.C test2001_02.C test2001_05.C test2001_07.C test2001_08.C         \
        test2001_11.C test2001_12.C test2001_13.C test2001_15.C test2001_16.C test2001_17.C          \
        test2001_18.C test2001_19.C test2001_20.C test2001_21.C test2001_22.C test2001_23.C test2001_24.C \
        test2001_27.C test2002_01.C test2003_02.C test2003_03.C test2003_04.C test2003_05.C test2003_06.C          \
        test2003_08.C test2003_09.C test2003_11.C test2003_12.C test2003_13.C test2003_14.C \
        test2003_17.C test2003_20.C test2003_21.C test2003_23.C test2003_25.C test2003_26.C test2003_27.C test2003_28.C          \
        test2003_29.C test2003_30.C test2003_31.C test2003_37.C test2003_38.C test2004_01.C test2004_03.C test2004_04.C          \
        test2004_05.C test2004_06.C test2004_07.C test2004_08.C test2004_09.C test2004_100.C test2004_101.C test2004_102.C       \
        test2004_103.C test2004_104.C test2004_105.C test2004_106.C test2004_107.C test2004_108.C test2004_109.C test2004_110.C  \
        test2004_111.C test2004_112.C test2004_113.C test2004_114.C test2004_115.C test2004_117.C test2004_118.C test2004_119.C  \
        test2004_12.C test2004_121.C test2004_122.C test2004_123.C test2004_124.C test2004_125.C test2004_128.C test2004_13.C    \
        test2004_132.C test2004_133.C test2004_134.C test2004_135.C test2004_136.C test2004_137.C test2004_138.C  \
        test2004_140.C test2004_144.C test2004_146.C test2004_147.C test2004_148.C test2004_150.C test2004_151.C test2004_152.C  \
        test2004_153.C test2004_154.C test2004_156.C test2004_158.C test2004_16.C test2004_160.C test2004_163.C   \
        test2004_17.C test2004_23.C test2004_24.C test2004_26.C test2004_27.C test2004_28.C test2004_29.C                \
        test2004_31.C test2004_33.C test2004_34.C test2004_37.C test2004_38.C test2004_39.C                \
        test2004_40.C test2004_44.C test2004_49.C test2004_50.C test2004_51.C test2004_53.C test2004_54.C          \
        test2004_56.C test2004_58.C test2004_59.C test2004_61.C test2004_62.C test2004_64.C test2004_65.C                \
        test2004_70.C test2004_71.C test2004_74.C test2004_76.C test2004_78.C                \
        test2004_79.C test2004_80.C test2004_81.C test2004_83.C test2004_84.C test2004_86.C test2004_87.C test2004_89.C          \
        test2004_90.C test2004_91.C test2004_94.C test2004_98.C          \
        test2004_99.C test2005_01.C test2005_03.C test2005_05.C test2005_105.C test2005_107.C test2005_109.C     \
        test2005_11.C test2005_110.C test2005_111.C test2005_113.C test2005_116.C test2005_119.C test2005_120.C   \
        test2005_121.C test2005_123.C test2005_124.C test2005_125.C test2005_126.C test2005_127.C test2005_13.C   \
        test2005_135.C test2005_140.C test2005_144.C test2005_145.C test2005_148.C test2005_149.C test2005_15.C    \
        test2005_150.C test2005_153.C test2005_154.C test2005_157.C test2005_159.C test2005_16.C test2005_160.C test2005_162.C   \
        test2005_169.C test2005_170.C test2005_171.C test2005_172.C test2005_174.C        \
        test2005_175.C test2005_176.C test2005_179.C test2005_180.C test2005_182.C test2005_183.C test2005_184.C test2005_185.C  \
        test2005_189.C test2005_190.C test2005_191.C test2005_192.C test2005_195.C test2005_196.C test2005_197.C test2005_198.C  \
        test2005_20.C test2005_203.C test2005_206.C test2005_207.C test2005_208.C test2005_209.C test2005_21.C test2005_22.C     \
        test2005_23.C test2005_24.C test2005_25.C test2005_26.C test2005_30.C test2005_31.C test2005_32.C test2005_33.C          \
        test2005_36.C test2005_40.C test2005_41.C test2005_45.C test2005_47.C test2005_48.C test2005_49.C                \
        test2005_52.C test2005_53.C test2005_55.C test2005_58.C test2005_60.C              \
        test2005_61.C test2005_66.C test2005_67.C test2005_68.C test2005_70.C                \
        test2005_72.C test2005_74.C test2005_78.C test2005_79.C test2005_80.C test2005_81.C                \
        test2005_84.C test2005_86.C test2005_87.C test2005_88.C test2005_89.C test2005_90.C test2005_91.C test2005_92.C          \
        test2005_94.C test2005_95.C test2005_96.C test2005_97.C test2006_01.C test2006_02.C test2006_03.C          \
        test2006_06.C test2006_07.C test2006_10.C test2006_101.C test2006_103.C    \
        test2006_106.C test2006_109.C test2006_113.C test2006_114.C \
        test2006_117.C test2006_118.C test2006_12.C test2006_120.C test2006_121.C test2006_122.C test2006_123.C  \
        test2006_129.C test2006_13.C test2006_14.C test2006_140.C test2006_142.C test2006_144.C \
        test2006_146.C test2006_151.C test2006_152.C test2006_153.C test2006_155.C test2006_156.C  \
        test2006_157.C test2006_158.C test2006_16.C test2006_161.C test2006_162.C test2006_163.C test2006_164.C \
        test2006_17.C test2006_179.C test2006_18.C test2006_181.C test2006_183.C test2006_19.C     \
        test2006_22.C test2006_23.C test2006_26.C test2006_34.C test2006_38.C test2006_41.C                \
        test2006_42.C test2006_44.C test2006_46.C test2006_53.C test2006_55.C test2006_56.C test2006_57.C          \
        test2006_59.C test2006_61.C test2006_62.C test2006_63.C test2006_66.C test2006_67.C test2006_68.C test2006_69.C          \
        test2006_70.C test2006_71.C test2006_72.C test2006_73.C test2006_74.C test2006_75.C test2006_76.C test2006_77.C          \
        test2006_80.C test2006_81.C test2006_82.C test2006_83.C test2006_86.C test2006_87.C      \
        test2006_94.C test2006_95.C test2006_96.C test2006_97.C test2006_98.C test2006_99.C test2007_01.C test2007_02.C          \
        test2007_03.C test2007_04.C test2007_05.C test2007_07.C \
        test2007_12.C test2007_128.C test2007_13.C test2007_130.C test2007_133.C test2007_137.C    \
        test2007_14.C test2007_140.C test2007_142.C test2007_147.C test2007_15.C test2007_152.C    \
        test2007_154.C test2007_157.C test2007_16.C test2007_160.C test2007_161.C test2007_162.C test2007_163.C test2007_165.C   \
        test2007_169.C test2007_17.C test2007_172.C test2007_173.C test2007_21.C test2007_23.C test2007_24.C      \
        test2007_25.C test2007_29.C test2007_41.C test2007_43.C test2007_46.C test2007_47.C              \
        test2007_48.C test2007_49.C test2007_50.C test2007_51.C test2007_52.C test2007_53.C test2007_55.C test2007_57.C          \
        test2007_62.C test2007_70.C test2007_75.C test2007_77.C test2007_78.C test2007_81.C          \
        test2007_82.C test2007_83.C test2007_84.C test2007_85.C test2007_90.C test2007_98.C test2008_01.C test2008_03.C          \
        test2008_06.C test2008_09.C test2008_10.C test2008_11.C test2008_13.C          \
        test2008_14.C test2009_01.C test2009_02.C test2009_03.C test2009_04.C test2009_05.C test2009_06.C test2009_08.C          \
        test2009_09.C test2009_10.C test2009_15.C test2009_18.C \
        test2009_21.C test2009_24.C test2009_27.C test2009_28.C test2009_30.C test2009_39.C test2010_18.C                        \
        test2011_01.C test2011_02.C test2011_05.C test2011_07.C test2011_100.C test2011_111.C test2011_115.C       \
        test2011_117.C test2011_118.C test2011_119.C test2011_120.C test2011_19.C test2011_20.C test2011_21.C test2011_22.C      \
        test2011_23.C test2011_24.C test2011_25.C test2011_26.C test2011_27.C test2011_28.C test2011_29.C test2011_30.C          \
        test2011_31.C test2011_32.C test2011_33.C test2011_34.C test2011_35.C test2011_36.C test2011_37.C test2011_38.C          \
        test2011_40.C test2011_41.C test2011_42.C test2011_43.C test2011_44.C test2011_45.C test2011_46.C test2011_47.C          \
        test2011_48.C test2011_49.C test2011_50.C test2011_51.C test2011_52.C test2011_53.C test2011_54.C test2011_55.C          \
        test2011_56.C test2011_57.C test2011_58.C test2011_59.C test2011_60.C test2011_61.C test2011_62.C test2011_63.C          \
        test2011_64.C test2011_67.C test2011_68.C test2011_69.C test2011_70.C test2011_71.C test2011_73.C          \
        test2011_75.C test2011_78.C test2011_79.C test2011_80.C test2011_81.C          \
        test2011_82.C test2011_83.C test2011_84.C test2011_85.C test2011_86.C test2011_87.C test2011_88.C test2011_89.C          \
        test2011_90.C test2011_91.C test2011_92.C test2011_93.C test2011_94.C test2011_95.C test2011_96.C test2011_97.C          \
        test2011_98.C test_CplusplusMacro_Cpp.C \
        test2001_04.C test2001_14.C \
        test2003_18.C test2003_32.C test2003_35.C test2003_36.C test2004_10.C \
        test2004_139.C  \
        test2004_145.C test2004_15.C \
        test2004_161.C \
        test2004_45.C test2004_60.C test2004_63.C \
        test2004_88.C test2005_07.C \
        test2005_100.C test2005_101.C test2005_102.C \
        test2005_112.C test2005_122.C             \
        test2005_158.C test2005_164.C \
        test2005_178.C test2005_187.C test2005_194.C                \
        test2005_202.C test2005_27.C \
        test2005_38.C test2005_44.C \
        test2005_75a.C test2005_75b.C \
        test2006_102.C test2006_112.C test2006_119.C               \
        test2006_160.C    \
        test2006_189.C test2006_39.C test2006_47.C           \
        test2006_50.C test2006_58.C test2006_85.C test2006_89.C     \
        test2006_90.C test2006_92.C test2007_11.C test2007_158.C          \
        test2007_159.C \
        test2007_94.C test2008_16.C test2009_31.C \
        test2010_19.C  \
        test2011_101.C test2011_110.C test2011_39.C \
        test2011_99.C test2005_114.C

#SA: 8/19/14
#const_prop_analysis.C:2534 SgReferenceType for SgDotExp not checked
CXX_TESTS_FAILING_CP_2534 = test2001_06.C test2004_162.C test2004_164.C test2005_104.C test2006_126.C test2006_149.C \
test2006_150.C test2006_177.C test2006_178.C test2007_103.C test2007_167.C test2007_59.C test2009_19.C test2009_20.C \
test2010_03.C test2010_04.C test2010_05.C test2010_06.C

#SA: 8/19/14
#const_prop_analysis.C:965 CPConcreteKind::op(SgUnaryOp*): Assertion `0' failed
CXX_TESTS_FAILING_CP_965 = test2001_09.C test2001_10.C test2001_28.C test2003_10.C \
test2003_16.C test2004_131.C test2004_155.C test2004_30.C test2004_42.C \
test2004_47.C test2004_73.C test2004_93.C test2004_95.C test2004_96.C \
test2004_97.C test2005_06.C test2005_108.C test2005_117.C test2005_14.C \
test2005_177.C test2005_19.C test2005_43.C test2005_51.C test2006_05.C \
test2006_125.C test2006_143.C test2006_25.C test2006_28.C test2006_52.C \
test2007_06.C test2007_08.C test2007_09.C test2007_102.C test2007_10.C \
test2007_135.C test2007_150.C test2007_151.C test2009_12.C test2009_14.C \
test2010_25.C test2011_133.C test2011_65.C test2011_74.C

#SA: 8/19/14
#const_prop_analysis.C:2547 isSgVarRefExp(isSgDotExp(op)->get_rhs_operand())->get_symbol()->get_declaration();
#get_rhs_operand() is not SgVarRefExp in failing cases
CXX_TESTS_FAILING_CP_2547 = inputForLoopLocator.C test2003_01.C test2004_02.C test2004_142.C test2004_157.C \
test2004_159.C test2004_18.C test2004_36.C test2004_43.C test2004_52.C \
test2004_55.C test2004_75.C test2005_08.C test2005_09.C test2005_106.C \
test2005_10.C test2005_12.C test2005_136.C test2005_138.C test2005_139.C \
test2005_141.C test2005_143.C test2005_146.C test2005_152.C test2005_156.C \
test2005_168.C test2005_17.C test2005_193.C test2005_29.C test2005_37.C \
test2005_71.C test2005_76.C test2006_115.C test2006_154.C test2006_185.C \
test2006_54.C test2006_78.C test2007_155.C test2007_164.C test2007_170.C \
test2007_171.C test2007_40.C test2007_44.C test2009_11.C test2009_42.C

#SA: 8/19/14
#const_prop_analysis.C:3781 Assertion `pedge->target()->CFGNodes().begin()->getNode() == n' failed
CXX_TESTS_FAILING_CP_3781 = test2001_25.C test2011_72.C

#SA: 8/19/14
#const_prop_analysis.C:1195 arrType = isSgArrayType(ref->get_lhs_operand()->get_type());
# ref->get_lhs_operand() is not of SgArrayType in failing cases
CXX_TESTS_FAILING_CP_1195 = test2005_166.C test2005_199.C test2006_116.C test2006_145.C

#SA: 8/19/14
#const_prop_analysis.C:1585 unhandled case in CPConcreteKind::doBinaryOp(DoOpType&, fuse::CPConcreteKindPtr)
CXX_TESTS_FAILING_CP_1585 = test2005_62.C

#SA: 8/19/14
#const_prop_analysis.C:1205 assert(isSgValueExp(arrType->get_index()))) fails
CXX_TESTS_FAILING_CP_1205 = test2006_40.C test2008_04.C test2008_12.C

#SA: 8/19/14
#const_prop_analysis.C:894: fuse::CPValueKindPtr fuse::CPConcreteKind::doUnaryIntegralOp(DoOpType&)
#Enum val unhandled
CXX_TESTS_FAILING_CP_894 = test2007_72.C

# Currently do not support varargs (e.g. printf)
CXX_TESTS_FAILING_VARARGS = test2001_29.C test2003_15.C test2004_11.C test2004_14.C test2004_19.C test2004_32.C test2005_02.C test2005_165.C test2005_50.C test2005_57.C test2006_111.C test2006_88.C test2006_91.C test2007_26.C test2007_28.C test2008_08.C test2009_37.C test_int_lexemes_donot_pass.C test_int_lexemes.C test2006_104.C  test2006_105.C test2006_107.C test2006_108.C test_simple_int.C  test_wchars.C test2008_05.C

# Template instantiation, lack of definition
CXX_TESTS_FAILING_TEMPLATE1 = test2004_69.C test2004_141.C test2004_67.C test2004_68.C test2005_155.C test2005_59.C test2005_73.C test2006_08.C test2006_37.C test2007_38.C test2005_99.C

# Templated method never got instantiated
CXX_TESTS_FAILING_TEMPLATE2 = test2005_173.C test2005_77.C test2005_137.C test2005_167.C test2005_63.C test2005_64.C test2007_166 test2005_134.C

# Friend method declaration doesn't have associated definition
CXX_TESTS_FAILING_FUNC_DEF1 = test2006_159.C

# Method declaration outside class doesn't have associated definition
CXX_TESTS_FAILING_FUNC_DEF2 = test2006_165.C

# Creation of dummy templated method instantiations
CXX_TESTS_FAILING_FUNC_DEF3 = test2005_28.C test2011_76.C

# Odd bug with finding defining declaration of function defined outside of class.
CXX_TESTS_FAILING_FUNC_DEF4 = test2005_118.C test2006_166.C

# Call to "Y::operator new(1);" is mapped to [extern void *operator new(unsigned long ); | SgFunctionDeclaration] rather than  [public: inline static void *operator new(unsigned long size){void *value;return value;} | SgFunctionDefinition]
CXX_TESTS_FAILING_FUNC_DEF5 = test2006_04.C

# Creating empty instances of template-initialized class member functions
CXX_TESTS_FAILING_FUNC_DEF_CREATE = inputBug327.C inputBug317.C test2004_120.C

# No Definition of __builtin_vsnprintf()
CXX_TESTS_FAILING_UNDEF_VSNPRINTF = test2004_127.C test2004_129.C test2004_85.C

# No Definition of __builtin_coshl()
CXX_TESTS_FAILING_UNDEF_COSHL = test2005_46.C

# No Definition of __builtin_atanf()
CXX_TESTS_FAILING_UNDEF_ATANF = test2004_48.C
# In SgTypeTraitBuiltinFunctionCallExp::cfgInEdges(1): Almost all of the type trait builtin functions take types as arguments, but a few do not (not handled!)
CXX_TESTS_FAILING_UNHANDLED = test2004_130.C test2004_143.C test2004_149.C test2004_35.C test2004_77.C test2005_103.C test2005_128.C test2005_129.C test2005_130.C test2005_131.C test2005_132.C test2005_133.C test2005_142.C test2005_161.C test2005_181.C test2005_34.C test2005_35.C test2005_39.C test2005_42.C test2005_82.C test2005_93.C test2005_98.C test2006_124.C test2006_141.C test2006_20.C test2006_79.C test2006_84.C test2007_141.C test2009_40.C test2009_41.C test2010_24.C test2011_06.C test2011_121.C

# We're not currently handling cases where function parameters have default values
CXX_TESTS_FAILING_DEF_FUNC_PARAM =  test2005_65.C test2005_83.C

CXX_TESTS_FAILING = ${CXX_TESTS_FAILING_VARARGS} ${CXX_TESTS_FAILING_TEMPLATE1} ${CXX_TESTS_FAILING_TEMPLATE2} ${CXX_TESTS_FAILING_FUNC_DEF1} ${CXX_TESTS_FAILING_FUNC_DEF2} ${CXX_TESTS_FAILING_FUNC_DEF3} ${CXX_TESTS_FAILING_FUNC_DEF4} ${CXX_TESTS_FAILING_FUNC_DEF5} ${CXX_TESTS_FAILING_FUNC_DEF_CREATE} ${CXX_TESTS_FAILING_UNDEF_VSNPRINTF} ${CXX_TESTS_FAILING_UNDEF_COSHL} ${CXX_TESTS_FAILING_UNDEF_ATANF} ${CXX_TESTS_FAILING_UNHANDLED} ${CXX_TESTS_FAILING_DEF_FUNC_PARAM} ${CXX_TESTS_FAILING_CP_2534} ${CXX_TESTS_FAILING_CP_965} ${CXX_TESTS_FAILING_CP_2547} ${CXX_TESTS_FAILING_CP_3781} ${CXX_TESTS_FAILING_CP_1195} ${CXX_TESTS_FAILING_CP_1585} ${CXX_TESTS_FAILING_CP_1205} ${CXX_TESTS_FAILING_CP_894}

CXX_TESTS_HANGING =

# DQ (6/1/2014): Failing test.
# test2005_163.C 
# test2011_77.C 


#FUSE_CXX_TESTS_REQUIRED_TO_PASS = ${CXX_TESTS_REQUIRED_TO_PASS:.C=.C-o}

#$(FUSE_CXX_TESTS_REQUIRED_TO_PASS): fuse $(CXX_TESTDIR)/$(@:.C=.C-o)
#	./fuse -c --edg:no_warnings $(ROSE_FLAGS) $(FUSE_FLAGS) -I$(top_srcdir)/tests/CompileTests/Cxx_tests $(CXX_TESTDIR)/$(@:.C-o=.C)


# command to run fuse ./fuse -fuse:"lc(cp)"
# runs constant propagation on all cxx_ tests
CXX_FUSE_TESTS = $(addprefix cxx_, $(addsuffix .passed, $(CXX_TESTS_REQUIRED_TO_PASS)))
$(CXX_FUSE_TESTS): cxx_%.passed: $(CXX_TESTDIR)/% $(TEST_EXIT_STATUS) $(FUSE)
	@$(RTH_RUN) CMD="./fuse -fuse:\"lc(cp)\" -I$(CXX_TESTDIR) -c $<" $(TEST_EXIT_STATUS) $@

CXX_FUSE_EXPENSIVE_TESTS = $(addprefix cxx_exp_, $(addsuffix .passed, $(CXX_TESTS_EXPENSIVE)))
$(CXX_FUSE_EXPENSIVE_TESTS): cxx_exp_%.passed: $(CXX_TESTDIR)/% $(TEST_EXIT_STATUS) $(FUSE)
	@$(RTH_RUN) CMD="./fuse -fuse:\"lc(cp)\" -I$(CXX_TESTDIR) -c $<" $(TEST_EXIT_STATUS) $@

.PHONY: check-cxx check-cxx-expensive
check-cxx: $(CXX_FUSE_TESTS)
check-cxx-expensive: $(CXX_FUSE_EXPENSIVE_TESTS)

###############
# CHECK RULES #
###############

check-expensive: check-fuse-expensive check-cxx-expensive
check-local: check-fuse check-fusetight check-cxx

###############
# CLEAN RULES #
###############
clean-debug:
	rm -rf *.dot *.png dbg/

clean-check:
	rm -f *.passed *.failed *.out *.err

clean-local: clean-fuse clean-check clean-debug
