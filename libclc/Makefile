all::

ifndef VERBOSE
  Verb = @
endif

utils/prepare-builtins.o: ./utils/prepare-builtins.cpp
	@echo CXX utils/prepare-builtins.o
	$(Verb) c++ -MMD -MF utils/prepare-builtins.o.d -I/opt/esdg-opencl/include  -fPIC -fvisibility-inlines-hidden -O3 -DNDEBUG  -D_GNU_SOURCE -Wall -W -Wno-unused-parameter -Wwrite-strings -pedantic -Wno-long-long -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fno-exceptions -fno-rtti -c -o utils/prepare-builtins.o ./utils/prepare-builtins.cpp
-include utils/prepare-builtins.o.d

utils/prepare-builtins: utils/prepare-builtins.o
	@echo LINK utils/prepare-builtins
	$(Verb) c++ -o utils/prepare-builtins utils/prepare-builtins.o -L/opt/esdg-opencl/lib   -lpthread -lLLVMBitWriter -lLLVMBitReader -lLLVMCore -lLLVMSupport -ldl

le1-llvm-none/lib/synchronization/barrier.cl.bc: ./le1/lib/synchronization/barrier.cl
	@echo LLVM-CC le1-llvm-none/lib/synchronization/barrier.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/synchronization/barrier.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/synchronization/barrier.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/synchronization/barrier.cl.bc ./le1/lib/synchronization/barrier.cl
-include le1-llvm-none/lib/synchronization/barrier.cl.bc.d

le1-llvm-none/lib/workitem/get_global_size.cl.bc: ./le1/lib/workitem/get_global_size.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_global_size.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_global_size.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_global_size.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_global_size.cl.bc ./le1/lib/workitem/get_global_size.cl
-include le1-llvm-none/lib/workitem/get_global_size.cl.bc.d

le1-llvm-none/lib/workitem/get_group_id.cl.bc: ./le1/lib/workitem/get_group_id.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_group_id.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_group_id.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_group_id.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_group_id.cl.bc ./le1/lib/workitem/get_group_id.cl
-include le1-llvm-none/lib/workitem/get_group_id.cl.bc.d

le1-llvm-none/lib/workitem/get_local_id.cl.bc: ./le1/lib/workitem/get_local_id.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_local_id.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_local_id.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_local_id.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_local_id.cl.bc ./le1/lib/workitem/get_local_id.cl
-include le1-llvm-none/lib/workitem/get_local_id.cl.bc.d

le1-llvm-none/lib/workitem/get_global_id.cl.bc: ./le1/lib/workitem/get_global_id.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_global_id.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_global_id.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_global_id.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_global_id.cl.bc ./le1/lib/workitem/get_global_id.cl
-include le1-llvm-none/lib/workitem/get_global_id.cl.bc.d

le1-llvm-none/lib/workitem/get_local_size.cl.bc: ./le1/lib/workitem/get_local_size.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_local_size.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_local_size.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_local_size.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_local_size.cl.bc ./le1/lib/workitem/get_local_size.cl
-include le1-llvm-none/lib/workitem/get_local_size.cl.bc.d

le1-llvm-none/lib/workitem/get_num_groups.cl.bc: ./le1/lib/workitem/get_num_groups.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_num_groups.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_num_groups.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_num_groups.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_num_groups.cl.bc ./le1/lib/workitem/get_num_groups.cl
-include le1-llvm-none/lib/workitem/get_num_groups.cl.bc.d

