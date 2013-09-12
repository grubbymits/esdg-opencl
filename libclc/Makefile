all::

ifndef VERBOSE
  Verb = @
endif

utils/prepare-builtins.o: ./utils/prepare-builtins.cpp
	@echo CXX utils/prepare-builtins.o
	$(Verb) c++ -MMD -MF utils/prepare-builtins.o.d -I/opt/esdg-opencl/include  -fPIC -fvisibility-inlines-hidden -g  -D_GNU_SOURCE -Wall -W -Wno-unused-parameter -Wwrite-strings -pedantic -Wno-long-long -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fno-exceptions -fno-rtti -c -o utils/prepare-builtins.o ./utils/prepare-builtins.cpp
-include utils/prepare-builtins.o.d

utils/prepare-builtins: utils/prepare-builtins.o
	@echo LINK utils/prepare-builtins
	$(Verb) c++ -o utils/prepare-builtins utils/prepare-builtins.o -L/opt/esdg-opencl/lib -lpthread -lLLVMBitWriter -lLLVMBitReader -lLLVMCore -lLLVMSupport -ldl

le1-llvm-none/lib/synchronization/barrier.cl.bc: ./le1/lib/synchronization/barrier.cl
	@echo LLVM-CC le1-llvm-none/lib/synchronization/barrier.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/synchronization/barrier.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/synchronization/barrier.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/synchronization/barrier.cl.bc ./le1/lib/synchronization/barrier.cl
-include le1-llvm-none/lib/synchronization/barrier.cl.bc.d

le1-llvm-none/lib/workitem/get_global_size.cl.bc: ./le1/lib/workitem/get_global_size.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_global_size.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_global_size.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_global_size.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_global_size.cl.bc ./le1/lib/workitem/get_global_size.cl
-include le1-llvm-none/lib/workitem/get_global_size.cl.bc.d

le1-llvm-none/lib/workitem/get_group_id.cl.bc: ./le1/lib/workitem/get_group_id.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_group_id.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_group_id.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_group_id.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_group_id.cl.bc ./le1/lib/workitem/get_group_id.cl
-include le1-llvm-none/lib/workitem/get_group_id.cl.bc.d

le1-llvm-none/lib/workitem/get_local_id.cl.bc: ./le1/lib/workitem/get_local_id.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_local_id.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_local_id.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_local_id.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_local_id.cl.bc ./le1/lib/workitem/get_local_id.cl
-include le1-llvm-none/lib/workitem/get_local_id.cl.bc.d

le1-llvm-none/lib/workitem/get_global_id.cl.bc: ./le1/lib/workitem/get_global_id.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_global_id.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_global_id.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_global_id.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_global_id.cl.bc ./le1/lib/workitem/get_global_id.cl
-include le1-llvm-none/lib/workitem/get_global_id.cl.bc.d

le1-llvm-none/lib/workitem/get_local_size.cl.bc: ./le1/lib/workitem/get_local_size.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_local_size.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_local_size.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_local_size.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_local_size.cl.bc ./le1/lib/workitem/get_local_size.cl
-include le1-llvm-none/lib/workitem/get_local_size.cl.bc.d

le1-llvm-none/lib/workitem/get_num_groups.cl.bc: ./le1/lib/workitem/get_num_groups.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_num_groups.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_num_groups.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_num_groups.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_num_groups.cl.bc ./le1/lib/workitem/get_num_groups.cl
-include le1-llvm-none/lib/workitem/get_num_groups.cl.bc.d

le1-llvm-none/lib/workitem/get_global_offset.cl.bc: ./le1/lib/workitem/get_global_offset.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_global_offset.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_global_offset.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_global_offset.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_global_offset.cl.bc ./le1/lib/workitem/get_global_offset.cl
-include le1-llvm-none/lib/workitem/get_global_offset.cl.bc.d

le1-llvm-none/lib/geometric/cross.cl.bc: ./generic/lib/geometric/cross.cl
	@echo LLVM-CC le1-llvm-none/lib/geometric/cross.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/geometric/cross.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/geometric/cross.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/geometric/cross.cl.bc ./generic/lib/geometric/cross.cl
-include le1-llvm-none/lib/geometric/cross.cl.bc.d

le1-llvm-none/lib/geometric/dot.cl.bc: ./generic/lib/geometric/dot.cl
	@echo LLVM-CC le1-llvm-none/lib/geometric/dot.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/geometric/dot.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/geometric/dot.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/geometric/dot.cl.bc ./generic/lib/geometric/dot.cl
-include le1-llvm-none/lib/geometric/dot.cl.bc.d

le1-llvm-none/lib/geometric/length.cl.bc: ./generic/lib/geometric/length.cl
	@echo LLVM-CC le1-llvm-none/lib/geometric/length.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/geometric/length.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/geometric/length.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/geometric/length.cl.bc ./generic/lib/geometric/length.cl
-include le1-llvm-none/lib/geometric/length.cl.bc.d

le1-llvm-none/lib/geometric/normalize.cl.bc: ./generic/lib/geometric/normalize.cl
	@echo LLVM-CC le1-llvm-none/lib/geometric/normalize.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/geometric/normalize.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/geometric/normalize.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/geometric/normalize.cl.bc ./generic/lib/geometric/normalize.cl
