#include <gtest/gtest.h>

// 自定义测试入口（替代GTest默认的gtest_main）
int main(int argc, char **argv) {
    // 可以在这里添加测试前的初始化逻辑（如日志配置、资源准备等）
    testing::InitGoogleTest(&argc, argv);  // 初始化Google Test框架
    
    // 可以添加自定义参数解析或环境设置
    // ...

    return RUN_ALL_TESTS();  // 运行所有测试用例
}