# 收集主程序源文件（排除test目录）
file(GLOB_RECURSE SOURCES "*.cpp" EXCLUDE "main.cpp")  # 业务逻辑代码
file(GLOB_RECURSE HEADERS "../include/*.h")            # 头文件

# 创建核心库（业务逻辑封装）
add_library(backup_core STATIC ${SOURCES} ${HEADERS})

# 核心库配置（包含目录、编译选项等）
target_include_directories(backup_core PUBLIC ../include)
# 添加 GLFW 头文件到核心库的包含路径，保证像 src/gui.cpp 这样的源文件能找到 <GLFW/glfw3.h>
target_include_directories(backup_core PUBLIC ../lib/glfw/include)
if(MSVC)
  target_compile_options(backup_core PRIVATE /W4 /utf-8 /Zc:__cplusplus)
else()
  target_compile_options(backup_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 创建 ImGui 库
set(IMGUI_SOURCES
    ../lib/imgui/imgui.cpp
    ../lib/imgui/imgui_draw.cpp
    ../lib/imgui/imgui_tables.cpp
    ../lib/imgui/imgui_widgets.cpp
    ../lib/imgui/backends/imgui_impl_glfw.cpp
    ../lib/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ../lib/imgui
    ../lib/imgui/backends
    ../lib/glfw/include  # 显式添加 GLFW 头文件路径
)
# 链接 GLFW 以获取其头文件路径和依赖关系
target_link_libraries(imgui PUBLIC glfw)
if(MSVC)
  target_compile_options(imgui PRIVATE /utf-8 /Zc:__cplusplus)
else()
  target_compile_options(imgui PRIVATE -Wall -Wextra)
endif()

# 主应用程序（仅包含入口文件main.cpp）
add_executable(backup_app main.cpp)
target_link_libraries(backup_app PRIVATE backup_core imgui glfw)  # 链接核心库、ImGui和GLFW
if(MSVC)
  target_compile_options(backup_app PRIVATE /utf-8 /Zc:__cplusplus)
  # Windows 下链接 OpenGL
  target_link_libraries(backup_app PRIVATE opengl32)
else()
  # Linux/macOS 下链接 OpenGL
  find_package(OpenGL REQUIRED)
  target_link_libraries(backup_app PRIVATE OpenGL::GL)
endif()