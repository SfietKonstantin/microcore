project(microgen)

set(${PROJECT_NAME}_SRCS
    microgen.py
    transformer.py
    exception.py
    bean.h.tpl
    bean.cpp.tpl
    beanobject.h.tpl
    beanobject.cpp.tpl
    types.h.tpl
    factory.h.tpl
    factoryjson.cpp.tpl
)

add_custom_target(${PROJECT_NAME}
    DEPENDS ${${PROJECT_NAME}_SRCS}
    SOURCES ${${PROJECT_NAME}_SRCS}
    COMMENT Prepare microgen
)

include(cmake/MicrogenMacros.cmake)
