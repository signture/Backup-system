#include <gtest/gtest.h>

#include "CBackup.h"
#include "CConfig.h"

#include <fstream>
#include <filesystem>
#include <memory>
#include <vector>
#include "testUtils.h"


TEST(BackupTest, BasicBackup) {  
    // 测试准备：创建测试文件s
    const std::string sourcePath = "test_file.txt";
    const std::string destDir = "test_dir";
    const std::string destPath = destDir + "/test_file.txt";
    const std::string testContent = "This is a test content for backup system.";

    // 确保目标目录存在
    if (!std::filesystem::exists(destDir)) {
        std::filesystem::create_directories(destDir);
    }
    
    // 创建源文件
    ASSERT_TRUE(CreateTestFile(sourcePath, testContent)) << "Failed to create test file";

    // 创建配置对象
    auto config = std::make_shared<CConfig>(sourcePath, destDir);
    
    CBackup backup;
    std::string result = backup.doBackup(config);
    EXPECT_TRUE(!result.empty()) << "Backup failed: " << result;

    // 检查备份文件是否存在
    std::ifstream backupFile(destPath);
    EXPECT_TRUE(backupFile.is_open()) << "Backup file not found";
    backupFile.close();
    
    // 检查备份文件内容是否与源文件相同
    std::vector<char> sourceBuffer, backupBuffer;
    bool readSource = ReadTestFile(sourcePath, sourceBuffer);
    bool readBackup = ReadTestFile(destPath, backupBuffer);
    EXPECT_TRUE(readSource) << "Failed to read source file";
    EXPECT_TRUE(readBackup) << "Failed to read backup file";
    EXPECT_EQ(sourceBuffer, backupBuffer) << "Backup content mismatch";


    // 测试清理
    CleanupTestFile(sourcePath);
    CleanupTestFile(destPath);
}