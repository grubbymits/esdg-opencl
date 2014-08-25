include_directories (${Coal_SOURCE_DIR}/include
                    ${Coal_SOURCE_DIR}/src
                    ${LLVM_INCLUDE_DIR}
                    ${GALLIUM_INCLUDE_DIRS}
                    ${CLANG_INCLUDE_DIRS}
                    /usr/include/libxml2
                    /opt/esdg-opencl/include/
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

set(CMAKE_INSTALL_RPATH "/opt/esdg-opencl/lib")
set(CMAKE_CXX_FLAGS
" -std=c++0x -g -fno-rtti -DINSIZZLEAPI -DOPT_UNROLL -DDBG_KERNEL -DDBG_OUTPUT")

configure_file(core/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/core/config.h)

set(COAL_SRC_FILES
    api/api_command.cpp
    api/api_device.cpp
    api/api_event.cpp
    api/api_kernel.cpp
    api/api_platform.cpp
    api/api_program.cpp
    api/api_context.cpp
    api/api_enqueue.cpp
    api/api_flush.cpp
    api/api_memory.cpp
    api/api_profiling.cpp
    api/api_sampler.cpp
    api/api_gl.cpp

    core/context.cpp
    core/commandqueue.cpp
    core/embedded_data.cpp
    core/memobject.cpp
    core/events.cpp
    core/program.cpp
    core/compiler.cpp
    core/kernel.cpp
    core/sampler.cpp
    core/object.cpp

    core/devices/LE1/LE1buffer.cpp
    core/devices/LE1/LE1device.cpp
    core/devices/LE1/LE1kernel.cpp
    core/devices/LE1/LE1program.cpp
    core/devices/LE1/LE1worker.cpp
    core/devices/LE1/LE1Simulator.cpp
    core/devices/LE1/LE1DataPrinter.cpp

    core/devices/LE1/coarsener/SourceRewriter.cpp
    core/devices/LE1/coarsener/StmtFixers.cpp

    core/devices/LE1/creduce/RewriteUtils.cpp
    core/devices/LE1/creduce/Transformation.cpp
    core/devices/LE1/creduce/TransformationManager.cpp
    core/devices/LE1/creduce/SimpleInliner.cpp

    #${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.h.embed.h
    #${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.c.bc.embed.h
    #${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_impl.h
    #${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_def.h
    #${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_impl.h
    #${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_def.h
)

#add_subdirectory(runtime)

add_library(OpenCL SHARED ${COAL_SRC_FILES})

add_library(InsizzleAPI SHARED IMPORTED)
#add_library(WorkitemCoarsen SHARED IMPORTED)

#add_library(xml2 STATIC IMPORTED)
#add_library(m SHARED IMPORTED)

#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.h.embed.h
 #                           PROPERTIES GENERATED 1)
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.c.bc.embed.h
 #                           PROPERTIES GENERATED 1)
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_impl.h
 #                           PROPERTIES GENERATED 1)
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_def.h
 #                           PROPERTIES GENERATED 1)
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_impl.h
 #                           PROPERTIES GENERATED 1)
#set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_def.h
 #                           PROPERTIES GENERATED 1)

#add_dependencies(OpenCL generate_stdlib_h)
#add_dependencies(OpenCL generate_stdlib_c)
#add_dependencies(OpenCL generate_builtins)
add_dependencies(OpenCL InsizzleAPI)
#add_dependencies(OpenCL WorkitemCoarsen)

SET(LIBRARY_OUTPUT_PATH ${Coal_BINARY_DIR}/lib)

SET_TARGET_PROPERTIES(OpenCL PROPERTIES
  VERSION ${${PROJECT_NAME}_VERSION}
  SOVERSION ${${PROJECT_NAME}_SOVERSION}
)

#SET_TARGET_PROPERTIES(InsizzleAPI PROPERTIES
 # IMPORTED_LOCATION /opt/esdg-opencl/lib/libInsizzleAPI.dylib
  #LINK_FLAGS "-L/usr/lib -lm `xml2-config --libs` -lpcre")

#SET_TARGET_PROPERTIES(WorkitemCoarsen PROPERTIES
 # IMPORTED_LOCATION /opt/esdg-opencl/lib/libWorkitemCoarsen.dylib)

set_source_files_properties(${COAL_SRC_FILES}
        PROPERTIES COMPILE_FLAGS ${LLVM_COMPILE_FLAGS})

set_target_properties(OpenCL PROPERTIES
  LINK_FLAGS "${LLVM_LDFLAGS} -shared -L/opt/esdg-opencl/lib -lInsizzleAPI"
  LINK_INTERFACE_LIBRARIES "")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
SET_TARGET_PROPERTIES(InsizzleAPI PROPERTIES
  IMPORTED_LOCATION /opt/esdg-opencl/lib/libInsizzleAPI.dylib)
  #LINK_FLAGS "-static lxml2 -lpcre -lm -z -llzma")

TARGET_LINK_LIBRARIES(OpenCL
        ${CLANG_LIBS}
        ${LLVM_LIBS_CORE}
        ${LLVM_LIBS_JIT}
        pthread
        InsizzleAPI
)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
SET_TARGET_PROPERTIES(InsizzleAPI PROPERTIES
  IMPORTED_LOCATION /opt/esdg-opencl/lib/libInsizzleAPI.so)
  #LINK_FLAGS "--whole-archive libInsizzleAPI.a -W1, --no-whole-archive")

TARGET_LINK_LIBRARIES(OpenCL
        ${CLANG_LIBS}
        ${LLVM_LIBS_CORE}
        ${LLVM_LIBS_JIT}
        pthread
        rt
        dl
        InsizzleAPI
        xml2
        pcre
        m
        lz
        lzma
)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

install(TARGETS OpenCL LIBRARY DESTINATION /opt/esdg-opencl/lib)
