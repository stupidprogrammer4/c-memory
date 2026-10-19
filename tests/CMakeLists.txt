foreach(test_name IN ITEMS allocator arena pool slab tracker stress contracts)
    add_executable(${test_name}_test unit/${test_name}_test.c)
    target_link_libraries(${test_name}_test PRIVATE CMemory::c_memory)
    add_test(NAME ${test_name} COMMAND ${test_name}_test)
endforeach()
add_executable(version_test version_test.c)
target_link_libraries(version_test PRIVATE CMemory::c_memory)
add_test(NAME version COMMAND version_test)

# Sanitized libraries are development artifacts; validate the installable
# package in ordinary Debug/Release builds with a fresh external consumer.
if(NOT CMEMORY_ENABLE_SANITIZERS)
    add_test(NAME installed_consumer
        COMMAND ${CMAKE_COMMAND}
            "-DPROJECT_BUILD_DIR=${PROJECT_BINARY_DIR}"
            "-DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/integration/consumer"
            "-DTEST_CONFIG=$<CONFIG>"
            "-DTEST_CTEST=${CMAKE_CTEST_COMMAND}"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/integration/installed_consumer.cmake)
endif()
