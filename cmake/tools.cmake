# CMake tools for project
cmake_minimum_required(VERSION 3.11)

# collect source files under folder and automatically generate source group in IDE
function(func_collect_source_files sourceFiles sourceDir)
    function(PopulateSourceFolder sourceDir allSources sourceGroupFolder)
        function(CreateSourceGroups groupStart sources root)
            foreach(CURRENT_FILE ${sources})
                get_filename_component(DIR_PATH ${CURRENT_FILE} DIRECTORY)
                file(RELATIVE_PATH SOURCE_GROUP_PATH ${root} ${DIR_PATH})
                if(SOURCE_GROUP_PATH)
                    string(REPLACE "/" "\\" SOURCE_GROUP_PATH ${SOURCE_GROUP_PATH})
                    string(REPLACE "\\" "\\\\" SOURCE_GROUP_PATH ${SOURCE_GROUP_PATH})
                endif()
                source_group("${groupStart}\\${SOURCE_GROUP_PATH}" FILES ${CURRENT_FILE})
            endforeach(CURRENT_FILE)
        endfunction(CreateSourceGroups)

        file(GLOB_RECURSE CURRENT_SOURCES
                ${sourceDir}/*.cpp
                ${sourceDir}/*.c
                ${sourceDir}/*.cc)
        file(GLOB_RECURSE CURRENT_HEADERS
                ${sourceDir}/*.h
                ${sourceDir}/*.hpp)
        set(${allSources} ${CURRENT_SOURCES} ${CURRENT_HEADERS} PARENT_SCOPE)

        CreateSourceGroups("${sourceGroupFolder}\\Sources" "${CURRENT_SOURCES}" ${sourceDir})
        CreateSourceGroups("${sourceGroupFolder}\\Headers" "${CURRENT_HEADERS}" ${sourceDir})
    endfunction(PopulateSourceFolder)

    # try to collect files by typical structure
    PopulateSourceFolder(${sourceDir}/include API_HEADER "Api")
    PopulateSourceFolder(${sourceDir}/src CURRENT_SRC "")
    set(${sourceFiles} ${API_HEADER} ${CURRENT_SRC} PARENT_SCOPE)
    if(NOT API_HEADER AND NOT CURRENT_SRC)
        # try to collect files directly
        PopulateSourceFolder(${sourceDir} ALL_SRC "")
        set(${sourceFiles} ${ALL_SRC} PARENT_SCOPE)
    endif()
endfunction(func_collect_source_files)

# Auto format code when build target
function(func_auto_format_code targetName sources...)
    if(NOT TARGET FastFormat)
        find_program(fastFormatter git-clang-format)
        if(fastFormatter)
            add_custom_target(FastFormat
                    COMMAND python ${fastFormatter} -f --style file ./**
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "Fast format")
        else()
            add_custom_target(FastFormat
                    COMMENT "Empty fast format")
        endif()
    endif()
    add_dependencies(${targetName} FastFormat)

    # collect all format source files
    set(FORMAT_SOURCE_FILES ${ARGV1})
    foreach(arg IN LISTS ARGN)
        set(FORMAT_SOURCE_FILES ${FORMAT_SOURCE_FILES} ${arg})
    endforeach()

    get_property(allFormatFiles GLOBAL PROPERTY ALL_FORMAT_SOURCE_FILES)
    set(allFormatFiles ${allFormatFiles} ${FORMAT_SOURCE_FILES})
    set_property(GLOBAL PROPERTY ALL_FORMAT_SOURCE_FILES ${allFormatFiles})
endfunction(func_auto_format_code)

function(func_link_libraries targetName libs...)
    set(ALL_LIBS "")
    list(APPEND ALL_LIBS ${ARGV1} ${ARGN})

    func_target_link_libraries(${targetName} "" ${ALL_LIBS})
endfunction(func_link_libraries)

function(func_target_link_libraries targetName keyword libs...)
    set(ALL_LIBS "")
    list(APPEND ALL_LIBS ${ARGV2} ${ARGN})

    if (ALL_LIBS)
        list(REVERSE ALL_LIBS)
        list(REMOVE_DUPLICATES ALL_LIBS)
        list(REVERSE ALL_LIBS)

        message(STATUS "${targetName} link libraries: ${ALL_LIBS}")
        target_link_libraries(${targetName} ${keyword} ${ALL_LIBS})
    endif()

    if (WIN32)
        # Copy .dll files alongside .exe for execution.
        get_target_property(targetType ${targetName} TYPE)
        if (targetType STREQUAL "EXECUTABLE")
            foreach(lib ${ALL_LIBS})
                if (TARGET ${lib})
                    # Ignore system libraries, such as iphlpapi
                    if (NOT "${lib}" STREQUAL "iphlpapi")
                        get_target_property(targetType ${lib} TYPE)
                        if (targetType STREQUAL "SHARED_LIBRARY")
                            add_custom_command(TARGET ${targetName} POST_BUILD
                                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                $<TARGET_FILE:${lib}>
                                $<TARGET_FILE_DIR:${targetName}>
                            )
                        endif()
                    endif()
                endif()
            endforeach()
        endif()
    endif()
endfunction(func_target_link_libraries)