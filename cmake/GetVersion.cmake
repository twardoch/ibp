# Read version from VERSION.TXT in the root directory
function(get_version_from_file OUTPUT_VAR)
    file(READ "${CMAKE_SOURCE_DIR}/VERSION.TXT" VERSION_CONTENT)
    string(STRIP "${VERSION_CONTENT}" VERSION_CONTENT)
    set(${OUTPUT_VAR} "${VERSION_CONTENT}" PARENT_SCOPE)
endfunction() 