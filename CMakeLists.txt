cmake_minimum_required(VERSION 3.21)

set(ProjectName "TEMP")
set(ExeName "temp")


project(${ProjectName} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders for IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)

# Fetch SFML (graphics/window/system only)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
)

# Disable optional SFML components
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  # Build static libraries instead
if(MINGW)
    set(SFML_USE_STATIC_FREETYPE TRUE CACHE BOOL "" FORCE)
endif()


FetchContent_MakeAvailable(SFML)


add_executable(${ExeName} WIN32
    src/main.cpp
)

# Expose the font header so IDEs and builds can find it
target_sources(${ExeName} PRIVATE
    src/fonts/font_data.h
)

# Add the fonts folder to inclue path
target_include_directories(${ExeName} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/fonts
)

target_link_libraries(${ExeName} PRIVATE
    SFML::Graphics
)

if (MSVC)
    target_compile_options(${ExeName} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${ExeName} PRIVATE -Wall -Wextra -Wpedantic)
    if(MINGW)
        target_link_libraries(${ExeName} PRIVATE mingw32)
    endif()
endif()

if(MINGW)
    # Statically link libstdc++ and libgcc so the produced .exe doesn't depend on MinGW runtime DLLs
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.13")
        target_link_options(${ExeName} PRIVATE "-static-libgcc" "-static-libstdc++")
    else()
        # Fallback for older CMake
        set_target_properties(${ExeName} PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

set_target_properties(${ExeName} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${ExeName}>"
)

# Put exe into seperate folder at project root
set(RUNTIME_DIR "${CMAKE_SOURCE_DIR}/${ProjectName}-build")
add_custom_command(TARGET ${ExeName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/sprites" "$<TARGET_FILE_DIR:${ExeName}>/sprites"
    COMMENT "Copying sprites -> $<TARGET_FILE_DIR:${ExeName}>/sprites"
)

set_target_properties(${ExeName} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${RUNTIME_DIR}"
)
