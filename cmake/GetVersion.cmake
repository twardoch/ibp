# Get version from git tags or fallback to VERSION.TXT
function(get_version_from_file OUTPUT_VAR)
    # Try to get version from git tags first
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_TAG
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        if(GIT_TAG)
            # Remove 'v' prefix if present
            string(REGEX REPLACE "^v" "" VERSION_FROM_GIT "${GIT_TAG}")
            # Validate semver format
            string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+(-[a-zA-Z0-9.-]+)?(\\+[a-zA-Z0-9.-]+)?$" VALID_VERSION "${VERSION_FROM_GIT}")
            if(VALID_VERSION)
                set(${OUTPUT_VAR} "${VERSION_FROM_GIT}" PARENT_SCOPE)
                return()
            endif()
        endif()
        
        # If not on exact tag, try to get version from latest tag + commits
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0 HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_LATEST_TAG
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        if(GIT_LATEST_TAG)
            # Get commit count since latest tag
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-list --count ${GIT_LATEST_TAG}..HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE COMMIT_COUNT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            
            # Get short commit hash
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE SHORT_COMMIT
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            
            if(COMMIT_COUNT AND SHORT_COMMIT AND COMMIT_COUNT GREATER 0)
                string(REGEX REPLACE "^v" "" BASE_VERSION "${GIT_LATEST_TAG}")
                set(${OUTPUT_VAR} "${BASE_VERSION}-dev.${COMMIT_COUNT}+${SHORT_COMMIT}" PARENT_SCOPE)
                return()
            endif()
        endif()
    endif()
    
    # Fallback to VERSION.TXT file
    if(EXISTS "${CMAKE_SOURCE_DIR}/VERSION.TXT")
        file(READ "${CMAKE_SOURCE_DIR}/VERSION.TXT" VERSION_CONTENT)
        string(STRIP "${VERSION_CONTENT}" VERSION_CONTENT)
        set(${OUTPUT_VAR} "${VERSION_CONTENT}" PARENT_SCOPE)
    else()
        # Last resort fallback
        set(${OUTPUT_VAR} "0.0.0-unknown" PARENT_SCOPE)
    endif()
endfunction()

# Get git metadata for version header
function(get_git_metadata)
    find_package(Git QUIET)
    if(GIT_FOUND)
        # Get commit hash
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        # Get branch name
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_BRANCH
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        # Get commit date
        execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=iso8601
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_DATE
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        # Check if working directory is dirty
        execute_process(
            COMMAND ${GIT_EXECUTABLE} diff --quiet HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE GIT_DIRTY
            ERROR_QUIET
        )
        
        if(GIT_DIRTY)
            set(GIT_DIRTY_FLAG "dirty")
        else()
            set(GIT_DIRTY_FLAG "clean")
        endif()
        
        set(GIT_COMMIT_HASH "${GIT_COMMIT_HASH}" PARENT_SCOPE)
        set(GIT_BRANCH "${GIT_BRANCH}" PARENT_SCOPE)
        set(GIT_COMMIT_DATE "${GIT_COMMIT_DATE}" PARENT_SCOPE)
        set(GIT_DIRTY_FLAG "${GIT_DIRTY_FLAG}" PARENT_SCOPE)
    else()
        set(GIT_COMMIT_HASH "unknown" PARENT_SCOPE)
        set(GIT_BRANCH "unknown" PARENT_SCOPE)
        set(GIT_COMMIT_DATE "unknown" PARENT_SCOPE)
        set(GIT_DIRTY_FLAG "unknown" PARENT_SCOPE)
    endif()
endfunction() 