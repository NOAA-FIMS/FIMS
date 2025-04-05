cmake_minimum_required(VERSION 3.10)
project(FIMS_Testing)

# Enable C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Enable coverage flags
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Add test executable
add_executable(test_suite test_main.cpp)  # Replace with your test files
target_link_libraries(test_suite GTest::GTest GTest::Main)

# Enable coverage reporting with gcov and lcov
if(CMAKE_BUILD_TYPE MATCHES Debug)
    find_program(LCOV lcov)
    find_program(GCOV gcov)

    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E remove -f coverage.info
        COMMAND ${LCOV} --directory . --capture --output-file coverage.info
        COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info
        COMMAND genhtml coverage.info --output-directory coverage_report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report..."
    )
endif()
