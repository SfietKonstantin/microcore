project(microgen)

set(${PROJECT_NAME}_SRCS
    microgen.py
    transformer.py
    exception.py
    bean.h.tpl
    bean-nested.h.tpl
    bean.cpp.tpl
    bean-nested.cpp.tpl
    beanobject.h.tpl
    beanobject-nested.h.tpl
    beanobject.cpp.tpl
    beanobject-nested.cpp.tpl
    typesjson.h.tpl
    factory.h.tpl
    factoryjson.cpp.tpl
    factorylistjson.cpp.tpl
)

add_custom_target(${PROJECT_NAME}
    DEPENDS ${${PROJECT_NAME}_SRCS}
    SOURCES ${${PROJECT_NAME}_SRCS}
    COMMENT Prepare microgen
)

include(cmake/MicrogenMacros.cmake)
