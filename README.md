esdg-opencl
===========

OpenCL driver and compiler for the LE1.

The driver is made from several components:
- Clover is the driver, it uses clang to parse and transform the kernels into
  coarsened bytecode files.
- The bytecode is statically linked to the OpenCL runtime library (libclc).
- The final bytecode is compiled using the LLVM backend and llc.
- And the code is run using the simulator (libInsizzle) from within Clover.

Installation:

- Install 'install-dir' to /opt/esdg-opencl/.
- Build and install LLVM
- Build clover and copy the resulting libOpenCL.so files to
  /opt/esdg-opencl/lib
- configure and make libclc, which will produce builtins.bc in the le1-llvm-none
  directory. Copy the .bc file to /opt/esdg-opencl/lib

And don't forget it doesn't really work yet!
