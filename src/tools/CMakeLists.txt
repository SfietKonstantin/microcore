project(microgen)

set(${PROJECT_NAME}_SRCS
    microgen.py
    bean.h.tpl
    bean.cpp.tpl
    beanobject.h.tpl
    beanobject.cpp.tpl
    types.h.tpl
    factory.h.tpl
    factoryjson.cpp.tpl
)

include(cmake/MicrogenMacros.cmake)
