# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'glfw' and either 'webgpu' or 'wgpu_native' exist.

add_library(glfw3webgpu STATIC glfw3webgpu.c)

target_include_directories(glfw3webgpu
    PUBLIC
        .
)

if (TARGET webgpu)
    target_link_libraries(glfw3webgpu
        PUBLIC
            glfw
            webgpu
    )
elseif (TARGET wgpu_native)
    target_link_libraries(glfw3webgpu
        PUBLIC
            glfw
            wgpu_native
    )
else()
    message(FATAL_ERROR "glfw3webgpu requires either a 'webgpu' or 'wgpu_native' target")
endif()

if (APPLE)
    target_compile_options(glfw3webgpu PRIVATE -x objective-c)

    target_link_libraries(glfw3webgpu
        PRIVATE
            "-framework Cocoa"
            "-framework CoreVideo"
            "-framework IOKit"
            "-framework QuartzCore"
    )
endif()