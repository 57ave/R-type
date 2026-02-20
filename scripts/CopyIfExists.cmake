if(EXISTS "${SRC_FILE}")
    get_filename_component(FILE_NAME "${SRC_FILE}" NAME)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_FILE}" "${DST_DIR}/${FILE_NAME}")
endif()
