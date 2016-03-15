find_package(PythonInterp REQUIRED)

macro(microgen_output_dir infile outdir)
    file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
    set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${rel}")
    get_filename_component(outdir ${_outfile} PATH)
    file(MAKE_DIRECTORY ${${outdir}})
endmacro()

function(microgen_bean outfiles)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    foreach(it ${ARGN})
        get_filename_component(it ${it} ABSOLUTE)
        microgen_output_dir(${it} outdir)
        get_filename_component(outfile ${_outfile} NAME_WE)
        set(hfile ${outdir}/${outfile}.h)
        set(cppfile ${outdir}/${outfile}.cpp)
        add_custom_command(OUTPUT ${hfile} ${cppfile}
                           COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/src/tools/microgen.py bean ${it} ${outdir}
                           DEPENDS ${it} ${microgen_SRCS}
                           ${CMAKE_BUILD_DIR})
        list(APPEND ${outfiles} ${hfile} ${cppfile})
    endforeach()
    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()

function(microgen_qtbean outfiles)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    foreach(it ${ARGN})
        get_filename_component(it ${it} ABSOLUTE)
        microgen_output_dir(${it} outdir)
        get_filename_component(outfile ${_outfile} NAME_WE)
        set(hfile ${outdir}/${outfile}object.h)
        set(cppfile ${outdir}/${outfile}object.cpp)
        add_custom_command(OUTPUT ${hfile} ${cppfile}
                           COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/src/tools/microgen.py qtbean ${it} ${outdir}
                           DEPENDS ${it} ${microgen_SRCS}
                           ${CMAKE_BUILD_DIR})
        qt5_wrap_cpp(mockfile ${hfile})
        list(APPEND ${outfiles} ${hfile} ${cppfile} ${mockfile})
    endforeach()
    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()

function(microgen_factory outfiles)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    foreach(it ${ARGN})
        get_filename_component(it ${it} ABSOLUTE)
        microgen_output_dir(${it} outdir)
        get_filename_component(outfile ${_outfile} NAME_WE)
        set(thfile ${outdir}/${outfile}types.h)
        set(hfile ${outdir}/${outfile}requestfactory.h)
        set(cppfile ${outdir}/${outfile}requestfactory.cpp)
        add_custom_command(OUTPUT ${thfile} ${hfile} ${cppfile}
                           COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/src/tools/microgen.py factory ${it} ${outdir}
                           DEPENDS ${it} ${microgen_SRCS}
                           ${CMAKE_BUILD_DIR})
        list(APPEND ${outfiles} ${hfile} ${cppfile})
    endforeach()
    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()
