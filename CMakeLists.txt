cmake_minimum_required(VERSION 3.0...3.25)

project(
    VoxelStreamer
    VERSION 0.1.0
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# WebGPU
# ------------------------------------------------------------

set(WGPU_DIR "${CMAKE_SOURCE_DIR}/third_party/wgpu-native")

if (NOT EMSCRIPTEN)
    add_library(wgpu_native SHARED IMPORTED)

    set_target_properties(wgpu_native PROPERTIES
        IMPORTED_LOCATION "${WGPU_DIR}/lib/libwgpu_native.dylib"
        INTERFACE_INCLUDE_DIRECTORIES "${WGPU_DIR}/include"
    )

    add_subdirectory(third_party/glfw)
else()
    add_library(glfw INTERFACE)
    target_link_libraries(glfw INTERFACE "-sUSE_GLFW=3")
endif()

# ------------------------------------------------------------
# Source files
# ------------------------------------------------------------

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/private/*.cpp"
)

list(REMOVE_ITEM SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/private/main.cpp"
)

# ------------------------------------------------------------
# Library
# ------------------------------------------------------------

add_library(MyLib ${SRC_FILES})

target_include_directories(MyLib
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/public"
)

if (EMSCRIPTEN)
    target_include_directories(MyLib
        PUBLIC
            "${WGPU_DIR}/include"
    )
else()
    target_link_libraries(MyLib
        PUBLIC
            wgpu_native
    )
endif()


add_subdirectory(third_party/glfw3webgpu)
target_include_directories(glfw3webgpu PUBLIC "${WGPU_DIR}/include")

target_link_libraries(MyLib PUBLIC glfw glfw3webgpu)

# ------------------------------------------------------------
# Executable
# ------------------------------------------------------------

add_executable(App
    "${CMAKE_CURRENT_SOURCE_DIR}/src/private/main.cpp"
)

target_link_libraries(App
    PRIVATE
        MyLib
)

# ------------------------------------------------------------
# Warnings
# ------------------------------------------------------------

set_target_properties(App MyLib PROPERTIES
    COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
    # target_compile_options(App PRIVATE /W4)
    # target_compile_options(MyLib PRIVATE /W4)
else()
    # target_compile_options(App PRIVATE -Wall -Wextra -pedantic)
    # target_compile_options(MyLib PRIVATE -Wall -Wextra -pedantic)
endif()

# ------------------------------------------------------------
# Native macOS linking
# ------------------------------------------------------------

if (NOT EMSCRIPTEN)
    target_link_libraries(App
        PRIVATE
            "-framework Metal"
            "-framework QuartzCore"
            "-framework Foundation"
    )

    set_target_properties(App PROPERTIES
        BUILD_RPATH "${WGPU_DIR}/lib"
    )
endif()

# ------------------------------------------------------------
# GLFW
# ------------------------------------------------------------

if (NOT EMSCRIPTEN)
    # add_subdirectory(glfw)
    # target_link_libraries(App PRIVATE glfw)
endif()

# ------------------------------------------------------------
# Xcode
# ------------------------------------------------------------

if (XCODE)
    set_target_properties(App PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
    )
endif()

# ------------------------------------------------------------
# Emscripten / WASM
# ------------------------------------------------------------

if (EMSCRIPTEN)
    set_target_properties(App PROPERTIES
        SUFFIX ".html"
    )

    target_link_options(App PRIVATE
        -sUSE_GLFW=3
        -sASYNCIFY
        --use-port=emdawnwebgpu
        --preload-file "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        -sALLOW_MEMORY_GROWTH
        -g
        -O0
        --source-map-base http://localhost:8000/
    )
endif()
