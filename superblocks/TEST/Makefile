### Change this to reflect the c code file

#EXE = Perf2/perf2

#EXE = 583simple/src/simple

#EXE = cse/cse

EXE = prog2/prog2

#EXE = 583wc/src/wc
#INPUT = 583wc/input1/cccp.c

#EXE = 583compress/src/compress
#INPUT = 583compress/input1/compress.in

### Change these two lines to reflect the LLVM Object Files and our library path
LLVM_OBJ_ROOT = /y/students/zajush/llvm2/llvm-install/
PROJ_OBJ_ROOT = /y/students/zajush/llvm2/superblock/eecs583-final-project-joshlzh-flintmu-zajush

### Which type of build are we using
# CLANG_STYLE = Debug+Profile+Asserts
CLANG_STYLE = Debug+Asserts
# CLANG_STYLE = Debug
# CLANG_STYLE = Release

PASS_OBJ = $(PROJ_OBJ_ROOT)/$(CLANG_STYLE)/lib/superblockpass.so

INIT_OPTIS = -loop-simplify

#ccp_prog
OPTIS = -early-cse -constprop

#cse
#OPTIS = -gvn

all: $(EXE).base.out $(EXE).sp.out
	diff $(EXE).base.out $(EXE).sp.out

# make cfg to open the cfg in gui
cfg-orig: $(EXE).base.bc
	opt -dot-cfg $(EXE).base.bc >& /dev/null
	xdot cfg.main.dot

cfg-sp: $(EXE).sp.bc
	opt -dot-cfg $(EXE).sp.bc >& /dev/null
	xdot cfg.main.dot

# generate bitcode (with -loop-simplify flag)
$(EXE).bc: $(EXE).c
	llvm-gcc -emit-llvm -c $< -o $(EXE).bc.tmp
	opt $(INIT_OPTIS) $(EXE).bc.tmp -o $@

# baseline
$(EXE).base.bc: $(EXE).bc
	opt $(OPTIS) < $< > $@
	llvm-dis $@

$(EXE).base.s: $(EXE).base.bc
	llc < $< > $@

$(EXE).base.exe: $(EXE).base.s
	g++ $< -o $@

$(EXE).base.out: $(EXE).base.exe
	- time ./$(EXE).base.exe $(INPUT) > $@

# insert edge profiling
$(EXE).profile.bc: $(EXE).bc
	opt -insert-edge-profiling $< -o $@

$(EXE).profile.s: $(EXE).profile.bc
	llc $< -o $@

$(EXE).profile.exe: $(EXE).profile.s
	g++ $(LLVM_OBJ_ROOT)/Release/lib/profile_rt.so $< -o $@

llvmprof.out: $(EXE).profile.exe
	- ./$(EXE).profile.exe $(INPUT) >& /dev/null

# superblock formation
$(EXE).sp.bc: $(EXE).bc llvmprof.out
	opt -load $(PASS_OBJ) \
	-profile-loader -profile-info-file=llvmprof.out \
	-superblock -time-passes \
	$(OPTIS) \
	< $(EXE).bc > $@
	llvm-dis $@

$(EXE).sp.s: $(EXE).sp.bc
	llc < $< > $@

$(EXE).sp.exe: $(EXE).sp.s
	g++ $< -o $@

$(EXE).sp.out: $(EXE).sp.exe
	- time ./$(EXE).sp.exe $(INPUT) > $@

.PHONY: all clean

clean:
	rm -f llvmprof.out $(EXE).bc* $(EXE).base.* $(EXE).profile.* $(EXE).sp.* \
	cfg.*.dot gmon.out result.*