le1-llvm-none/lib/workitem/get_global_offset.cl.bc: ./le1/lib/workitem/get_global_offset.cl
	@echo LLVM-CC le1-llvm-none/lib/workitem/get_global_offset.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/workitem/get_global_offset.cl.bc.d -target le1-llvm-none -I`dirname ./le1/lib/workitem/get_global_offset.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/workitem/get_global_offset.cl.bc ./le1/lib/workitem/get_global_offset.cl
-include le1-llvm-none/lib/workitem/get_global_offset.cl.bc.d

le1-llvm-none/lib/convert.cl.bc: ./generic/lib/convert.cl
	@echo LLVM-CC le1-llvm-none/lib/convert.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/convert.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/convert.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/convert.cl.bc ./generic/lib/convert.cl
-include le1-llvm-none/lib/convert.cl.bc.d

le1-llvm-none/lib/geometric/cross.cl.bc: ./generic/lib/geometric/cross.cl
	@echo LLVM-CC le1-llvm-none/lib/geometric/cross.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/geometric/cross.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/geometric/cross.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/geometric/cross.cl.bc ./generic/lib/geometric/cross.cl
-include le1-llvm-none/lib/geometric/cross.cl.bc.d

le1-llvm-none/lib/geometric/dot.cl.bc: ./generic/lib/geometric/dot.cl
	@echo LLVM-CC le1-llvm-none/lib/geometric/dot.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/geometric/dot.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/geometric/dot.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/geometric/dot.cl.bc ./generic/lib/geometric/dot.cl
-include le1-llvm-none/lib/geometric/dot.cl.bc.d

le1-llvm-none/lib/geometric/length.cl.bc: ./generic/lib/geometric/length.cl
	@echo LLVM-CC le1-llvm-none/lib/geometric/length.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/geometric/length.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/geometric/length.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/geometric/length.cl.bc ./generic/lib/geometric/length.cl
-include le1-llvm-none/lib/geometric/length.cl.bc.d

le1-llvm-none/lib/geometric/normalize.cl.bc: ./generic/lib/geometric/normalize.cl
	@echo LLVM-CC le1-llvm-none/lib/geometric/normalize.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/geometric/normalize.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/geometric/normalize.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/geometric/normalize.cl.bc ./generic/lib/geometric/normalize.cl
-include le1-llvm-none/lib/geometric/normalize.cl.bc.d

le1-llvm-none/lib/integer/abs.cl.bc: ./generic/lib/integer/abs.cl
	@echo LLVM-CC le1-llvm-none/lib/integer/abs.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/integer/abs.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/integer/abs.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/integer/abs.cl.bc ./generic/lib/integer/abs.cl
-include le1-llvm-none/lib/integer/abs.cl.bc.d

le1-llvm-none/lib/integer/add_sat.cl.bc: ./generic/lib/integer/add_sat.cl
	@echo LLVM-CC le1-llvm-none/lib/integer/add_sat.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/integer/add_sat.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/integer/add_sat.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/integer/add_sat.cl.bc ./generic/lib/integer/add_sat.cl
-include le1-llvm-none/lib/integer/add_sat.cl.bc.d

le1-llvm-none/lib/integer/add_sat.ll.bc: ./generic/lib/integer/add_sat.ll
	@echo LLVM-AS le1-llvm-none/lib/integer/add_sat.ll.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-as -o le1-llvm-none/lib/integer/add_sat.ll.bc ./generic/lib/integer/add_sat.ll

le1-llvm-none/lib/integer/add_sat_impl.ll.bc: ./generic/lib/integer/add_sat_impl.ll
	@echo LLVM-AS le1-llvm-none/lib/integer/add_sat_impl.ll.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-as -o le1-llvm-none/lib/integer/add_sat_impl.ll.bc ./generic/lib/integer/add_sat_impl.ll

le1-llvm-none/lib/integer/sub_sat.cl.bc: ./generic/lib/integer/sub_sat.cl
	@echo LLVM-CC le1-llvm-none/lib/integer/sub_sat.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/integer/sub_sat.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/integer/sub_sat.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/integer/sub_sat.cl.bc ./generic/lib/integer/sub_sat.cl
-include le1-llvm-none/lib/integer/sub_sat.cl.bc.d

le1-llvm-none/lib/integer/sub_sat.ll.bc: ./generic/lib/integer/sub_sat.ll
	@echo LLVM-AS le1-llvm-none/lib/integer/sub_sat.ll.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-as -o le1-llvm-none/lib/integer/sub_sat.ll.bc ./generic/lib/integer/sub_sat.ll

le1-llvm-none/lib/integer/sub_sat_impl.ll.bc: ./generic/lib/integer/sub_sat_impl.ll
	@echo LLVM-AS le1-llvm-none/lib/integer/sub_sat_impl.ll.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-as -o le1-llvm-none/lib/integer/sub_sat_impl.ll.bc ./generic/lib/integer/sub_sat_impl.ll

le1-llvm-none/lib/math/hypot.cl.bc: ./generic/lib/math/hypot.cl
	@echo LLVM-CC le1-llvm-none/lib/math/hypot.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/math/hypot.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/math/hypot.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/math/hypot.cl.bc ./generic/lib/math/hypot.cl
-include le1-llvm-none/lib/math/hypot.cl.bc.d

le1-llvm-none/lib/math/mad.cl.bc: ./generic/lib/math/mad.cl
	@echo LLVM-CC le1-llvm-none/lib/math/mad.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/math/mad.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/math/mad.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/math/mad.cl.bc ./generic/lib/math/mad.cl
-include le1-llvm-none/lib/math/mad.cl.bc.d

le1-llvm-none/lib/relational/any.cl.bc: ./generic/lib/relational/any.cl
	@echo LLVM-CC le1-llvm-none/lib/relational/any.cl.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/relational/any.cl.bc.d -target le1-llvm-none -I`dirname ./generic/lib/relational/any.cl` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/relational/any.cl.bc ./generic/lib/relational/any.cl
-include le1-llvm-none/lib/relational/any.cl.bc.d

le1-llvm-none/lib/compiler-rt/absvdi2.c.bc: ./generic/lib/compiler-rt/absvdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/absvdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/absvdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/absvdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/absvdi2.c.bc ./generic/lib/compiler-rt/absvdi2.c
-include le1-llvm-none/lib/compiler-rt/absvdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/absvsi2.c.bc: ./generic/lib/compiler-rt/absvsi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/absvsi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/absvsi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/absvsi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/absvsi2.c.bc ./generic/lib/compiler-rt/absvsi2.c
-include le1-llvm-none/lib/compiler-rt/absvsi2.c.bc.d

le1-llvm-none/lib/compiler-rt/absvti2.c.bc: ./generic/lib/compiler-rt/absvti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/absvti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/absvti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/absvti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/absvti2.c.bc ./generic/lib/compiler-rt/absvti2.c
-include le1-llvm-none/lib/compiler-rt/absvti2.c.bc.d

le1-llvm-none/lib/compiler-rt/adddf3.c.bc: ./generic/lib/compiler-rt/adddf3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/adddf3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/adddf3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/adddf3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/adddf3.c.bc ./generic/lib/compiler-rt/adddf3.c
-include le1-llvm-none/lib/compiler-rt/adddf3.c.bc.d

le1-llvm-none/lib/compiler-rt/addsf3.c.bc: ./generic/lib/compiler-rt/addsf3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/addsf3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/addsf3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/addsf3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/addsf3.c.bc ./generic/lib/compiler-rt/addsf3.c
-include le1-llvm-none/lib/compiler-rt/addsf3.c.bc.d

le1-llvm-none/lib/compiler-rt/addvdi3.c.bc: ./generic/lib/compiler-rt/addvdi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/addvdi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/addvdi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/addvdi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/addvdi3.c.bc ./generic/lib/compiler-rt/addvdi3.c
-include le1-llvm-none/lib/compiler-rt/addvdi3.c.bc.d

le1-llvm-none/lib/compiler-rt/addvsi3.c.bc: ./generic/lib/compiler-rt/addvsi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/addvsi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/addvsi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/addvsi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/addvsi3.c.bc ./generic/lib/compiler-rt/addvsi3.c
-include le1-llvm-none/lib/compiler-rt/addvsi3.c.bc.d

le1-llvm-none/lib/compiler-rt/addvti3.c.bc: ./generic/lib/compiler-rt/addvti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/addvti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/addvti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/addvti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/addvti3.c.bc ./generic/lib/compiler-rt/addvti3.c
-include le1-llvm-none/lib/compiler-rt/addvti3.c.bc.d

le1-llvm-none/lib/compiler-rt/apple_versioning.c.bc: ./generic/lib/compiler-rt/apple_versioning.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/apple_versioning.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/apple_versioning.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/apple_versioning.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/apple_versioning.c.bc ./generic/lib/compiler-rt/apple_versioning.c
-include le1-llvm-none/lib/compiler-rt/apple_versioning.c.bc.d

le1-llvm-none/lib/compiler-rt/ashldi3.c.bc: ./generic/lib/compiler-rt/ashldi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ashldi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ashldi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ashldi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ashldi3.c.bc ./generic/lib/compiler-rt/ashldi3.c
-include le1-llvm-none/lib/compiler-rt/ashldi3.c.bc.d

le1-llvm-none/lib/compiler-rt/ashlti3.c.bc: ./generic/lib/compiler-rt/ashlti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ashlti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ashlti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ashlti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ashlti3.c.bc ./generic/lib/compiler-rt/ashlti3.c
-include le1-llvm-none/lib/compiler-rt/ashlti3.c.bc.d

le1-llvm-none/lib/compiler-rt/ashrdi3.c.bc: ./generic/lib/compiler-rt/ashrdi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ashrdi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ashrdi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ashrdi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ashrdi3.c.bc ./generic/lib/compiler-rt/ashrdi3.c
-include le1-llvm-none/lib/compiler-rt/ashrdi3.c.bc.d

le1-llvm-none/lib/compiler-rt/ashrti3.c.bc: ./generic/lib/compiler-rt/ashrti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ashrti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ashrti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ashrti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ashrti3.c.bc ./generic/lib/compiler-rt/ashrti3.c
-include le1-llvm-none/lib/compiler-rt/ashrti3.c.bc.d

le1-llvm-none/lib/compiler-rt/clear_cache.c.bc: ./generic/lib/compiler-rt/clear_cache.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/clear_cache.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/clear_cache.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/clear_cache.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/clear_cache.c.bc ./generic/lib/compiler-rt/clear_cache.c
-include le1-llvm-none/lib/compiler-rt/clear_cache.c.bc.d

le1-llvm-none/lib/compiler-rt/clzdi2.c.bc: ./generic/lib/compiler-rt/clzdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/clzdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/clzdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/clzdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/clzdi2.c.bc ./generic/lib/compiler-rt/clzdi2.c
-include le1-llvm-none/lib/compiler-rt/clzdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/clzsi2.c.bc: ./generic/lib/compiler-rt/clzsi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/clzsi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/clzsi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/clzsi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/clzsi2.c.bc ./generic/lib/compiler-rt/clzsi2.c
-include le1-llvm-none/lib/compiler-rt/clzsi2.c.bc.d

le1-llvm-none/lib/compiler-rt/clzti2.c.bc: ./generic/lib/compiler-rt/clzti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/clzti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/clzti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/clzti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/clzti2.c.bc ./generic/lib/compiler-rt/clzti2.c
-include le1-llvm-none/lib/compiler-rt/clzti2.c.bc.d

le1-llvm-none/lib/compiler-rt/cmpdi2.c.bc: ./generic/lib/compiler-rt/cmpdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/cmpdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/cmpdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/cmpdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/cmpdi2.c.bc ./generic/lib/compiler-rt/cmpdi2.c
-include le1-llvm-none/lib/compiler-rt/cmpdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/cmpti2.c.bc: ./generic/lib/compiler-rt/cmpti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/cmpti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/cmpti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/cmpti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/cmpti2.c.bc ./generic/lib/compiler-rt/cmpti2.c
-include le1-llvm-none/lib/compiler-rt/cmpti2.c.bc.d

le1-llvm-none/lib/compiler-rt/comparedf2.c.bc: ./generic/lib/compiler-rt/comparedf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/comparedf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/comparedf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/comparedf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/comparedf2.c.bc ./generic/lib/compiler-rt/comparedf2.c
-include le1-llvm-none/lib/compiler-rt/comparedf2.c.bc.d

le1-llvm-none/lib/compiler-rt/comparesf2.c.bc: ./generic/lib/compiler-rt/comparesf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/comparesf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/comparesf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/comparesf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/comparesf2.c.bc ./generic/lib/compiler-rt/comparesf2.c
-include le1-llvm-none/lib/compiler-rt/comparesf2.c.bc.d

le1-llvm-none/lib/compiler-rt/ctzdi2.c.bc: ./generic/lib/compiler-rt/ctzdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ctzdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ctzdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ctzdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ctzdi2.c.bc ./generic/lib/compiler-rt/ctzdi2.c
-include le1-llvm-none/lib/compiler-rt/ctzdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/ctzsi2.c.bc: ./generic/lib/compiler-rt/ctzsi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ctzsi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ctzsi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ctzsi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ctzsi2.c.bc ./generic/lib/compiler-rt/ctzsi2.c
-include le1-llvm-none/lib/compiler-rt/ctzsi2.c.bc.d

le1-llvm-none/lib/compiler-rt/ctzti2.c.bc: ./generic/lib/compiler-rt/ctzti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ctzti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ctzti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ctzti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ctzti2.c.bc ./generic/lib/compiler-rt/ctzti2.c
-include le1-llvm-none/lib/compiler-rt/ctzti2.c.bc.d

le1-llvm-none/lib/compiler-rt/divdc3.c.bc: ./generic/lib/compiler-rt/divdc3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divdc3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divdc3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divdc3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divdc3.c.bc ./generic/lib/compiler-rt/divdc3.c
-include le1-llvm-none/lib/compiler-rt/divdc3.c.bc.d

le1-llvm-none/lib/compiler-rt/divdf3.c.bc: ./generic/lib/compiler-rt/divdf3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divdf3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divdf3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divdf3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divdf3.c.bc ./generic/lib/compiler-rt/divdf3.c
-include le1-llvm-none/lib/compiler-rt/divdf3.c.bc.d

le1-llvm-none/lib/compiler-rt/divdi3.c.bc: ./generic/lib/compiler-rt/divdi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divdi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divdi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divdi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divdi3.c.bc ./generic/lib/compiler-rt/divdi3.c
-include le1-llvm-none/lib/compiler-rt/divdi3.c.bc.d

le1-llvm-none/lib/compiler-rt/divmoddi4.c.bc: ./generic/lib/compiler-rt/divmoddi4.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divmoddi4.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divmoddi4.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divmoddi4.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divmoddi4.c.bc ./generic/lib/compiler-rt/divmoddi4.c
-include le1-llvm-none/lib/compiler-rt/divmoddi4.c.bc.d

le1-llvm-none/lib/compiler-rt/divmodsi4.c.bc: ./generic/lib/compiler-rt/divmodsi4.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divmodsi4.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divmodsi4.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divmodsi4.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divmodsi4.c.bc ./generic/lib/compiler-rt/divmodsi4.c
-include le1-llvm-none/lib/compiler-rt/divmodsi4.c.bc.d

le1-llvm-none/lib/compiler-rt/divsc3.c.bc: ./generic/lib/compiler-rt/divsc3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divsc3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divsc3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divsc3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divsc3.c.bc ./generic/lib/compiler-rt/divsc3.c
-include le1-llvm-none/lib/compiler-rt/divsc3.c.bc.d

le1-llvm-none/lib/compiler-rt/divsf3.c.bc: ./generic/lib/compiler-rt/divsf3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divsf3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divsf3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divsf3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divsf3.c.bc ./generic/lib/compiler-rt/divsf3.c
-include le1-llvm-none/lib/compiler-rt/divsf3.c.bc.d

le1-llvm-none/lib/compiler-rt/divsi3.c.bc: ./generic/lib/compiler-rt/divsi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divsi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divsi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divsi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divsi3.c.bc ./generic/lib/compiler-rt/divsi3.c
-include le1-llvm-none/lib/compiler-rt/divsi3.c.bc.d

le1-llvm-none/lib/compiler-rt/divti3.c.bc: ./generic/lib/compiler-rt/divti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divti3.c.bc ./generic/lib/compiler-rt/divti3.c
-include le1-llvm-none/lib/compiler-rt/divti3.c.bc.d

le1-llvm-none/lib/compiler-rt/divxc3.c.bc: ./generic/lib/compiler-rt/divxc3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/divxc3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/divxc3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/divxc3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/divxc3.c.bc ./generic/lib/compiler-rt/divxc3.c
-include le1-llvm-none/lib/compiler-rt/divxc3.c.bc.d

le1-llvm-none/lib/compiler-rt/enable_execute_stack.c.bc: ./generic/lib/compiler-rt/enable_execute_stack.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/enable_execute_stack.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/enable_execute_stack.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/enable_execute_stack.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/enable_execute_stack.c.bc ./generic/lib/compiler-rt/enable_execute_stack.c
-include le1-llvm-none/lib/compiler-rt/enable_execute_stack.c.bc.d

le1-llvm-none/lib/compiler-rt/eprintf.c.bc: ./generic/lib/compiler-rt/eprintf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/eprintf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/eprintf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/eprintf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/eprintf.c.bc ./generic/lib/compiler-rt/eprintf.c
-include le1-llvm-none/lib/compiler-rt/eprintf.c.bc.d

le1-llvm-none/lib/compiler-rt/extendsfdf2.c.bc: ./generic/lib/compiler-rt/extendsfdf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/extendsfdf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/extendsfdf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/extendsfdf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/extendsfdf2.c.bc ./generic/lib/compiler-rt/extendsfdf2.c
-include le1-llvm-none/lib/compiler-rt/extendsfdf2.c.bc.d

le1-llvm-none/lib/compiler-rt/ffsdi2.c.bc: ./generic/lib/compiler-rt/ffsdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ffsdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ffsdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ffsdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ffsdi2.c.bc ./generic/lib/compiler-rt/ffsdi2.c
-include le1-llvm-none/lib/compiler-rt/ffsdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/ffsti2.c.bc: ./generic/lib/compiler-rt/ffsti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ffsti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ffsti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ffsti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ffsti2.c.bc ./generic/lib/compiler-rt/ffsti2.c
-include le1-llvm-none/lib/compiler-rt/ffsti2.c.bc.d

le1-llvm-none/lib/compiler-rt/fixdfdi.c.bc: ./generic/lib/compiler-rt/fixdfdi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixdfdi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixdfdi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixdfdi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixdfdi.c.bc ./generic/lib/compiler-rt/fixdfdi.c
-include le1-llvm-none/lib/compiler-rt/fixdfdi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixdfsi.c.bc: ./generic/lib/compiler-rt/fixdfsi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixdfsi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixdfsi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixdfsi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixdfsi.c.bc ./generic/lib/compiler-rt/fixdfsi.c
-include le1-llvm-none/lib/compiler-rt/fixdfsi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixdfti.c.bc: ./generic/lib/compiler-rt/fixdfti.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixdfti.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixdfti.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixdfti.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixdfti.c.bc ./generic/lib/compiler-rt/fixdfti.c
-include le1-llvm-none/lib/compiler-rt/fixdfti.c.bc.d

le1-llvm-none/lib/compiler-rt/fixsfdi.c.bc: ./generic/lib/compiler-rt/fixsfdi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixsfdi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixsfdi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixsfdi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixsfdi.c.bc ./generic/lib/compiler-rt/fixsfdi.c
-include le1-llvm-none/lib/compiler-rt/fixsfdi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixsfsi.c.bc: ./generic/lib/compiler-rt/fixsfsi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixsfsi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixsfsi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixsfsi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixsfsi.c.bc ./generic/lib/compiler-rt/fixsfsi.c
-include le1-llvm-none/lib/compiler-rt/fixsfsi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixsfti.c.bc: ./generic/lib/compiler-rt/fixsfti.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixsfti.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixsfti.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixsfti.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixsfti.c.bc ./generic/lib/compiler-rt/fixsfti.c
-include le1-llvm-none/lib/compiler-rt/fixsfti.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunsdfdi.c.bc: ./generic/lib/compiler-rt/fixunsdfdi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunsdfdi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunsdfdi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunsdfdi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunsdfdi.c.bc ./generic/lib/compiler-rt/fixunsdfdi.c
-include le1-llvm-none/lib/compiler-rt/fixunsdfdi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunsdfsi.c.bc: ./generic/lib/compiler-rt/fixunsdfsi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunsdfsi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunsdfsi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunsdfsi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunsdfsi.c.bc ./generic/lib/compiler-rt/fixunsdfsi.c
-include le1-llvm-none/lib/compiler-rt/fixunsdfsi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunsdfti.c.bc: ./generic/lib/compiler-rt/fixunsdfti.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunsdfti.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunsdfti.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunsdfti.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunsdfti.c.bc ./generic/lib/compiler-rt/fixunsdfti.c
-include le1-llvm-none/lib/compiler-rt/fixunsdfti.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunssfdi.c.bc: ./generic/lib/compiler-rt/fixunssfdi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunssfdi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunssfdi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunssfdi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunssfdi.c.bc ./generic/lib/compiler-rt/fixunssfdi.c
-include le1-llvm-none/lib/compiler-rt/fixunssfdi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunssfsi.c.bc: ./generic/lib/compiler-rt/fixunssfsi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunssfsi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunssfsi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunssfsi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunssfsi.c.bc ./generic/lib/compiler-rt/fixunssfsi.c
-include le1-llvm-none/lib/compiler-rt/fixunssfsi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunssfti.c.bc: ./generic/lib/compiler-rt/fixunssfti.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunssfti.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunssfti.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunssfti.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunssfti.c.bc ./generic/lib/compiler-rt/fixunssfti.c
-include le1-llvm-none/lib/compiler-rt/fixunssfti.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunsxfdi.c.bc: ./generic/lib/compiler-rt/fixunsxfdi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunsxfdi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunsxfdi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunsxfdi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunsxfdi.c.bc ./generic/lib/compiler-rt/fixunsxfdi.c
-include le1-llvm-none/lib/compiler-rt/fixunsxfdi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunsxfsi.c.bc: ./generic/lib/compiler-rt/fixunsxfsi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunsxfsi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunsxfsi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunsxfsi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunsxfsi.c.bc ./generic/lib/compiler-rt/fixunsxfsi.c
-include le1-llvm-none/lib/compiler-rt/fixunsxfsi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixunsxfti.c.bc: ./generic/lib/compiler-rt/fixunsxfti.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixunsxfti.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixunsxfti.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixunsxfti.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixunsxfti.c.bc ./generic/lib/compiler-rt/fixunsxfti.c
-include le1-llvm-none/lib/compiler-rt/fixunsxfti.c.bc.d

le1-llvm-none/lib/compiler-rt/fixxfdi.c.bc: ./generic/lib/compiler-rt/fixxfdi.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixxfdi.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixxfdi.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixxfdi.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixxfdi.c.bc ./generic/lib/compiler-rt/fixxfdi.c
-include le1-llvm-none/lib/compiler-rt/fixxfdi.c.bc.d

le1-llvm-none/lib/compiler-rt/fixxfti.c.bc: ./generic/lib/compiler-rt/fixxfti.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/fixxfti.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/fixxfti.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/fixxfti.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/fixxfti.c.bc ./generic/lib/compiler-rt/fixxfti.c
-include le1-llvm-none/lib/compiler-rt/fixxfti.c.bc.d

le1-llvm-none/lib/compiler-rt/floatdidf.c.bc: ./generic/lib/compiler-rt/floatdidf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatdidf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatdidf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatdidf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatdidf.c.bc ./generic/lib/compiler-rt/floatdidf.c
-include le1-llvm-none/lib/compiler-rt/floatdidf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatdisf.c.bc: ./generic/lib/compiler-rt/floatdisf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatdisf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatdisf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatdisf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatdisf.c.bc ./generic/lib/compiler-rt/floatdisf.c
-include le1-llvm-none/lib/compiler-rt/floatdisf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatdixf.c.bc: ./generic/lib/compiler-rt/floatdixf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatdixf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatdixf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatdixf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatdixf.c.bc ./generic/lib/compiler-rt/floatdixf.c
-include le1-llvm-none/lib/compiler-rt/floatdixf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatsidf.c.bc: ./generic/lib/compiler-rt/floatsidf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatsidf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatsidf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatsidf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatsidf.c.bc ./generic/lib/compiler-rt/floatsidf.c
-include le1-llvm-none/lib/compiler-rt/floatsidf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatsisf.c.bc: ./generic/lib/compiler-rt/floatsisf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatsisf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatsisf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatsisf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatsisf.c.bc ./generic/lib/compiler-rt/floatsisf.c
-include le1-llvm-none/lib/compiler-rt/floatsisf.c.bc.d

le1-llvm-none/lib/compiler-rt/floattidf.c.bc: ./generic/lib/compiler-rt/floattidf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floattidf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floattidf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floattidf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floattidf.c.bc ./generic/lib/compiler-rt/floattidf.c
-include le1-llvm-none/lib/compiler-rt/floattidf.c.bc.d

le1-llvm-none/lib/compiler-rt/floattisf.c.bc: ./generic/lib/compiler-rt/floattisf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floattisf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floattisf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floattisf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floattisf.c.bc ./generic/lib/compiler-rt/floattisf.c
-include le1-llvm-none/lib/compiler-rt/floattisf.c.bc.d

le1-llvm-none/lib/compiler-rt/floattixf.c.bc: ./generic/lib/compiler-rt/floattixf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floattixf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floattixf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floattixf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floattixf.c.bc ./generic/lib/compiler-rt/floattixf.c
-include le1-llvm-none/lib/compiler-rt/floattixf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatundidf.c.bc: ./generic/lib/compiler-rt/floatundidf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatundidf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatundidf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatundidf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatundidf.c.bc ./generic/lib/compiler-rt/floatundidf.c
-include le1-llvm-none/lib/compiler-rt/floatundidf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatundisf.c.bc: ./generic/lib/compiler-rt/floatundisf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatundisf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatundisf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatundisf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatundisf.c.bc ./generic/lib/compiler-rt/floatundisf.c
-include le1-llvm-none/lib/compiler-rt/floatundisf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatundixf.c.bc: ./generic/lib/compiler-rt/floatundixf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatundixf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatundixf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatundixf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatundixf.c.bc ./generic/lib/compiler-rt/floatundixf.c
-include le1-llvm-none/lib/compiler-rt/floatundixf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatunsidf.c.bc: ./generic/lib/compiler-rt/floatunsidf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatunsidf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatunsidf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatunsidf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatunsidf.c.bc ./generic/lib/compiler-rt/floatunsidf.c
-include le1-llvm-none/lib/compiler-rt/floatunsidf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatunsisf.c.bc: ./generic/lib/compiler-rt/floatunsisf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatunsisf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatunsisf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatunsisf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatunsisf.c.bc ./generic/lib/compiler-rt/floatunsisf.c
-include le1-llvm-none/lib/compiler-rt/floatunsisf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatuntidf.c.bc: ./generic/lib/compiler-rt/floatuntidf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatuntidf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatuntidf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatuntidf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatuntidf.c.bc ./generic/lib/compiler-rt/floatuntidf.c
-include le1-llvm-none/lib/compiler-rt/floatuntidf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatuntisf.c.bc: ./generic/lib/compiler-rt/floatuntisf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatuntisf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatuntisf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatuntisf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatuntisf.c.bc ./generic/lib/compiler-rt/floatuntisf.c
-include le1-llvm-none/lib/compiler-rt/floatuntisf.c.bc.d

le1-llvm-none/lib/compiler-rt/floatuntixf.c.bc: ./generic/lib/compiler-rt/floatuntixf.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/floatuntixf.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/floatuntixf.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/floatuntixf.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/floatuntixf.c.bc ./generic/lib/compiler-rt/floatuntixf.c
-include le1-llvm-none/lib/compiler-rt/floatuntixf.c.bc.d

le1-llvm-none/lib/compiler-rt/gcc_personality_v0.c.bc: ./generic/lib/compiler-rt/gcc_personality_v0.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/gcc_personality_v0.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/gcc_personality_v0.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/gcc_personality_v0.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/gcc_personality_v0.c.bc ./generic/lib/compiler-rt/gcc_personality_v0.c
-include le1-llvm-none/lib/compiler-rt/gcc_personality_v0.c.bc.d

le1-llvm-none/lib/compiler-rt/int_util.c.bc: ./generic/lib/compiler-rt/int_util.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/int_util.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/int_util.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/int_util.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/int_util.c.bc ./generic/lib/compiler-rt/int_util.c
-include le1-llvm-none/lib/compiler-rt/int_util.c.bc.d

le1-llvm-none/lib/compiler-rt/lshrdi3.c.bc: ./generic/lib/compiler-rt/lshrdi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/lshrdi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/lshrdi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/lshrdi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/lshrdi3.c.bc ./generic/lib/compiler-rt/lshrdi3.c
-include le1-llvm-none/lib/compiler-rt/lshrdi3.c.bc.d

le1-llvm-none/lib/compiler-rt/lshrti3.c.bc: ./generic/lib/compiler-rt/lshrti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/lshrti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/lshrti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/lshrti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/lshrti3.c.bc ./generic/lib/compiler-rt/lshrti3.c
-include le1-llvm-none/lib/compiler-rt/lshrti3.c.bc.d

le1-llvm-none/lib/compiler-rt/moddi3.c.bc: ./generic/lib/compiler-rt/moddi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/moddi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/moddi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/moddi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/moddi3.c.bc ./generic/lib/compiler-rt/moddi3.c
-include le1-llvm-none/lib/compiler-rt/moddi3.c.bc.d

le1-llvm-none/lib/compiler-rt/modsi3.c.bc: ./generic/lib/compiler-rt/modsi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/modsi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/modsi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/modsi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/modsi3.c.bc ./generic/lib/compiler-rt/modsi3.c
-include le1-llvm-none/lib/compiler-rt/modsi3.c.bc.d

le1-llvm-none/lib/compiler-rt/modti3.c.bc: ./generic/lib/compiler-rt/modti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/modti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/modti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/modti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/modti3.c.bc ./generic/lib/compiler-rt/modti3.c
-include le1-llvm-none/lib/compiler-rt/modti3.c.bc.d

le1-llvm-none/lib/compiler-rt/muldc3.c.bc: ./generic/lib/compiler-rt/muldc3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/muldc3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/muldc3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/muldc3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/muldc3.c.bc ./generic/lib/compiler-rt/muldc3.c
-include le1-llvm-none/lib/compiler-rt/muldc3.c.bc.d

le1-llvm-none/lib/compiler-rt/muldf3.c.bc: ./generic/lib/compiler-rt/muldf3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/muldf3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/muldf3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/muldf3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/muldf3.c.bc ./generic/lib/compiler-rt/muldf3.c
-include le1-llvm-none/lib/compiler-rt/muldf3.c.bc.d

le1-llvm-none/lib/compiler-rt/muldi3.c.bc: ./generic/lib/compiler-rt/muldi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/muldi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/muldi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/muldi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/muldi3.c.bc ./generic/lib/compiler-rt/muldi3.c
-include le1-llvm-none/lib/compiler-rt/muldi3.c.bc.d

le1-llvm-none/lib/compiler-rt/mulodi4.c.bc: ./generic/lib/compiler-rt/mulodi4.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/mulodi4.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/mulodi4.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/mulodi4.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/mulodi4.c.bc ./generic/lib/compiler-rt/mulodi4.c
-include le1-llvm-none/lib/compiler-rt/mulodi4.c.bc.d

le1-llvm-none/lib/compiler-rt/mulosi4.c.bc: ./generic/lib/compiler-rt/mulosi4.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/mulosi4.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/mulosi4.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/mulosi4.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/mulosi4.c.bc ./generic/lib/compiler-rt/mulosi4.c
-include le1-llvm-none/lib/compiler-rt/mulosi4.c.bc.d

le1-llvm-none/lib/compiler-rt/muloti4.c.bc: ./generic/lib/compiler-rt/muloti4.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/muloti4.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/muloti4.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/muloti4.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/muloti4.c.bc ./generic/lib/compiler-rt/muloti4.c
-include le1-llvm-none/lib/compiler-rt/muloti4.c.bc.d

le1-llvm-none/lib/compiler-rt/mulsc3.c.bc: ./generic/lib/compiler-rt/mulsc3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/mulsc3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/mulsc3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/mulsc3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/mulsc3.c.bc ./generic/lib/compiler-rt/mulsc3.c
-include le1-llvm-none/lib/compiler-rt/mulsc3.c.bc.d

le1-llvm-none/lib/compiler-rt/mulsf3.c.bc: ./generic/lib/compiler-rt/mulsf3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/mulsf3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/mulsf3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/mulsf3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/mulsf3.c.bc ./generic/lib/compiler-rt/mulsf3.c
-include le1-llvm-none/lib/compiler-rt/mulsf3.c.bc.d

le1-llvm-none/lib/compiler-rt/multi3.c.bc: ./generic/lib/compiler-rt/multi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/multi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/multi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/multi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/multi3.c.bc ./generic/lib/compiler-rt/multi3.c
-include le1-llvm-none/lib/compiler-rt/multi3.c.bc.d

le1-llvm-none/lib/compiler-rt/mulvdi3.c.bc: ./generic/lib/compiler-rt/mulvdi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/mulvdi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/mulvdi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/mulvdi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/mulvdi3.c.bc ./generic/lib/compiler-rt/mulvdi3.c
-include le1-llvm-none/lib/compiler-rt/mulvdi3.c.bc.d

le1-llvm-none/lib/compiler-rt/mulvsi3.c.bc: ./generic/lib/compiler-rt/mulvsi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/mulvsi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/mulvsi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/mulvsi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/mulvsi3.c.bc ./generic/lib/compiler-rt/mulvsi3.c
-include le1-llvm-none/lib/compiler-rt/mulvsi3.c.bc.d

le1-llvm-none/lib/compiler-rt/mulvti3.c.bc: ./generic/lib/compiler-rt/mulvti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/mulvti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/mulvti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/mulvti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/mulvti3.c.bc ./generic/lib/compiler-rt/mulvti3.c
-include le1-llvm-none/lib/compiler-rt/mulvti3.c.bc.d

le1-llvm-none/lib/compiler-rt/mulxc3.c.bc: ./generic/lib/compiler-rt/mulxc3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/mulxc3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/mulxc3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/mulxc3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/mulxc3.c.bc ./generic/lib/compiler-rt/mulxc3.c
-include le1-llvm-none/lib/compiler-rt/mulxc3.c.bc.d

le1-llvm-none/lib/compiler-rt/negdf2.c.bc: ./generic/lib/compiler-rt/negdf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/negdf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/negdf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/negdf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/negdf2.c.bc ./generic/lib/compiler-rt/negdf2.c
-include le1-llvm-none/lib/compiler-rt/negdf2.c.bc.d

le1-llvm-none/lib/compiler-rt/negdi2.c.bc: ./generic/lib/compiler-rt/negdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/negdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/negdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/negdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/negdi2.c.bc ./generic/lib/compiler-rt/negdi2.c
-include le1-llvm-none/lib/compiler-rt/negdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/negsf2.c.bc: ./generic/lib/compiler-rt/negsf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/negsf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/negsf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/negsf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/negsf2.c.bc ./generic/lib/compiler-rt/negsf2.c
-include le1-llvm-none/lib/compiler-rt/negsf2.c.bc.d

le1-llvm-none/lib/compiler-rt/negti2.c.bc: ./generic/lib/compiler-rt/negti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/negti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/negti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/negti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/negti2.c.bc ./generic/lib/compiler-rt/negti2.c
-include le1-llvm-none/lib/compiler-rt/negti2.c.bc.d

le1-llvm-none/lib/compiler-rt/negvdi2.c.bc: ./generic/lib/compiler-rt/negvdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/negvdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/negvdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/negvdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/negvdi2.c.bc ./generic/lib/compiler-rt/negvdi2.c
-include le1-llvm-none/lib/compiler-rt/negvdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/negvsi2.c.bc: ./generic/lib/compiler-rt/negvsi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/negvsi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/negvsi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/negvsi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/negvsi2.c.bc ./generic/lib/compiler-rt/negvsi2.c
-include le1-llvm-none/lib/compiler-rt/negvsi2.c.bc.d

le1-llvm-none/lib/compiler-rt/negvti2.c.bc: ./generic/lib/compiler-rt/negvti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/negvti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/negvti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/negvti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/negvti2.c.bc ./generic/lib/compiler-rt/negvti2.c
-include le1-llvm-none/lib/compiler-rt/negvti2.c.bc.d

le1-llvm-none/lib/compiler-rt/paritydi2.c.bc: ./generic/lib/compiler-rt/paritydi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/paritydi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/paritydi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/paritydi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/paritydi2.c.bc ./generic/lib/compiler-rt/paritydi2.c
-include le1-llvm-none/lib/compiler-rt/paritydi2.c.bc.d

le1-llvm-none/lib/compiler-rt/paritysi2.c.bc: ./generic/lib/compiler-rt/paritysi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/paritysi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/paritysi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/paritysi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/paritysi2.c.bc ./generic/lib/compiler-rt/paritysi2.c
-include le1-llvm-none/lib/compiler-rt/paritysi2.c.bc.d

le1-llvm-none/lib/compiler-rt/parityti2.c.bc: ./generic/lib/compiler-rt/parityti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/parityti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/parityti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/parityti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/parityti2.c.bc ./generic/lib/compiler-rt/parityti2.c
-include le1-llvm-none/lib/compiler-rt/parityti2.c.bc.d

le1-llvm-none/lib/compiler-rt/popcountdi2.c.bc: ./generic/lib/compiler-rt/popcountdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/popcountdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/popcountdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/popcountdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/popcountdi2.c.bc ./generic/lib/compiler-rt/popcountdi2.c
-include le1-llvm-none/lib/compiler-rt/popcountdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/popcountsi2.c.bc: ./generic/lib/compiler-rt/popcountsi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/popcountsi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/popcountsi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/popcountsi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/popcountsi2.c.bc ./generic/lib/compiler-rt/popcountsi2.c
-include le1-llvm-none/lib/compiler-rt/popcountsi2.c.bc.d

le1-llvm-none/lib/compiler-rt/popcountti2.c.bc: ./generic/lib/compiler-rt/popcountti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/popcountti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/popcountti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/popcountti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/popcountti2.c.bc ./generic/lib/compiler-rt/popcountti2.c
-include le1-llvm-none/lib/compiler-rt/popcountti2.c.bc.d

le1-llvm-none/lib/compiler-rt/powidf2.c.bc: ./generic/lib/compiler-rt/powidf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/powidf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/powidf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/powidf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/powidf2.c.bc ./generic/lib/compiler-rt/powidf2.c
-include le1-llvm-none/lib/compiler-rt/powidf2.c.bc.d

le1-llvm-none/lib/compiler-rt/powisf2.c.bc: ./generic/lib/compiler-rt/powisf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/powisf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/powisf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/powisf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/powisf2.c.bc ./generic/lib/compiler-rt/powisf2.c
-include le1-llvm-none/lib/compiler-rt/powisf2.c.bc.d

le1-llvm-none/lib/compiler-rt/powitf2.c.bc: ./generic/lib/compiler-rt/powitf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/powitf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/powitf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/powitf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/powitf2.c.bc ./generic/lib/compiler-rt/powitf2.c
-include le1-llvm-none/lib/compiler-rt/powitf2.c.bc.d

le1-llvm-none/lib/compiler-rt/powixf2.c.bc: ./generic/lib/compiler-rt/powixf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/powixf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/powixf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/powixf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/powixf2.c.bc ./generic/lib/compiler-rt/powixf2.c
-include le1-llvm-none/lib/compiler-rt/powixf2.c.bc.d

le1-llvm-none/lib/compiler-rt/subdf3.c.bc: ./generic/lib/compiler-rt/subdf3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/subdf3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/subdf3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/subdf3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/subdf3.c.bc ./generic/lib/compiler-rt/subdf3.c
-include le1-llvm-none/lib/compiler-rt/subdf3.c.bc.d

le1-llvm-none/lib/compiler-rt/subsf3.c.bc: ./generic/lib/compiler-rt/subsf3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/subsf3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/subsf3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/subsf3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/subsf3.c.bc ./generic/lib/compiler-rt/subsf3.c
-include le1-llvm-none/lib/compiler-rt/subsf3.c.bc.d

le1-llvm-none/lib/compiler-rt/subvdi3.c.bc: ./generic/lib/compiler-rt/subvdi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/subvdi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/subvdi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/subvdi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/subvdi3.c.bc ./generic/lib/compiler-rt/subvdi3.c
-include le1-llvm-none/lib/compiler-rt/subvdi3.c.bc.d

le1-llvm-none/lib/compiler-rt/subvsi3.c.bc: ./generic/lib/compiler-rt/subvsi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/subvsi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/subvsi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/subvsi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/subvsi3.c.bc ./generic/lib/compiler-rt/subvsi3.c
-include le1-llvm-none/lib/compiler-rt/subvsi3.c.bc.d

le1-llvm-none/lib/compiler-rt/subvti3.c.bc: ./generic/lib/compiler-rt/subvti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/subvti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/subvti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/subvti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/subvti3.c.bc ./generic/lib/compiler-rt/subvti3.c
-include le1-llvm-none/lib/compiler-rt/subvti3.c.bc.d

le1-llvm-none/lib/compiler-rt/trampoline_setup.c.bc: ./generic/lib/compiler-rt/trampoline_setup.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/trampoline_setup.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/trampoline_setup.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/trampoline_setup.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/trampoline_setup.c.bc ./generic/lib/compiler-rt/trampoline_setup.c
-include le1-llvm-none/lib/compiler-rt/trampoline_setup.c.bc.d

le1-llvm-none/lib/compiler-rt/truncdfsf2.c.bc: ./generic/lib/compiler-rt/truncdfsf2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/truncdfsf2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/truncdfsf2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/truncdfsf2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/truncdfsf2.c.bc ./generic/lib/compiler-rt/truncdfsf2.c
-include le1-llvm-none/lib/compiler-rt/truncdfsf2.c.bc.d

le1-llvm-none/lib/compiler-rt/ucmpdi2.c.bc: ./generic/lib/compiler-rt/ucmpdi2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ucmpdi2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ucmpdi2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ucmpdi2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ucmpdi2.c.bc ./generic/lib/compiler-rt/ucmpdi2.c
-include le1-llvm-none/lib/compiler-rt/ucmpdi2.c.bc.d

le1-llvm-none/lib/compiler-rt/ucmpti2.c.bc: ./generic/lib/compiler-rt/ucmpti2.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/ucmpti2.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/ucmpti2.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/ucmpti2.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/ucmpti2.c.bc ./generic/lib/compiler-rt/ucmpti2.c
-include le1-llvm-none/lib/compiler-rt/ucmpti2.c.bc.d

le1-llvm-none/lib/compiler-rt/udivdi3.c.bc: ./generic/lib/compiler-rt/udivdi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/udivdi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/udivdi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/udivdi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/udivdi3.c.bc ./generic/lib/compiler-rt/udivdi3.c
-include le1-llvm-none/lib/compiler-rt/udivdi3.c.bc.d

le1-llvm-none/lib/compiler-rt/udivmoddi4.c.bc: ./generic/lib/compiler-rt/udivmoddi4.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/udivmoddi4.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/udivmoddi4.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/udivmoddi4.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/udivmoddi4.c.bc ./generic/lib/compiler-rt/udivmoddi4.c
-include le1-llvm-none/lib/compiler-rt/udivmoddi4.c.bc.d

le1-llvm-none/lib/compiler-rt/udivmodsi4.c.bc: ./generic/lib/compiler-rt/udivmodsi4.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/udivmodsi4.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/udivmodsi4.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/udivmodsi4.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/udivmodsi4.c.bc ./generic/lib/compiler-rt/udivmodsi4.c
-include le1-llvm-none/lib/compiler-rt/udivmodsi4.c.bc.d

le1-llvm-none/lib/compiler-rt/udivmodti4.c.bc: ./generic/lib/compiler-rt/udivmodti4.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/udivmodti4.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/udivmodti4.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/udivmodti4.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/udivmodti4.c.bc ./generic/lib/compiler-rt/udivmodti4.c
-include le1-llvm-none/lib/compiler-rt/udivmodti4.c.bc.d

le1-llvm-none/lib/compiler-rt/udivsi3.c.bc: ./generic/lib/compiler-rt/udivsi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/udivsi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/udivsi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/udivsi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/udivsi3.c.bc ./generic/lib/compiler-rt/udivsi3.c
-include le1-llvm-none/lib/compiler-rt/udivsi3.c.bc.d

le1-llvm-none/lib/compiler-rt/udivti3.c.bc: ./generic/lib/compiler-rt/udivti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/udivti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/udivti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/udivti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/udivti3.c.bc ./generic/lib/compiler-rt/udivti3.c
-include le1-llvm-none/lib/compiler-rt/udivti3.c.bc.d

le1-llvm-none/lib/compiler-rt/umoddi3.c.bc: ./generic/lib/compiler-rt/umoddi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/umoddi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/umoddi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/umoddi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/umoddi3.c.bc ./generic/lib/compiler-rt/umoddi3.c
-include le1-llvm-none/lib/compiler-rt/umoddi3.c.bc.d

le1-llvm-none/lib/compiler-rt/umodsi3.c.bc: ./generic/lib/compiler-rt/umodsi3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/umodsi3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/umodsi3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/umodsi3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/umodsi3.c.bc ./generic/lib/compiler-rt/umodsi3.c
-include le1-llvm-none/lib/compiler-rt/umodsi3.c.bc.d

le1-llvm-none/lib/compiler-rt/umodti3.c.bc: ./generic/lib/compiler-rt/umodti3.c
	@echo LLVM-CC le1-llvm-none/lib/compiler-rt/umodti3.c.bc
	$(Verb) /opt/esdg-opencl/bin/clang-3.2 -MMD -MF le1-llvm-none/lib/compiler-rt/umodti3.c.bc.d -target le1-llvm-none -I`dirname ./generic/lib/compiler-rt/umodti3.c` -I./le1/include -I./generic/include -I/usr/include/x86_64-linux-gnu -Dcl_clang_storage_class_specifiers -emit-llvm -c -o le1-llvm-none/lib/compiler-rt/umodti3.c.bc ./generic/lib/compiler-rt/umodti3.c
-include le1-llvm-none/lib/compiler-rt/umodti3.c.bc.d

le1-llvm-none/lib/builtins.link.bc: le1-llvm-none/lib/synchronization/barrier.cl.bc le1-llvm-none/lib/workitem/get_global_size.cl.bc le1-llvm-none/lib/workitem/get_group_id.cl.bc le1-llvm-none/lib/workitem/get_local_id.cl.bc le1-llvm-none/lib/workitem/get_global_id.cl.bc le1-llvm-none/lib/workitem/get_local_size.cl.bc le1-llvm-none/lib/workitem/get_num_groups.cl.bc le1-llvm-none/lib/workitem/get_global_offset.cl.bc le1-llvm-none/lib/convert.cl.bc le1-llvm-none/lib/geometric/cross.cl.bc le1-llvm-none/lib/geometric/dot.cl.bc le1-llvm-none/lib/geometric/length.cl.bc le1-llvm-none/lib/geometric/normalize.cl.bc le1-llvm-none/lib/integer/abs.cl.bc le1-llvm-none/lib/integer/add_sat.cl.bc le1-llvm-none/lib/integer/add_sat.ll.bc le1-llvm-none/lib/integer/add_sat_impl.ll.bc le1-llvm-none/lib/integer/sub_sat.cl.bc le1-llvm-none/lib/integer/sub_sat.ll.bc le1-llvm-none/lib/integer/sub_sat_impl.ll.bc le1-llvm-none/lib/math/hypot.cl.bc le1-llvm-none/lib/math/mad.cl.bc le1-llvm-none/lib/relational/any.cl.bc le1-llvm-none/lib/compiler-rt/absvdi2.c.bc le1-llvm-none/lib/compiler-rt/absvsi2.c.bc le1-llvm-none/lib/compiler-rt/absvti2.c.bc le1-llvm-none/lib/compiler-rt/adddf3.c.bc le1-llvm-none/lib/compiler-rt/addsf3.c.bc le1-llvm-none/lib/compiler-rt/addvdi3.c.bc le1-llvm-none/lib/compiler-rt/addvsi3.c.bc le1-llvm-none/lib/compiler-rt/addvti3.c.bc le1-llvm-none/lib/compiler-rt/apple_versioning.c.bc le1-llvm-none/lib/compiler-rt/ashldi3.c.bc le1-llvm-none/lib/compiler-rt/ashlti3.c.bc le1-llvm-none/lib/compiler-rt/ashrdi3.c.bc le1-llvm-none/lib/compiler-rt/ashrti3.c.bc le1-llvm-none/lib/compiler-rt/clear_cache.c.bc le1-llvm-none/lib/compiler-rt/clzdi2.c.bc le1-llvm-none/lib/compiler-rt/clzsi2.c.bc le1-llvm-none/lib/compiler-rt/clzti2.c.bc le1-llvm-none/lib/compiler-rt/cmpdi2.c.bc le1-llvm-none/lib/compiler-rt/cmpti2.c.bc le1-llvm-none/lib/compiler-rt/comparedf2.c.bc le1-llvm-none/lib/compiler-rt/comparesf2.c.bc le1-llvm-none/lib/compiler-rt/ctzdi2.c.bc le1-llvm-none/lib/compiler-rt/ctzsi2.c.bc le1-llvm-none/lib/compiler-rt/ctzti2.c.bc le1-llvm-none/lib/compiler-rt/divdc3.c.bc le1-llvm-none/lib/compiler-rt/divdf3.c.bc le1-llvm-none/lib/compiler-rt/divdi3.c.bc le1-llvm-none/lib/compiler-rt/divmoddi4.c.bc le1-llvm-none/lib/compiler-rt/divmodsi4.c.bc le1-llvm-none/lib/compiler-rt/divsc3.c.bc le1-llvm-none/lib/compiler-rt/divsf3.c.bc le1-llvm-none/lib/compiler-rt/divsi3.c.bc le1-llvm-none/lib/compiler-rt/divti3.c.bc le1-llvm-none/lib/compiler-rt/divxc3.c.bc le1-llvm-none/lib/compiler-rt/enable_execute_stack.c.bc le1-llvm-none/lib/compiler-rt/eprintf.c.bc le1-llvm-none/lib/compiler-rt/extendsfdf2.c.bc le1-llvm-none/lib/compiler-rt/ffsdi2.c.bc le1-llvm-none/lib/compiler-rt/ffsti2.c.bc le1-llvm-none/lib/compiler-rt/fixdfdi.c.bc le1-llvm-none/lib/compiler-rt/fixdfsi.c.bc le1-llvm-none/lib/compiler-rt/fixdfti.c.bc le1-llvm-none/lib/compiler-rt/fixsfdi.c.bc le1-llvm-none/lib/compiler-rt/fixsfsi.c.bc le1-llvm-none/lib/compiler-rt/fixsfti.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfti.c.bc le1-llvm-none/lib/compiler-rt/fixunssfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunssfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunssfti.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfti.c.bc le1-llvm-none/lib/compiler-rt/fixxfdi.c.bc le1-llvm-none/lib/compiler-rt/fixxfti.c.bc le1-llvm-none/lib/compiler-rt/floatdidf.c.bc le1-llvm-none/lib/compiler-rt/floatdisf.c.bc le1-llvm-none/lib/compiler-rt/floatdixf.c.bc le1-llvm-none/lib/compiler-rt/floatsidf.c.bc le1-llvm-none/lib/compiler-rt/floatsisf.c.bc le1-llvm-none/lib/compiler-rt/floattidf.c.bc le1-llvm-none/lib/compiler-rt/floattisf.c.bc le1-llvm-none/lib/compiler-rt/floattixf.c.bc le1-llvm-none/lib/compiler-rt/floatundidf.c.bc le1-llvm-none/lib/compiler-rt/floatundisf.c.bc le1-llvm-none/lib/compiler-rt/floatundixf.c.bc le1-llvm-none/lib/compiler-rt/floatunsidf.c.bc le1-llvm-none/lib/compiler-rt/floatunsisf.c.bc le1-llvm-none/lib/compiler-rt/floatuntidf.c.bc le1-llvm-none/lib/compiler-rt/floatuntisf.c.bc le1-llvm-none/lib/compiler-rt/floatuntixf.c.bc le1-llvm-none/lib/compiler-rt/gcc_personality_v0.c.bc le1-llvm-none/lib/compiler-rt/int_util.c.bc le1-llvm-none/lib/compiler-rt/lshrdi3.c.bc le1-llvm-none/lib/compiler-rt/lshrti3.c.bc le1-llvm-none/lib/compiler-rt/moddi3.c.bc le1-llvm-none/lib/compiler-rt/modsi3.c.bc le1-llvm-none/lib/compiler-rt/modti3.c.bc le1-llvm-none/lib/compiler-rt/muldc3.c.bc le1-llvm-none/lib/compiler-rt/muldf3.c.bc le1-llvm-none/lib/compiler-rt/muldi3.c.bc le1-llvm-none/lib/compiler-rt/mulodi4.c.bc le1-llvm-none/lib/compiler-rt/mulosi4.c.bc le1-llvm-none/lib/compiler-rt/muloti4.c.bc le1-llvm-none/lib/compiler-rt/mulsc3.c.bc le1-llvm-none/lib/compiler-rt/mulsf3.c.bc le1-llvm-none/lib/compiler-rt/multi3.c.bc le1-llvm-none/lib/compiler-rt/mulvdi3.c.bc le1-llvm-none/lib/compiler-rt/mulvsi3.c.bc le1-llvm-none/lib/compiler-rt/mulvti3.c.bc le1-llvm-none/lib/compiler-rt/mulxc3.c.bc le1-llvm-none/lib/compiler-rt/negdf2.c.bc le1-llvm-none/lib/compiler-rt/negdi2.c.bc le1-llvm-none/lib/compiler-rt/negsf2.c.bc le1-llvm-none/lib/compiler-rt/negti2.c.bc le1-llvm-none/lib/compiler-rt/negvdi2.c.bc le1-llvm-none/lib/compiler-rt/negvsi2.c.bc le1-llvm-none/lib/compiler-rt/negvti2.c.bc le1-llvm-none/lib/compiler-rt/paritydi2.c.bc le1-llvm-none/lib/compiler-rt/paritysi2.c.bc le1-llvm-none/lib/compiler-rt/parityti2.c.bc le1-llvm-none/lib/compiler-rt/popcountdi2.c.bc le1-llvm-none/lib/compiler-rt/popcountsi2.c.bc le1-llvm-none/lib/compiler-rt/popcountti2.c.bc le1-llvm-none/lib/compiler-rt/powidf2.c.bc le1-llvm-none/lib/compiler-rt/powisf2.c.bc le1-llvm-none/lib/compiler-rt/powitf2.c.bc le1-llvm-none/lib/compiler-rt/powixf2.c.bc le1-llvm-none/lib/compiler-rt/subdf3.c.bc le1-llvm-none/lib/compiler-rt/subsf3.c.bc le1-llvm-none/lib/compiler-rt/subvdi3.c.bc le1-llvm-none/lib/compiler-rt/subvsi3.c.bc le1-llvm-none/lib/compiler-rt/subvti3.c.bc le1-llvm-none/lib/compiler-rt/trampoline_setup.c.bc le1-llvm-none/lib/compiler-rt/truncdfsf2.c.bc le1-llvm-none/lib/compiler-rt/ucmpdi2.c.bc le1-llvm-none/lib/compiler-rt/ucmpti2.c.bc le1-llvm-none/lib/compiler-rt/udivdi3.c.bc le1-llvm-none/lib/compiler-rt/udivmoddi4.c.bc le1-llvm-none/lib/compiler-rt/udivmodsi4.c.bc le1-llvm-none/lib/compiler-rt/udivmodti4.c.bc le1-llvm-none/lib/compiler-rt/udivsi3.c.bc le1-llvm-none/lib/compiler-rt/udivti3.c.bc le1-llvm-none/lib/compiler-rt/umoddi3.c.bc le1-llvm-none/lib/compiler-rt/umodsi3.c.bc le1-llvm-none/lib/compiler-rt/umodti3.c.bc generic/lib/newlib/libm/newlib.bc
	@echo LLVM-LINK le1-llvm-none/lib/builtins.link.bc
	$(Verb) /opt/esdg-opencl/bin/llvm-link -o le1-llvm-none/lib/builtins.link.bc le1-llvm-none/lib/synchronization/barrier.cl.bc le1-llvm-none/lib/workitem/get_global_size.cl.bc le1-llvm-none/lib/workitem/get_group_id.cl.bc le1-llvm-none/lib/workitem/get_local_id.cl.bc le1-llvm-none/lib/workitem/get_global_id.cl.bc le1-llvm-none/lib/workitem/get_local_size.cl.bc le1-llvm-none/lib/workitem/get_num_groups.cl.bc le1-llvm-none/lib/workitem/get_global_offset.cl.bc le1-llvm-none/lib/convert.cl.bc le1-llvm-none/lib/geometric/cross.cl.bc le1-llvm-none/lib/geometric/dot.cl.bc le1-llvm-none/lib/geometric/length.cl.bc le1-llvm-none/lib/geometric/normalize.cl.bc le1-llvm-none/lib/integer/abs.cl.bc le1-llvm-none/lib/integer/add_sat.cl.bc le1-llvm-none/lib/integer/add_sat.ll.bc le1-llvm-none/lib/integer/add_sat_impl.ll.bc le1-llvm-none/lib/integer/sub_sat.cl.bc le1-llvm-none/lib/integer/sub_sat.ll.bc le1-llvm-none/lib/integer/sub_sat_impl.ll.bc le1-llvm-none/lib/math/hypot.cl.bc le1-llvm-none/lib/math/mad.cl.bc le1-llvm-none/lib/relational/any.cl.bc le1-llvm-none/lib/compiler-rt/absvdi2.c.bc le1-llvm-none/lib/compiler-rt/absvsi2.c.bc le1-llvm-none/lib/compiler-rt/absvti2.c.bc le1-llvm-none/lib/compiler-rt/adddf3.c.bc le1-llvm-none/lib/compiler-rt/addsf3.c.bc le1-llvm-none/lib/compiler-rt/addvdi3.c.bc le1-llvm-none/lib/compiler-rt/addvsi3.c.bc le1-llvm-none/lib/compiler-rt/addvti3.c.bc le1-llvm-none/lib/compiler-rt/apple_versioning.c.bc le1-llvm-none/lib/compiler-rt/ashldi3.c.bc le1-llvm-none/lib/compiler-rt/ashlti3.c.bc le1-llvm-none/lib/compiler-rt/ashrdi3.c.bc le1-llvm-none/lib/compiler-rt/ashrti3.c.bc le1-llvm-none/lib/compiler-rt/clear_cache.c.bc le1-llvm-none/lib/compiler-rt/clzdi2.c.bc le1-llvm-none/lib/compiler-rt/clzsi2.c.bc le1-llvm-none/lib/compiler-rt/clzti2.c.bc le1-llvm-none/lib/compiler-rt/cmpdi2.c.bc le1-llvm-none/lib/compiler-rt/cmpti2.c.bc le1-llvm-none/lib/compiler-rt/comparedf2.c.bc le1-llvm-none/lib/compiler-rt/comparesf2.c.bc le1-llvm-none/lib/compiler-rt/ctzdi2.c.bc le1-llvm-none/lib/compiler-rt/ctzsi2.c.bc le1-llvm-none/lib/compiler-rt/ctzti2.c.bc le1-llvm-none/lib/compiler-rt/divdc3.c.bc le1-llvm-none/lib/compiler-rt/divdf3.c.bc le1-llvm-none/lib/compiler-rt/divdi3.c.bc le1-llvm-none/lib/compiler-rt/divmoddi4.c.bc le1-llvm-none/lib/compiler-rt/divmodsi4.c.bc le1-llvm-none/lib/compiler-rt/divsc3.c.bc le1-llvm-none/lib/compiler-rt/divsf3.c.bc le1-llvm-none/lib/compiler-rt/divsi3.c.bc le1-llvm-none/lib/compiler-rt/divti3.c.bc le1-llvm-none/lib/compiler-rt/divxc3.c.bc le1-llvm-none/lib/compiler-rt/enable_execute_stack.c.bc le1-llvm-none/lib/compiler-rt/eprintf.c.bc le1-llvm-none/lib/compiler-rt/extendsfdf2.c.bc le1-llvm-none/lib/compiler-rt/ffsdi2.c.bc le1-llvm-none/lib/compiler-rt/ffsti2.c.bc le1-llvm-none/lib/compiler-rt/fixdfdi.c.bc le1-llvm-none/lib/compiler-rt/fixdfsi.c.bc le1-llvm-none/lib/compiler-rt/fixdfti.c.bc le1-llvm-none/lib/compiler-rt/fixsfdi.c.bc le1-llvm-none/lib/compiler-rt/fixsfsi.c.bc le1-llvm-none/lib/compiler-rt/fixsfti.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfti.c.bc le1-llvm-none/lib/compiler-rt/fixunssfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunssfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunssfti.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfti.c.bc le1-llvm-none/lib/compiler-rt/fixxfdi.c.bc le1-llvm-none/lib/compiler-rt/fixxfti.c.bc le1-llvm-none/lib/compiler-rt/floatdidf.c.bc le1-llvm-none/lib/compiler-rt/floatdisf.c.bc le1-llvm-none/lib/compiler-rt/floatdixf.c.bc le1-llvm-none/lib/compiler-rt/floatsidf.c.bc le1-llvm-none/lib/compiler-rt/floatsisf.c.bc le1-llvm-none/lib/compiler-rt/floattidf.c.bc le1-llvm-none/lib/compiler-rt/floattisf.c.bc le1-llvm-none/lib/compiler-rt/floattixf.c.bc le1-llvm-none/lib/compiler-rt/floatundidf.c.bc le1-llvm-none/lib/compiler-rt/floatundisf.c.bc le1-llvm-none/lib/compiler-rt/floatundixf.c.bc le1-llvm-none/lib/compiler-rt/floatunsidf.c.bc le1-llvm-none/lib/compiler-rt/floatunsisf.c.bc le1-llvm-none/lib/compiler-rt/floatuntidf.c.bc le1-llvm-none/lib/compiler-rt/floatuntisf.c.bc le1-llvm-none/lib/compiler-rt/floatuntixf.c.bc le1-llvm-none/lib/compiler-rt/gcc_personality_v0.c.bc le1-llvm-none/lib/compiler-rt/int_util.c.bc le1-llvm-none/lib/compiler-rt/lshrdi3.c.bc le1-llvm-none/lib/compiler-rt/lshrti3.c.bc le1-llvm-none/lib/compiler-rt/moddi3.c.bc le1-llvm-none/lib/compiler-rt/modsi3.c.bc le1-llvm-none/lib/compiler-rt/modti3.c.bc le1-llvm-none/lib/compiler-rt/muldc3.c.bc le1-llvm-none/lib/compiler-rt/muldf3.c.bc le1-llvm-none/lib/compiler-rt/muldi3.c.bc le1-llvm-none/lib/compiler-rt/mulodi4.c.bc le1-llvm-none/lib/compiler-rt/mulosi4.c.bc le1-llvm-none/lib/compiler-rt/muloti4.c.bc le1-llvm-none/lib/compiler-rt/mulsc3.c.bc le1-llvm-none/lib/compiler-rt/mulsf3.c.bc le1-llvm-none/lib/compiler-rt/multi3.c.bc le1-llvm-none/lib/compiler-rt/mulvdi3.c.bc le1-llvm-none/lib/compiler-rt/mulvsi3.c.bc le1-llvm-none/lib/compiler-rt/mulvti3.c.bc le1-llvm-none/lib/compiler-rt/mulxc3.c.bc le1-llvm-none/lib/compiler-rt/negdf2.c.bc le1-llvm-none/lib/compiler-rt/negdi2.c.bc le1-llvm-none/lib/compiler-rt/negsf2.c.bc le1-llvm-none/lib/compiler-rt/negti2.c.bc le1-llvm-none/lib/compiler-rt/negvdi2.c.bc le1-llvm-none/lib/compiler-rt/negvsi2.c.bc le1-llvm-none/lib/compiler-rt/negvti2.c.bc le1-llvm-none/lib/compiler-rt/paritydi2.c.bc le1-llvm-none/lib/compiler-rt/paritysi2.c.bc le1-llvm-none/lib/compiler-rt/parityti2.c.bc le1-llvm-none/lib/compiler-rt/popcountdi2.c.bc le1-llvm-none/lib/compiler-rt/popcountsi2.c.bc le1-llvm-none/lib/compiler-rt/popcountti2.c.bc le1-llvm-none/lib/compiler-rt/powidf2.c.bc le1-llvm-none/lib/compiler-rt/powisf2.c.bc le1-llvm-none/lib/compiler-rt/powitf2.c.bc le1-llvm-none/lib/compiler-rt/powixf2.c.bc le1-llvm-none/lib/compiler-rt/subdf3.c.bc le1-llvm-none/lib/compiler-rt/subsf3.c.bc le1-llvm-none/lib/compiler-rt/subvdi3.c.bc le1-llvm-none/lib/compiler-rt/subvsi3.c.bc le1-llvm-none/lib/compiler-rt/subvti3.c.bc le1-llvm-none/lib/compiler-rt/trampoline_setup.c.bc le1-llvm-none/lib/compiler-rt/truncdfsf2.c.bc le1-llvm-none/lib/compiler-rt/ucmpdi2.c.bc le1-llvm-none/lib/compiler-rt/ucmpti2.c.bc le1-llvm-none/lib/compiler-rt/udivdi3.c.bc le1-llvm-none/lib/compiler-rt/udivmoddi4.c.bc le1-llvm-none/lib/compiler-rt/udivmodsi4.c.bc le1-llvm-none/lib/compiler-rt/udivmodti4.c.bc le1-llvm-none/lib/compiler-rt/udivsi3.c.bc le1-llvm-none/lib/compiler-rt/udivti3.c.bc le1-llvm-none/lib/compiler-rt/umoddi3.c.bc le1-llvm-none/lib/compiler-rt/umodsi3.c.bc le1-llvm-none/lib/compiler-rt/umodti3.c.bc generic/lib/newlib/libm/newlib.bc

le1-llvm-none/lib/builtins.opt.bc: le1-llvm-none/lib/builtins.link.bc
	@echo OPT le1-llvm-none/lib/builtins.opt.bc
	$(Verb) /opt/esdg-opencl/bin/opt -targetlibinfo -no-aa -tbaa -basicaa -globalopt -ipsccp -deadargelim -instcombine -simplifycfg -basiccg -prune-eh -inline -functionattrs -argpromotion -sroa -domtree -early-cse -simplify-libcalls -lazy-value-info -jump-threading -correlated-propagation -simplifycfg -instcombine -simplifycfg -reassociate -domtree -loops -loop-simplify -lcssa -loop-rotate -licm -lcssa -loop-unswitch -instcombine -scalar-evolution -loop-simplify -lcssa -indvars -loop-idiom -loop-deletion -loop-unroll -memdep -gvn -memdep -memcpyopt -sccp -instcombine -lazy-value-info -jump-threading -correlated-propagation -domtree -memdep -dse -adce -simplifycfg -instcombine -strip-dead-prototypes -globaldce -constmerge -preverify -domtree -verify -o le1-llvm-none/lib/builtins.opt.bc le1-llvm-none/lib/builtins.link.bc

le1-llvm-none/lib/builtins.bc: le1-llvm-none/lib/builtins.opt.bc utils/prepare-builtins
	@echo PREPARE-BUILTINS le1-llvm-none/lib/builtins.bc
	$(Verb) utils/prepare-builtins -o le1-llvm-none/lib/builtins.bc le1-llvm-none/lib/builtins.opt.bc

Makefile: ./configure.py ./build/metabuild.py ./build/ninja_syntax.py ./generic/lib/SOURCES ./le1/lib/SOURCES
	@echo CONFIGURE
	$(Verb) ./configure.py

all:: le1-llvm-none/lib/builtins.bc

clean: 
	rm -f utils/prepare-builtins.o utils/prepare-builtins le1-llvm-none/lib/synchronization/barrier.cl.bc le1-llvm-none/lib/workitem/get_global_size.cl.bc le1-llvm-none/lib/workitem/get_group_id.cl.bc le1-llvm-none/lib/workitem/get_local_id.cl.bc le1-llvm-none/lib/workitem/get_global_id.cl.bc le1-llvm-none/lib/workitem/get_local_size.cl.bc le1-llvm-none/lib/workitem/get_num_groups.cl.bc le1-llvm-none/lib/workitem/get_global_offset.cl.bc le1-llvm-none/lib/convert.cl.bc le1-llvm-none/lib/geometric/cross.cl.bc le1-llvm-none/lib/geometric/dot.cl.bc le1-llvm-none/lib/geometric/length.cl.bc le1-llvm-none/lib/geometric/normalize.cl.bc le1-llvm-none/lib/integer/abs.cl.bc le1-llvm-none/lib/integer/add_sat.cl.bc le1-llvm-none/lib/integer/add_sat.ll.bc le1-llvm-none/lib/integer/add_sat_impl.ll.bc le1-llvm-none/lib/integer/sub_sat.cl.bc le1-llvm-none/lib/integer/sub_sat.ll.bc le1-llvm-none/lib/integer/sub_sat_impl.ll.bc le1-llvm-none/lib/math/hypot.cl.bc le1-llvm-none/lib/math/mad.cl.bc le1-llvm-none/lib/relational/any.cl.bc le1-llvm-none/lib/compiler-rt/absvdi2.c.bc le1-llvm-none/lib/compiler-rt/absvsi2.c.bc le1-llvm-none/lib/compiler-rt/absvti2.c.bc le1-llvm-none/lib/compiler-rt/adddf3.c.bc le1-llvm-none/lib/compiler-rt/addsf3.c.bc le1-llvm-none/lib/compiler-rt/addvdi3.c.bc le1-llvm-none/lib/compiler-rt/addvsi3.c.bc le1-llvm-none/lib/compiler-rt/addvti3.c.bc le1-llvm-none/lib/compiler-rt/apple_versioning.c.bc le1-llvm-none/lib/compiler-rt/ashldi3.c.bc le1-llvm-none/lib/compiler-rt/ashlti3.c.bc le1-llvm-none/lib/compiler-rt/ashrdi3.c.bc le1-llvm-none/lib/compiler-rt/ashrti3.c.bc le1-llvm-none/lib/compiler-rt/clear_cache.c.bc le1-llvm-none/lib/compiler-rt/clzdi2.c.bc le1-llvm-none/lib/compiler-rt/clzsi2.c.bc le1-llvm-none/lib/compiler-rt/clzti2.c.bc le1-llvm-none/lib/compiler-rt/cmpdi2.c.bc le1-llvm-none/lib/compiler-rt/cmpti2.c.bc le1-llvm-none/lib/compiler-rt/comparedf2.c.bc le1-llvm-none/lib/compiler-rt/comparesf2.c.bc le1-llvm-none/lib/compiler-rt/ctzdi2.c.bc le1-llvm-none/lib/compiler-rt/ctzsi2.c.bc le1-llvm-none/lib/compiler-rt/ctzti2.c.bc le1-llvm-none/lib/compiler-rt/divdc3.c.bc le1-llvm-none/lib/compiler-rt/divdf3.c.bc le1-llvm-none/lib/compiler-rt/divdi3.c.bc le1-llvm-none/lib/compiler-rt/divmoddi4.c.bc le1-llvm-none/lib/compiler-rt/divmodsi4.c.bc le1-llvm-none/lib/compiler-rt/divsc3.c.bc le1-llvm-none/lib/compiler-rt/divsf3.c.bc le1-llvm-none/lib/compiler-rt/divsi3.c.bc le1-llvm-none/lib/compiler-rt/divti3.c.bc le1-llvm-none/lib/compiler-rt/divxc3.c.bc le1-llvm-none/lib/compiler-rt/enable_execute_stack.c.bc le1-llvm-none/lib/compiler-rt/eprintf.c.bc le1-llvm-none/lib/compiler-rt/extendsfdf2.c.bc le1-llvm-none/lib/compiler-rt/ffsdi2.c.bc le1-llvm-none/lib/compiler-rt/ffsti2.c.bc le1-llvm-none/lib/compiler-rt/fixdfdi.c.bc le1-llvm-none/lib/compiler-rt/fixdfsi.c.bc le1-llvm-none/lib/compiler-rt/fixdfti.c.bc le1-llvm-none/lib/compiler-rt/fixsfdi.c.bc le1-llvm-none/lib/compiler-rt/fixsfsi.c.bc le1-llvm-none/lib/compiler-rt/fixsfti.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunsdfti.c.bc le1-llvm-none/lib/compiler-rt/fixunssfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunssfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunssfti.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfdi.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfsi.c.bc le1-llvm-none/lib/compiler-rt/fixunsxfti.c.bc le1-llvm-none/lib/compiler-rt/fixxfdi.c.bc le1-llvm-none/lib/compiler-rt/fixxfti.c.bc le1-llvm-none/lib/compiler-rt/floatdidf.c.bc le1-llvm-none/lib/compiler-rt/floatdisf.c.bc le1-llvm-none/lib/compiler-rt/floatdixf.c.bc le1-llvm-none/lib/compiler-rt/floatsidf.c.bc le1-llvm-none/lib/compiler-rt/floatsisf.c.bc le1-llvm-none/lib/compiler-rt/floattidf.c.bc le1-llvm-none/lib/compiler-rt/floattisf.c.bc le1-llvm-none/lib/compiler-rt/floattixf.c.bc le1-llvm-none/lib/compiler-rt/floatundidf.c.bc le1-llvm-none/lib/compiler-rt/floatundisf.c.bc le1-llvm-none/lib/compiler-rt/floatundixf.c.bc le1-llvm-none/lib/compiler-rt/floatunsidf.c.bc le1-llvm-none/lib/compiler-rt/floatunsisf.c.bc le1-llvm-none/lib/compiler-rt/floatuntidf.c.bc le1-llvm-none/lib/compiler-rt/floatuntisf.c.bc le1-llvm-none/lib/compiler-rt/floatuntixf.c.bc le1-llvm-none/lib/compiler-rt/gcc_personality_v0.c.bc le1-llvm-none/lib/compiler-rt/int_util.c.bc le1-llvm-none/lib/compiler-rt/lshrdi3.c.bc le1-llvm-none/lib/compiler-rt/lshrti3.c.bc le1-llvm-none/lib/compiler-rt/moddi3.c.bc le1-llvm-none/lib/compiler-rt/modsi3.c.bc le1-llvm-none/lib/compiler-rt/modti3.c.bc le1-llvm-none/lib/compiler-rt/muldc3.c.bc le1-llvm-none/lib/compiler-rt/muldf3.c.bc le1-llvm-none/lib/compiler-rt/muldi3.c.bc le1-llvm-none/lib/compiler-rt/mulodi4.c.bc le1-llvm-none/lib/compiler-rt/mulosi4.c.bc le1-llvm-none/lib/compiler-rt/muloti4.c.bc le1-llvm-none/lib/compiler-rt/mulsc3.c.bc le1-llvm-none/lib/compiler-rt/mulsf3.c.bc le1-llvm-none/lib/compiler-rt/multi3.c.bc le1-llvm-none/lib/compiler-rt/mulvdi3.c.bc le1-llvm-none/lib/compiler-rt/mulvsi3.c.bc le1-llvm-none/lib/compiler-rt/mulvti3.c.bc le1-llvm-none/lib/compiler-rt/mulxc3.c.bc le1-llvm-none/lib/compiler-rt/negdf2.c.bc le1-llvm-none/lib/compiler-rt/negdi2.c.bc le1-llvm-none/lib/compiler-rt/negsf2.c.bc le1-llvm-none/lib/compiler-rt/negti2.c.bc le1-llvm-none/lib/compiler-rt/negvdi2.c.bc le1-llvm-none/lib/compiler-rt/negvsi2.c.bc le1-llvm-none/lib/compiler-rt/negvti2.c.bc le1-llvm-none/lib/compiler-rt/paritydi2.c.bc le1-llvm-none/lib/compiler-rt/paritysi2.c.bc le1-llvm-none/lib/compiler-rt/parityti2.c.bc le1-llvm-none/lib/compiler-rt/popcountdi2.c.bc le1-llvm-none/lib/compiler-rt/popcountsi2.c.bc le1-llvm-none/lib/compiler-rt/popcountti2.c.bc le1-llvm-none/lib/compiler-rt/powidf2.c.bc le1-llvm-none/lib/compiler-rt/powisf2.c.bc le1-llvm-none/lib/compiler-rt/powitf2.c.bc le1-llvm-none/lib/compiler-rt/powixf2.c.bc le1-llvm-none/lib/compiler-rt/subdf3.c.bc le1-llvm-none/lib/compiler-rt/subsf3.c.bc le1-llvm-none/lib/compiler-rt/subvdi3.c.bc le1-llvm-none/lib/compiler-rt/subvsi3.c.bc le1-llvm-none/lib/compiler-rt/subvti3.c.bc le1-llvm-none/lib/compiler-rt/trampoline_setup.c.bc le1-llvm-none/lib/compiler-rt/truncdfsf2.c.bc le1-llvm-none/lib/compiler-rt/ucmpdi2.c.bc le1-llvm-none/lib/compiler-rt/ucmpti2.c.bc le1-llvm-none/lib/compiler-rt/udivdi3.c.bc le1-llvm-none/lib/compiler-rt/udivmoddi4.c.bc le1-llvm-none/lib/compiler-rt/udivmodsi4.c.bc le1-llvm-none/lib/compiler-rt/udivmodti4.c.bc le1-llvm-none/lib/compiler-rt/udivsi3.c.bc le1-llvm-none/lib/compiler-rt/udivti3.c.bc le1-llvm-none/lib/compiler-rt/umoddi3.c.bc le1-llvm-none/lib/compiler-rt/umodsi3.c.bc le1-llvm-none/lib/compiler-rt/umodti3.c.bc le1-llvm-none/lib/builtins.link.bc le1-llvm-none/lib/builtins.opt.bc le1-llvm-none/lib/builtins.bc

distclean: clean
	rm -f Makefile
