
g++ -Wall -fno-rtti -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS \
    -D__STDC_LIMIT_MACROS \
    -I=/usr/local/include/ \
    -L /usr/local/lib/ $1 \
    -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization \
    -lclangCodeGen -lclangParse -lclangSema -lclangStaticAnalyzerFrontend \
    -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangAnalysis \
    -lclangARCMigrate -lclangEdit \
    -lclangAST -lclangLex -lclangBasic -lLLVMMC -lLLVMSupport -lpthread
