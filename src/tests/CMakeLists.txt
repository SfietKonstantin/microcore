project(microcore-tests)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_EXTRA_WARNINGS_FLAGS} ${CMAKE_CXX_COVERAGE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)
find_program(LCOV_EXEC lcov)

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
    mockdatasourcelistener.h
    mockmodellistener.h
    tst_main.cpp
    tst_pipe.cpp
    tst_http.cpp
    tst_datasource.cpp
    tst_modeldata.cpp
    tst_model.cpp
    tst_modeloperator.cpp
    tst_qtmodel.cpp
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

# Build mock server
find_program(NPM_EXEC npm)
find_program(FOREVER_EXEC forever)
find_program(COFFEE_EXEC coffee)

if (NPM_EXEC AND FOREVER_EXEC AND COFFEE_EXEC)
    add_definitions(-DENABLE_MOCK_SERVER)
    set(ENABLE_MOCK_SERVER True)
    set(MOCK_SERVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server)
    message("Building with mock server")
    add_custom_command(OUTPUT ${MOCK_SERVER_DIR}/node_modules
                       COMMAND ${NPM_EXEC} install
                       DEPENDS ${MOCK_SERVER_DIR}/package.json
                       WORKING_DIRECTORY ${MOCK_SERVER_DIR}
                       COMMENT "Invoking NPM")
    add_custom_target(${PROJECT_NAME}-npm
                      DEPENDS ${MOCK_SERVER_DIR}/node_modules
                      COMMENT "Built target NPM invocation")
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-npm)
    add_custom_command(OUTPUT ${MOCK_SERVER_DIR}/app.js
                       COMMAND ${COFFEE_EXEC} -c ${MOCK_SERVER_DIR}/app.coffee
                       DEPENDS ${MOCK_SERVER_DIR}/app.coffee
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                       COMMENT "Generating mock server")
    add_custom_target(${PROJECT_NAME}-mock-server
                      DEPENDS ${MOCK_SERVER_DIR}/app.js
                      COMMENT "Built target mock server generation")
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-mock-server)
endif(NPM_EXEC AND FOREVER_EXEC AND COFFEE_EXEC)

if (LCOV_EXEC)
    message("Building auto lcov cleaning")
    add_test(NAME pre COMMAND ${LCOV_EXEC} --directory ${CMAKE_BINARY_DIR} --zerocounters) # lcov clear
endif (LCOV_EXEC)
if (ENABLE_MOCK_SERVER)
    add_test(NAME start-mock-server COMMAND ${FOREVER_EXEC} start ${MOCK_SERVER_DIR}/app.js)
    add_test(NAME start-mock-server-sleep COMMAND sleep 1) # Let the server start
endif (ENABLE_MOCK_SERVER)
add_test(tests ${PROJECT_NAME})
if (ENABLE_MOCK_SERVER)
    add_test(NAME stop-mock-server COMMAND ${FOREVER_EXEC} stop ${MOCK_SERVER_DIR}/app.js)
endif (ENABLE_MOCK_SERVER)