-include le1-llvm-none/lib/geometric/normalize.cl.bc.d

le1-llvm-none/lib/integer/abs.cl.bc: ./generic/lib/integer/abs.cl
	@echo LLVM-CC le1-llvm-none/lib/integer/abs.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/integer/abs.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/integer/abs.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/integer/abs.cl.bc ./generic/lib/integer/abs.cl
-include le1-llvm-none/lib/integer/abs.cl.bc.d

le1-llvm-none/lib/integer/add_sat.cl.bc: ./generic/lib/integer/add_sat.cl
	@echo LLVM-CC le1-llvm-none/lib/integer/add_sat.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/integer/add_sat.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/integer/add_sat.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/integer/add_sat.cl.bc ./generic/lib/integer/add_sat.cl
-include le1-llvm-none/lib/integer/add_sat.cl.bc.d

le1-llvm-none/lib/integer/add_sat.ll.bc: ./generic/lib/integer/add_sat.ll
	@echo LLVM-AS le1-llvm-none/lib/integer/add_sat.ll.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-as -o le1-llvm-none/lib/integer/add_sat.ll.bc ./generic/lib/integer/add_sat.ll

le1-llvm-none/lib/integer/add_sat_impl.ll.bc: ./generic/lib/integer/add_sat_impl.ll
	@echo LLVM-AS le1-llvm-none/lib/integer/add_sat_impl.ll.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-as -o le1-llvm-none/lib/integer/add_sat_impl.ll.bc ./generic/lib/integer/add_sat_impl.ll

le1-llvm-none/lib/integer/sub_sat.cl.bc: ./generic/lib/integer/sub_sat.cl
	@echo LLVM-CC le1-llvm-none/lib/integer/sub_sat.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/integer/sub_sat.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/integer/sub_sat.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/integer/sub_sat.cl.bc ./generic/lib/integer/sub_sat.cl
-include le1-llvm-none/lib/integer/sub_sat.cl.bc.d

le1-llvm-none/lib/integer/sub_sat.ll.bc: ./generic/lib/integer/sub_sat.ll
	@echo LLVM-AS le1-llvm-none/lib/integer/sub_sat.ll.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-as -o le1-llvm-none/lib/integer/sub_sat.ll.bc ./generic/lib/integer/sub_sat.ll

le1-llvm-none/lib/integer/sub_sat_impl.ll.bc: ./generic/lib/integer/sub_sat_impl.ll
	@echo LLVM-AS le1-llvm-none/lib/integer/sub_sat_impl.ll.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-as -o le1-llvm-none/lib/integer/sub_sat_impl.ll.bc ./generic/lib/integer/sub_sat_impl.ll

le1-llvm-none/lib/math/hypot.cl.bc: ./generic/lib/math/hypot.cl
	@echo LLVM-CC le1-llvm-none/lib/math/hypot.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/math/hypot.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/math/hypot.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/math/hypot.cl.bc ./generic/lib/math/hypot.cl
-include le1-llvm-none/lib/math/hypot.cl.bc.d

le1-llvm-none/lib/math/mad.cl.bc: ./generic/lib/math/mad.cl
	@echo LLVM-CC le1-llvm-none/lib/math/mad.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/math/mad.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/math/mad.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/math/mad.cl.bc ./generic/lib/math/mad.cl
-include le1-llvm-none/lib/math/mad.cl.bc.d

le1-llvm-none/lib/relational/any.cl.bc: ./generic/lib/relational/any.cl
	@echo LLVM-CC le1-llvm-none/lib/relational/any.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/relational/any.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/relational/any.cl` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/relational/any.cl.bc ./generic/lib/relational/any.cl
-include le1-llvm-none/lib/relational/any.cl.bc.d

le1-llvm-none/lib/softfloat/softfloat.c.bc: ./generic/lib/softfloat/softfloat.c
	@echo LLVM-CC le1-llvm-none/lib/softfloat/softfloat.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/softfloat/softfloat.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/softfloat/softfloat.c` -I./le1/include -I./generic/include -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/softfloat/softfloat.c.bc ./generic/lib/softfloat/softfloat.c
-include le1-llvm-none/lib/softfloat/softfloat.c.bc.d

