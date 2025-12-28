# 最低CMake版本要求
cmake_minimum_required(VERSION 3.14)

# 项目名称和版本
project(BackupTool VERSION 1.0 LANGUAGES CXX)

# 设置C++标准（全局生效）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 在所有子项目添加之前统一设置 MSVC 运行时类型，避免子模块使用不同 CRT 导致链接错误
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "MSVC runtime library" FORCE)
endif()
# Force googletest to use shared CRT to match project runtime and avoid linker conflicts
set(gtest_force_shared_crt ON CACHE BOOL "Force Googletest to use shared CRT" FORCE)

# 添加JSON库
include(FetchContent)
# FetchContent_Declare(
#     json
#     GIT_REPOSITORY https://github.com/nlohmann/json.git
#     GIT_TAG v3.11.3  # 指定版本
# )
# FetchContent_MakeAvailable(json)
include_directories(${CMAKE_SOURCE_DIR}/lib/json/include)

# 配置 GLFW 构建选项（在 add_subdirectory 之前设置）
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)

# 添加 GLFW 库
 
# 在添加子模块（如 GLFW）之前设置 MSVC 静态运行时选项，确保所有子项目使用相同的 CRT，
# 否则会导致链接时出现未解析的 __imp__ 符号（runtime mismatch）。
if(MSVC)
  # Ensure all targets (including subdirectories like GLFW) use the same MSVC runtime.
  # Prefer the dynamic DLL runtime to match GLFW's usual build configuration and avoid
  # unresolved __imp__ symbols caused by CRT mismatches.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "MSVC runtime library" FORCE)
endif()

add_subdirectory(lib/glfw)

# 添加 ImGui 库的头文件路径
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/backends)

# 不需要额外设置编译标志，这里统一使用 CMAKE_MSVC_RUNTIME_LIBRARY 控制 CRT 类型。

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# 设置输出目录（全局生效）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含子目录（分别处理主程序和测试）
add_subdirectory(src)    # 主程序逻辑（src/CMakeLists.txt）
add_subdirectory(test)   # 测试逻辑（test/CMakeLists.txt）