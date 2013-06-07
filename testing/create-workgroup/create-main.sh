# compile-complete.sh kernel mcpu num_kernel_args <list>kernel_args

kernel_name=$1
mcpu=$2
#num_args=$3
args=""

count=0
# Get the data addresses which are the kernel arguments
for arg in "$@"
  do
    if echo "$arg" | grep "0x"
      then
        args[$count]="${arg}"
        ((count++))
    fi
  done

# Create main function to launch the kernel
file="main.c"
echo "int main(void) {" >> $file
echo -n "  ${kernel_name}(" >> $file
i=0;
#let "end=$num_args-1"
let "end=${#args[@]}-1"
echo $end
while((i<$end));
  do
    echo -n "${args[$i]}, " >> $file
    let i++
  done
echo "${args[$end]});" >> $file
echo "}" >> $file
