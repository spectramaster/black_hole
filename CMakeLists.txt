cmake_minimum_required(VERSION 3.21)
project(BlackHoleSim VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(APPLE)
    message(STATUS "Building for macOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
    # macOS OpenGL is deprecated but we'll use it anyway
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

# Dependencies
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# ImGui - using system package
find_path(IMGUI_INCLUDE_DIR imgui/imgui.h REQUIRED)
find_library(IMGUI_LIBRARY imgui REQUIRED)

# Common dependencies
set(DEPS glfw GLEW::GLEW glm::glm OpenGL::GL ${IMGUI_LIBRARY})

# 2D Lensing executable
add_executable(BlackHole2D 2D_lensing.cpp)
target_link_libraries(BlackHole2D PRIVATE ${DEPS})
target_include_directories(BlackHole2D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 3D Black Hole executable
add_executable(BlackHole3D black_hole.cpp)
target_link_libraries(BlackHole3D PRIVATE ${DEPS})
target_include_directories(BlackHole3D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Shader files (copy to output dir)
file(GLOB SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.comp"
)

foreach(shader ${SHADERS})
    add_custom_command(TARGET BlackHole3D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${shader}
        $<TARGET_FILE_DIR:BlackHole3D>
    )
endforeach()

# Test executable for Mac compatibility
add_executable(TestMacCompat tests/test_mac_compatibility.cpp)
target_link_libraries(TestMacCompat PRIVATE glfw GLEW::GLEW OpenGL::GL)
target_include_directories(TestMacCompat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Simple ImGui test
add_executable(TestImGui black_hole_simple_ui.cpp)
target_link_libraries(TestImGui PRIVATE ${DEPS} stb)
target_include_directories(TestImGui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/include/imgui
)

# UI-Enhanced Black Hole Simulator
add_executable(BlackHoleUI black_hole_ui.cpp src/ui/imgui_interface.cpp)
target_link_libraries(BlackHoleUI PRIVATE ${DEPS} stb)
target_include_directories(BlackHoleUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/include/imgui
)

# Copy shaders for UI version
foreach(shader ${SHADERS})
    add_custom_command(TARGET BlackHoleUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${shader}
        $<TARGET_FILE_DIR:BlackHoleUI>
    )
endforeach()

# ========== Unit Tests ==========

# Performance Manager Unit Test
add_executable(TestPerformanceManager
    tests/unit/test_performance_manager.cpp
    src/performance/performance_manager.cpp
)
target_include_directories(TestPerformanceManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TestPerformanceManager PRIVATE pthread)

# Enable testing
enable_testing()
add_test(NAME PerformanceManagerTest COMMAND TestPerformanceManager)

# Shader Compilation Integration Test (requires display)
add_executable(TestShaderCompilation
    tests/integration/test_shader_compilation.cpp
)
target_link_libraries(TestShaderCompilation PRIVATE glfw GLEW::GLEW OpenGL::GL)
target_include_directories(TestShaderCompilation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Note: This test requires a display, will fail in headless environment

# Shader Syntax Validation Test (no OpenGL context needed)
add_executable(TestShaderSyntax
    tests/unit/test_shader_syntax.cpp
)
add_test(NAME ShaderSyntaxTest COMMAND TestShaderSyntax WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

