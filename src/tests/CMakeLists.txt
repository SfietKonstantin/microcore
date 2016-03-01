project(microcore-tests)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_EXTRA_WARNINGS_FLAGS} ${CMAKE_CXX_COVERAGE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)
find_program(LCOV_PATH lcov)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${QT_INCLUDES}
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
)

set(${PROJECT_NAME}_SRCS
    mockjob.h
    mockjobfactory.h
    tst_main.cpp
    tst_datasource_simple.cpp
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SRCS}
)
qt5_use_modules(${PROJECT_NAME} Core Network Test)
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
    Qt5::Test
    gtest
    gmock
    microcore
)

add_test(NAME pre COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters) # lcov clear
add_test(tests ${PROJECT_NAME})
