#[[
    Abstractions and Models are NOT at WAR!
                                            - Cobwoy's Talisman
    But Abstractions don't care and Models can't understand!!
                                            - Lul, Practicality
 ]]
 
 # Macros
# Recursive directory list generator
# Snippet gracefully taken from https://stackoverflow.com/a/53017508
MACRO(COLLECTIVEINCLUDES CurrentDir)
    FILE(GLOB_RECURSE Children LIST_DIRECTORIES true RELATIVE ${CurrentDir} ${CurrentDir}/*)
    FOREACH(Child ${Children})
        IF(IS_DIRECTORY ${CurrentDir}/${Child})
        target_include_directories(KarmaEngine
            PRIVATE
            ${CurrentDir}/${Child}
            PUBLIC
            $<BUILD_INTERFACE:${CurrentDir}/${Child}>
        )
        ENDIF()
    ENDFOREACH()
ENDMACRO()

# Introducing the same directory hierarchy in the solution projects (for MSVC and Xcode)
# Snippet gracefully taken from https://blog.audio-tk.com/2015/09/01/sorting-source-files-and-projects-in-folders-with-cmake-and-visual-studioxcode/
MACRO(SOURCE_GROUP_BY_FOLDER Target)
    SET(SOURCE_GROUP_DELIMITER "/")
    SET(Last_Dir "")
    SET(Files "")
    message(STATUS "Organizing KarmaEngine's files by same, project directory hierarchy.")
    FOREACH(File ${CPPFILES} ${HEADERFILES})
        FILE(RELATIVE_PATH Relative_File "${PROJECT_SOURCE_DIR}/${Target}" ${File})
        GET_FILENAME_COMPONENT(Dir "${Relative_File}" PATH)
        IF (NOT "${Dir}" STREQUAL "${Last_Dir}")
            IF (Files)
                SOURCE_GROUP("${Last_Dir}" FILES ${Files})
            ENDIF (Files)
            SET(Files "")
        ENDIF (NOT "${Dir}" STREQUAL "${Last_Dir}")
        SET(Files ${Files} ${File})
        SET(Last_Dir "${Dir}")
    ENDFOREACH(File)
    IF (Files)
        SOURCE_GROUP("${Last_Dir}" FILES ${Files})
    ENDIF (Files)
ENDMACRO(SOURCE_GROUP_BY_FOLDER)

# Platform specific Defines
if(WIN32)
    add_compile_definitions(UDK_WINDOWS_PLATFORM)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(UDK_LINUX_PLATFORM)
elseif(APPLE)
    add_compile_definitions(UDK_MAC_PLATFORM)
    set(wxUSE_ACCESSIBILITY 0)
endif()

# Handling MSVC static class members for dynamic linkage. I know!
if(MSVC AND BUILD_SHARED_LIBS)
    if(WIN32)
        # Let Karma handle them
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
        add_compile_definitions(UDK_DYNAMIC_LINK)
        # Disable stupid C4251 warnings due to STL's negligence
		# https://docs.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level?view=msvc-170#remarks
        add_compile_options(/wd4251)
    elseif(APPLE)
        add_compile_definitions(UDK_DYNAMIC_LINK UDK_BUILD_SO) # May need to find appropriate define name because Apple doesn't generate SO
    endif()
endif()

set(wxUSE_STL ON CACHE INTERNAL "")
set(wxUSE_REGEX OFF CACHE INTERNAL "")
set(wxUSE_LIBJPEG OFF CACHE INTERNAL "")
set(wxUSE_LIBTIFF OFF CACHE INTERNAL "")
set(wxUSE_ZLIB builtin CACHE INTERNAL "")

# Involved libraries and essentials
add_subdirectory(vendor/wxWidgets)

list(APPEND ESSENTIAL_LIBS spdlog)


# Finally the runnable file!
add_executable(UDK src/UDK/UDKChief.cpp)


target_include_directories(UDK
        PRIVATE  
		${CMAKE_CURRENT_SOURCE_DIR}/../UDK/vendor/wxWidgets/include
    	${CMAKE_CURRENT_SOURCE_DIR}
)


target_compile_definitions(UDK PRIVATE UDK)


if(MSVC)
    # Set local debugger path
    set_target_properties(UDK PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${REPOSITORYROOT}/UDKBin")
elseif(XCODE)
    # Set local debugger path
    set_target_properties(UDK PROPERTIES
    XCODE_GENERATE_SCHEME TRUE
    XCODE_SCHEME_WORKING_DIRECTORY "${REPOSITORYROOT}/UDKBin")
endif()

# Linking to our UDK application
target_link_libraries(UDK PUBLIC wx::core)

# Cluster UDK's libraries into a single folder for project/solution visualization
# set_property(TARGET wxWidgets PROPERTY FOLDER "${UDKLIBRARYFOLDER}")


#[[
 *                                          /\
 *                                         / /
 *                                      /\| |
 *                                      | | |/\
 *                                      | | / /
 *                                      | `  /
 *                                      `\  (___
 *                                     _.->  ,-.-.
 *                                  _.'      |  \ \
 *                                 /    _____| 0 |0\
 *                                |    /`    `^-.\.-'`-._
 *                                |   |                  `-._
 *                                |   :                      `.
 *                                \    `._     `-.__         O.'
 *         _.--,                   \     `._     __.^--._O_..-'
 *        `---, `.                  `\     /` ` `
 *             `\ `,                  `\   |
 *              |   :                   ;  |
 *              /    `.              ___|__|___
 *             /       `.           (          )
 *            /    `---.:____...---' `--------`.
 *           /        (         `.      __      `.
 *          |          `---------' _   /  \       \
 *          |    .-.      _._     (_)  `--'        \
 *          |   (   )    /   \                       \
 *           \   `-'     \   /                       ;-._
 *            \           `-'           \           .'   `.
 *            /`.                  `\    `\     _.-'`-.    `.___
 *           |   `-._                `\    `\.-'       `-.   ,--`
 *            \      `--.___        ___`\    \           ||^\\
 *             `._        | ``----''     `.   `\         `'  `
 *                `--;     \  jgs          `.   `.
 *                   //^||^\\               //^||^\\
 *                   '  `'  `               '   '  `
 ]]
