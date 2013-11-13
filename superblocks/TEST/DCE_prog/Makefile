### Change this to reflect the c code file
EXE = DCE_prog

### Change these two lines to reflect the LLVM Object Files and our library path
LLVM_OBJ_ROOT = /y/students/zajush/llvm2/llvm-install
PROJ_OBJ_ROOT = /y/students/zajush/llvm2/superblock/eecs583-final-project-joshlzh-flintmu-zajush

### Which type of build are we using
# CLANG_STYLE = Debug+Asserts
# CLANG_STYLE = Debug
CLANG_STYLE = Release

PASS_OBJ = $(PROJ_OBJ_ROOT)/$(CLANG_STYLE)/lib/superblockpass.so

all: llvmprof.out $(EXE).ls.out $(EXE).sp.out
	diff $(EXE).ls.out $(EXE).sp.out

# make cfg to open the cfg in gui
cfg: $(EXE).ls.out
	opt -dot-cfg $(EXE).ls.bc >& /dev/null
	xdot cfg.main.dot
cfg2: $(EXE).sp.out
	opt -dot-cfg $(EXE).sp.bc >& /dev/null
	xdot cfg.main.dot

# generate bitcode
$(EXE).bc: $(EXE).c
	llvm-gcc -emit-llvm -c $< -o $@

# loop simplify
$(EXE).ls.bc: $(EXE).bc
	opt -loop-simplify < $< > $@

$(EXE).ls.s: $(EXE).ls.bc
	llc $< -o $@

$(EXE).ls.exe: $(EXE).ls.s
	g++ $< -o $@

$(EXE).ls.out: $(EXE).ls.exe
	- ./$(EXE).ls.exe $(INPUT) > $@

# insert edge profiling
$(EXE).profile.bc: $(EXE).ls.bc
	opt -insert-edge-profiling $< -o $@

$(EXE).profile.s: $(EXE).profile.bc
	llc $< -o $@

$(EXE).profile.exe: $(EXE).profile.s
	g++ $(LLVM_OBJ_ROOT)/$(CLANG_STYLE)/lib/profile_rt.so $< -o $@

llvmprof.out: $(EXE).profile.exe
	- ./$(EXE).profile.exe $(INPUT) >& /dev/null

# superblock formation
$(EXE).sp.bc: $(EXE).ls.bc llvmprof.out
	opt -load $(PASS_OBJ) \
	-profile-loader -profile-info-file=llvmprof.out \
	-superblock \
	< $(EXE).ls.bc > $@
	llvm-dis $@

$(EXE).sp.s: $(EXE).sp.bc
	llc < $< > $@

$(EXE).sp.exe: $(EXE).sp.s
	g++ -o $@ $<

$(EXE).sp.out: $(EXE).sp.exe
	- ./$(EXE).sp.exe $(INPUT) > $@

.PHONY: all clean

clean:
	rm -f llvmprof.out $(EXE).bc $(EXE).ls.bc \
	$(EXE).profile.out $(EXE).profile.exe $(EXE).profile.bc $(EXE).profile.s \
	$(EXE).sp.out $(EXE).sp.exe $(EXE).sp.bc $(EXE).sp.s \
	cfg.main.dot \
	$(EXE).ls.ll $(EXE).ls.s $(EXE).ls.exe $(EXE).ls.out gmon.out result.*
