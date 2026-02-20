set(file_path "cmake/Macros.cmake")
if(EXISTS "${file_path}")
    file(READ "${file_path}" content)
    
    # Target block to replace with
    set(target_block "    if (TARGET \${target})\n        return()\n    endif()")
    
    # Old block to find
    set(old_block "    if (TARGET \${target})\n        message(FATAL_ERROR \"Target '\${target}' is already defined\")\n    endif()")
    
    string(FIND "${content}" "${target_block}" already_patched)
    if(already_patched EQUAL -1)
        string(FIND "${content}" "${old_block}" found_old)
        if(NOT found_old EQUAL -1)
            string(REPLACE "${old_block}" "${target_block}" content "${content}")
            file(WRITE "${file_path}" "${content}")
        else()
            # Regex fallback
            string(REGEX REPLACE 
                "if[ \t]*\\(TARGET[ \t]*\\\${target}\\)[ \t\r\n]*message\\(FATAL_ERROR[ \t]*\"Target[ \t]*'\\\${target}'[ \t]*is[ \t]*already[ \t]*defined\"\\)[ \t\r\n]*endif\\(\\)"
                "${target_block}"
                content "${content}"
            )
            file(WRITE "${file_path}" "${content}")
        endif()
    endif()
endif()
