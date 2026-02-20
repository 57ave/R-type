if(NOT EXISTS "${SOURCE_DIR}")
    message(FATAL_ERROR "SOURCE_DIR does not exist: ${SOURCE_DIR}")
endif()

if(NOT EXISTS "${DEST_DIR}")
    file(MAKE_DIRECTORY "${DEST_DIR}")
endif()

file(GLOB DLLS "${SOURCE_DIR}/*.dll")

if(NOT DLLS)
    message(STATUS "No DLLs found in ${SOURCE_DIR}")
else()
    foreach(DLL ${DLLS})
        get_filename_component(DLL_NAME ${DLL} NAME)
        # Use copy_if_different to avoid unnecessary writes
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL}" "${DEST_DIR}/${DLL_NAME}")
    endforeach()
endif()
