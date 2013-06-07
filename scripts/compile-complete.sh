# Script to take an OpenCL kernel, compile it into LLVM bytecode and then
# replicate several work-items into a single instance and produce assembly
# output for the LE1. This is then linked with a main function that is
# used as a launcher

# compile-complete.sh kernel mcpu <list>kernel_args

kernel_name=$1
config=$2
args=""

count=0
for arg in "$@"
  do
    if echo "$arg" | grep "0x"
      then
        args[$count]="${arg}"
        ((count++))
    fi
  done

lib_dir=/home/sam/src/le1-opencl-compiler/libclc
kernel_bc=kernel.bc

# Compile the .cl kernel file and link with libclc
clang -ccc-host-triple le1-llvm-none -emit-llvm -c \
      -I"${lib_dir}/le1/include" \
      -I"${lib_dir}/generic/include" \
      -Xclang -mlink-bitcode-file \
      -Xclang "${lib_dir}/le1-llvm-none/lib/builtins.bc" \
      -include clc/clc.h -Dcl_clang_storage_class_specifiers "$kernel_name.cl" \
      -o ${kernel_bc}

# Merge kernel instances
pocl_lib=/usr/local/lib/pocl/llvmopencl.so
size_x=3
size_y=1
size_z=1
merged_bc="merged_kernel.bc"

/usr/local/bin/opt -load ${pocl_lib} -flatten -always-inline \
    -globaldce -simplifycfg -loop-simplify -isolate-regions -loop-barriers \
    -barriertails -barriers -isolate-regions -add-wi-metadata -wi-aa -workitem \
    -kernel=${kernel} -local-size=${size_x} ${size_y} ${size_z} \
    -o ${merged_bc} ${kernel_bc}


# Create main function to launch the kernel
launcher="main.c"
# This are the pointers to the kernel attributes, used by work-item intrinsics
#echo "int* work_dim = (int*)0x0;" >> $launcher      #0
#echo "int* global_size = (int*)0x4;" >> $launcher   #4
#echo "int* local_size = (int*)0x10;" >> $launcher   #16
#echo "int* num_groups = (int*)0x1C;" >> $launcher   #28
#echo "int* global_offset = (int*)0x28;" >> $launcher #40
echo "int work_dim = 2;" >> $launcher      #0
echo "int global_size[3] = {1024, 1024};" >> $launcher   #4
echo "int local_size[3] = {256, 256};" >> $launcher   #16
echo "int num_groups[3] = {4, 4};" >> $launcher   #28
echo "int global_offset[3] = {0, 0, 0};" >> $launcher #40

echo "int main(void) {" >> $launcher
echo -n "  ${kernel_name}(" >> $launcher
i=0;
let "end=${#args[@]}-1"
echo $end
while (( i<$end ));
  do
    echo -n "${args[$i]}, " >> $launcher
    let i++
  done
echo "${args[$end]});" >> $launcher
echo "}" >> $launcher

launcher_bc="launcher.bc"
final_bc="$kernel_name.bc"

# Compile and link launcher function with the merged kernel
clang -ccc-host-triple le1-llvm-none -emit-llvm -c ${launcher} -o ${launcher_bc}
llvm-link ${launcher_bc} ${merged_bc} -o ${final_bc}

# Compile into assembly for the LE1
scheduler="-post-RA-scheduler=true"
opts="-O3"
target="-march=le1 -mcpu=${config}"
llc ${target} ${scheduler} ${opts} ${final_bc} -o "${kernel_name}.s"


