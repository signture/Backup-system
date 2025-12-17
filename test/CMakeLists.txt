# 启用测试功能
enable_testing()

# 引入Google Test（假设GTest位于lib/googletest）
add_subdirectory(../lib/googletest googletest)  # 第二个参数指定GTest构建目录（避免冲突）

# 收集测试源文件
file(GLOB_RECURSE TEST_SOURCES "test_*.cpp")

# 创建测试可执行文件
add_executable(test_app ${TEST_SOURCES})

# 链接依赖（核心库+GTest+JSON库）
target_link_libraries(test_app PRIVATE 
  backup_core                # 依赖主程序核心库
  gtest                      # 依赖GTest库
)

# 添加测试用例（CTest识别）
add_test(NAME BackupToolTests COMMAND test_app)