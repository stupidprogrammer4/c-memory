cmake_minimum_required(VERSION 3.20)
project(CMemory VERSION 0.1.0 LANGUAGES C)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
option(CMEMORY_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(CMEMORY_BUILD_EXAMPLES "Build examples" ON)
option(CMEMORY_BUILD_BENCHMARKS "Build reproducible allocation benchmark" OFF)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()
if(CMEMORY_ENABLE_SANITIZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Sanitizer builds currently require GCC or Clang")
    endif()
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(c_memory
    src/core/version.c
    src/core/allocator.c
    src/allocators/system.c
    src/allocators/arena.c
    src/allocators/pool.c
    src/allocators/slab.c
    src/diagnostics/tracker.c)
add_library(CMemory::c_memory ALIAS c_memory)
set_target_properties(c_memory PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0.1)
target_compile_features(c_memory PUBLIC c_std_17)
target_include_directories(c_memory PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
if(CMEMORY_BUILD_EXAMPLES)
    add_executable(arena_example examples/arena.c)
    target_link_libraries(arena_example PRIVATE CMemory::c_memory)
    if(BUILD_TESTING)
        add_test(NAME arena_example COMMAND arena_example)
    endif()
endif()

if(CMEMORY_BUILD_BENCHMARKS)
    add_executable(allocation_benchmark benchmarks/allocation.c)
    target_link_libraries(allocation_benchmark PRIVATE CMemory::c_memory)
endif()

install(TARGETS c_memory EXPORT CMemoryTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/c-memory)
install(FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(EXPORT CMemoryTargets NAMESPACE CMemory:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CMemory)
configure_package_config_file(cmake/CMemoryConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CMemoryConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CMemory)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/CMemoryConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMinorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMemoryConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CMemoryConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CMemory)

set(CPACK_PACKAGE_NAME c-memory)
set(CPACK_PACKAGE_VENDOR "stupidprogrammer4")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_FILE_NAME "c-memory-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
