function(validate_bin_name json_file bin_name)
    #message(STATUS "BIN_NAME: ${bin_name}")
    #message(STATUS "JSON file : ${json_file}")

    file(GLOB BIN_FILES
        "${CMAKE_BINARY_DIR}/sample_apps/**/${bin_name}"
    )

    if(NOT BIN_FILES)
        message(FATAL_ERROR
            "Binary '${bin_name}' not found "
            "from JSON file '${json_file}'"
        )
    endif()
endfunction()
