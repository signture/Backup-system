# 收集主程序源文件（排除test目录）
file(GLOB_RECURSE SOURCES "*.cpp" EXCLUDE "main.cpp")  # 业务逻辑代码
file(GLOB_RECURSE HEADERS "../include/*.h")            # 头文件

# 创建核心库（业务逻辑封装）
add_library(backup_core STATIC ${SOURCES} ${HEADERS})

# 核心库配置（包含目录、编译选项等）
target_include_directories(backup_core PUBLIC ../include)
if(MSVC)
  target_compile_options(backup_core PRIVATE /W4 /utf-8 /Zc:__cplusplus)
else()
  target_compile_options(backup_core PRIVATE -Wall -Wextra -Wpedantic)
endif()


# 主应用程序（仅包含入口文件main.cpp）
add_executable(backup_app main.cpp)
target_link_libraries(backup_app PRIVATE backup_core)  # 链接核心库
if(MSVC)
  target_compile_options(backup_app PRIVATE /utf-8 /Zc:__cplusplus)
endif()