le1-llvm-none/lib/builtins.link.bc: le1-llvm-none/lib/synchronization/barrier.cl.bc le1-llvm-none/lib/workitem/get_global_size.cl.bc le1-llvm-none/lib/workitem/get_group_id.cl.bc le1-llvm-none/lib/workitem/get_local_id.cl.bc le1-llvm-none/lib/workitem/get_global_id.cl.bc le1-llvm-none/lib/workitem/get_local_size.cl.bc le1-llvm-none/lib/workitem/get_num_groups.cl.bc le1-llvm-none/lib/workitem/get_global_offset.cl.bc le1-llvm-none/lib/geometric/cross.cl.bc le1-llvm-none/lib/geometric/dot.cl.bc le1-llvm-none/lib/geometric/length.cl.bc le1-llvm-none/lib/geometric/normalize.cl.bc le1-llvm-none/lib/integer/abs.cl.bc le1-llvm-none/lib/integer/add_sat.cl.bc le1-llvm-none/lib/integer/add_sat.ll.bc le1-llvm-none/lib/integer/add_sat_impl.ll.bc le1-llvm-none/lib/integer/sub_sat.cl.bc le1-llvm-none/lib/integer/sub_sat.ll.bc le1-llvm-none/lib/integer/sub_sat_impl.ll.bc le1-llvm-none/lib/math/hypot.cl.bc le1-llvm-none/lib/math/mad.cl.bc le1-llvm-none/lib/relational/any.cl.bc le1-llvm-none/lib/softfloat/softfloat.c.bc
	@echo LLVM-LINK le1-llvm-none/lib/builtins.link.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-link -o le1-llvm-none/lib/builtins.link.bc le1-llvm-none/lib/synchronization/barrier.cl.bc le1-llvm-none/lib/workitem/get_global_size.cl.bc le1-llvm-none/lib/workitem/get_group_id.cl.bc le1-llvm-none/lib/workitem/get_local_id.cl.bc le1-llvm-none/lib/workitem/get_global_id.cl.bc le1-llvm-none/lib/workitem/get_local_size.cl.bc le1-llvm-none/lib/workitem/get_num_groups.cl.bc le1-llvm-none/lib/workitem/get_global_offset.cl.bc le1-llvm-none/lib/geometric/cross.cl.bc le1-llvm-none/lib/geometric/dot.cl.bc le1-llvm-none/lib/geometric/length.cl.bc le1-llvm-none/lib/geometric/normalize.cl.bc le1-llvm-none/lib/integer/abs.cl.bc le1-llvm-none/lib/integer/add_sat.cl.bc le1-llvm-none/lib/integer/add_sat.ll.bc le1-llvm-none/lib/integer/add_sat_impl.ll.bc le1-llvm-none/lib/integer/sub_sat.cl.bc le1-llvm-none/lib/integer/sub_sat.ll.bc le1-llvm-none/lib/integer/sub_sat_impl.ll.bc le1-llvm-none/lib/math/hypot.cl.bc le1-llvm-none/lib/math/mad.cl.bc le1-llvm-none/lib/relational/any.cl.bc le1-llvm-none/lib/softfloat/softfloat.c.bc

le1-llvm-none/lib/builtins.opt.bc: le1-llvm-none/lib/builtins.link.bc
	@echo OPT le1-llvm-none/lib/builtins.opt.bc
	$(Verb) /opt/esdg-opencl/bin/opt -O3 -o le1-llvm-none/lib/builtins.opt.bc le1-llvm-none/lib/builtins.link.bc

le1-llvm-none/lib/builtins.bc: le1-llvm-none/lib/builtins.opt.bc utils/prepare-builtins
	@echo PREPARE-BUILTINS le1-llvm-none/lib/builtins.bc
	$(Verb) utils/prepare-builtins -o le1-llvm-none/lib/builtins.bc le1-llvm-none/lib/builtins.opt.bc

Makefile: ./configure.py ./build/metabuild.py ./build/ninja_syntax.py ./generic/lib/SOURCES ./le1/lib/SOURCES
	@echo CONFIGURE
	$(Verb) ./configure.py

all:: le1-llvm-none/lib/builtins.bc

clean: 
	rm -f utils/prepare-builtins.o utils/prepare-builtins le1-llvm-none/lib/synchronization/barrier.cl.bc le1-llvm-none/lib/workitem/get_global_size.cl.bc le1-llvm-none/lib/workitem/get_group_id.cl.bc le1-llvm-none/lib/workitem/get_local_id.cl.bc le1-llvm-none/lib/workitem/get_global_id.cl.bc le1-llvm-none/lib/workitem/get_local_size.cl.bc le1-llvm-none/lib/workitem/get_num_groups.cl.bc le1-llvm-none/lib/workitem/get_global_offset.cl.bc le1-llvm-none/lib/geometric/cross.cl.bc le1-llvm-none/lib/geometric/dot.cl.bc le1-llvm-none/lib/geometric/length.cl.bc le1-llvm-none/lib/geometric/normalize.cl.bc le1-llvm-none/lib/integer/abs.cl.bc le1-llvm-none/lib/integer/add_sat.cl.bc le1-llvm-none/lib/integer/add_sat.ll.bc le1-llvm-none/lib/integer/add_sat_impl.ll.bc le1-llvm-none/lib/integer/sub_sat.cl.bc le1-llvm-none/lib/integer/sub_sat.ll.bc le1-llvm-none/lib/integer/sub_sat_impl.ll.bc le1-llvm-none/lib/math/hypot.cl.bc le1-llvm-none/lib/math/mad.cl.bc le1-llvm-none/lib/relational/any.cl.bc le1-llvm-none/lib/softfloat/softfloat.c.bc le1-llvm-none/lib/builtins.link.bc le1-llvm-none/lib/builtins.opt.bc le1-llvm-none/lib/builtins.bc

distclean: clean
	rm -f Makefile
