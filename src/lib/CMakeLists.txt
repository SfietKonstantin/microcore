project(microcore)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_EXTRA_WARNINGS_FLAGS} ${CMAKE_CXX_COVERAGE_FLAGS}")

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

set(${PROJECT_NAME}_CORE_SRCS
    include/microcore/core/globals.h
    include/microcore/core/ijob.h
    include/microcore/core/ijobfactory.h
    include/microcore/core/pipe.h
    include/microcore/core/executor.h
    include/microcore/core/listenerrepository.h
)

set(${PROJECT_NAME}_DATA_SRCS
    include/microcore/data/type_helper.h
    include/microcore/data/iindexeddatastore.h
    include/microcore/data/iitem.h
    include/microcore/data/item.h
    include/microcore/data/indexeddatastore.h
    include/microcore/data/imodel.h
    include/microcore/data/imutablemodel.h
    include/microcore/data/indexedmodel.h
)

set(${PROJECT_NAME}_QT_SRCS
    include/microcore/qt/qobjectptr.h
    include/microcore/qt/iviewmodel.h
    include/microcore/qt/viewmodel.h
    include/microcore/qt/viewcontroller.h
    src/qt/viewcontroller.cpp
    include/microcore/qt/viewmodelcontroller.h
    include/microcore/qt/iviewitem.h
    include/microcore/qt/viewitem.h
    include/microcore/qt/viewitemcontroller.h
)

set(${PROJECT_NAME}_ERROR_SRCS
    include/microcore/error/error.h
    src/error/error.cpp
)

set(${PROJECT_NAME}_HTTP_SRCS
    include/microcore/http/httptypes.h
    include/microcore/http/httprequest.h
    src/http/httprequest.cpp
    include/microcore/http/httprequestfactory.h
    src/http/httprequestfactory.cpp
)

set(${PROJECT_NAME}_JSON_SRCS
    include/microcore/json/jsontypes.h
    include/microcore/json/jsonrequestfactory.h
    src/json/jsonrequestfactory.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_CORE_SRCS}
    ${${PROJECT_NAME}_DATA_SRCS}
    ${${PROJECT_NAME}_QT_SRCS}
    ${${PROJECT_NAME}_ERROR_SRCS}
    ${${PROJECT_NAME}_HTTP_SRCS}
    ${${PROJECT_NAME}_JSON_SRCS}
)
qt5_use_modules(${PROJECT_NAME} Core Network Qml)
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
    Qt5::Qml
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include
    ${PROJECT_SOURCE_DIR}
)
