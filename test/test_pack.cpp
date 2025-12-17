#include <gtest/gtest.h>

#include "myPack.h"
#include "testUtils.h"
#include "CBackup.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// 测试pack功能
TEST(myPackTest, PackTest) {
    // 测试准备：创建测试文件和目录
    const std::string testDir = "test_pack_dir";
    const std::string subDir = testDir + "/subdir";
    const std::string file1 = testDir + "/test_file1.txt";
    const std::string file2 = subDir + "/test_file2.txt";
    const std::string destDir = "test_pack_dest";
    const std::string testContent1 = "This is test content for file1.";
    const std::string testContent2 = "This is test content for file2 in subdirectory.";

    // 确保测试目录存在
    std::filesystem::create_directories(testDir);
    std::filesystem::create_directories(subDir);
    std::filesystem::create_directories(destDir);

    // 创建测试文件
    ASSERT_TRUE(CreateTestFile(file1, testContent1)) << "Failed to create test file1";
    ASSERT_TRUE(CreateTestFile(file2, testContent2)) << "Failed to create test file2";

    // 准备文件列表
    std::vector<std::string> files = {file1, file2};

    // 执行打包操作
    myPack packer;
    std::string packedFilePath = packer.pack(files, destDir);

    // 验证打包文件是否创建成功
    EXPECT_FALSE(packedFilePath.empty()) << "Pack operation failed";
    EXPECT_TRUE(std::filesystem::exists(packedFilePath)) << "Packed file not found: " << packedFilePath;

    // 清理测试文件
    try {
        // 需要判断删除动作是否成功
        std::filesystem::remove_all(testDir);
        // 注意：不要删除打包文件，后面的测试还需要用到
    } catch (...) {
        // 清理失败时忽略错误，继续测试
    }
}

// 测试unpack功能
TEST(myPackTest, UnpackTest) {
    // 测试准备：先创建一个打包文件
    const std::string testDir = "test_unpack_dir";
    const std::string subDir = testDir + "/subdir";
    const std::string file1 = testDir + "/test_file1.txt";
    const std::string file2 = subDir + "/test_file2.txt";
    const std::string packDestDir = "test_unpack_pack_dest";
    const std::string unpackDestDir = "test_unpack_unpack_dest";
    const std::string testContent1 = "This is test content for file1.";
    const std::string testContent2 = "This is test content for file2 in subdirectory.";

    // 确保测试目录存在
    std::filesystem::create_directories(testDir);
    std::filesystem::create_directories(subDir);
    std::filesystem::create_directories(packDestDir);
    std::filesystem::create_directories(unpackDestDir);

    // 创建测试文件
    ASSERT_TRUE(CreateTestFile(file1, testContent1)) << "Failed to create test file1";
    ASSERT_TRUE(CreateTestFile(file2, testContent2)) << "Failed to create test file2";

    // 准备文件列表
    std::vector<std::string> files = {file1, file2};

    // 执行打包操作
    myPack packer;
    std::string packedFilePath = packer.pack(files, packDestDir);
    EXPECT_FALSE(packedFilePath.empty()) << "Pack operation failed";

    // 执行解包操作
    bool unpackResult = packer.unpack(packedFilePath, unpackDestDir);
    EXPECT_TRUE(unpackResult) << "Unpack operation failed";

    // 验证解包后的文件是否存在且内容正确
    std::string unpackedFile1 = unpackDestDir + "/test_file1.txt";
    std::string unpackedFile2 = unpackDestDir + "/subdir/test_file2.txt";
    
    EXPECT_TRUE(std::filesystem::exists(unpackedFile1)) << "Unpacked file1 not found";
    EXPECT_TRUE(std::filesystem::exists(unpackedFile2)) << "Unpacked file2 not found";
    
    // 验证文件内容
    std::vector<char> content1, content2;
    EXPECT_TRUE(ReadTestFile(unpackedFile1, content1)) << "Failed to read unpacked file1";
    EXPECT_TRUE(ReadTestFile(unpackedFile2, content2)) << "Failed to read unpacked file2";
    
    std::string content1Str(content1.begin(), content1.end());
    std::string content2Str(content2.begin(), content2.end());
    
    EXPECT_EQ(content1Str, testContent1) << "Content of unpacked file1 mismatch";
    EXPECT_EQ(content2Str, testContent2) << "Content of unpacked file2 mismatch";

    // 清理测试文件
    try {
        std::filesystem::remove_all(testDir);
        std::filesystem::remove_all(packDestDir);
        std::filesystem::remove_all(unpackDestDir);
    } catch (...) {
        // 清理失败时忽略错误
    }
}

// 测试边界情况：打包空文件列表
TEST(myPackTest, PackEmptyFileList) {
    const std::string destDir = "test_empty_pack_dest";
    std::filesystem::create_directories(destDir);
    
    std::vector<std::string> emptyFiles;
    myPack packer;
    std::string packedFilePath = packer.pack(emptyFiles, destDir);
    
    // 空文件列表打包操作应该失败
    EXPECT_TRUE(packedFilePath.empty()) << "Packing empty file list should fail";
    
    // 清理测试文件
    try {
        std::filesystem::remove_all(destDir);
    } catch (...) {
        // 清理失败时忽略错误
    }
}

// 测试边界情况：解包不存在的文件
TEST(myPackTest, UnpackNonExistentFile) {
    const std::string nonExistentFile = "non_existent_pack_file.bkp";
    const std::string destDir = "test_non_existent_unpack_dest";
    std::filesystem::create_directories(destDir);
    
    myPack packer;
    bool result = packer.unpack(nonExistentFile, destDir);
    
    // 预期应该返回false表示解包失败
    EXPECT_FALSE(result) << "Unpacking non-existent file should fail";
    
    // 清理测试文件
    try {
        std::filesystem::remove_all(destDir);
    } catch (...) {
        // 清理失败时忽略错误
    }
}


// 新增：超长路径打包/解包测试（Linux下构造200+字符路径）
TEST(myPackTest, LongPathPackUnpack) {
    // 构造超长路径：多层嵌套目录 + 长文件名（总长度≈250字符）
    const std::string baseDir = "test_long_path_dir";
    const std::string longSubDir = baseDir + "/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z";
    const std::string longFileName = "very_long_file_name_1234567890_abcdefghijklmnopqrstuvwxyz_1234567890.txt";
    const std::string file1 = longSubDir + "/" + longFileName;
    const std::string packDestDir = "test_long_path_pack_dest";
    const std::string unpackDestDir = "test_long_path_unpack_dest";
    const std::string testContent = "Long path file content";

    // 清理旧文件
    CleanupTestDir(baseDir);      // 使用CleanupTestDir而非CleanupTestFile
    CleanupTestDir(packDestDir); 
    CleanupTestDir(unpackDestDir);

    // 创建超长路径和文件
    std::filesystem::create_directories(longSubDir);
    ASSERT_TRUE(CreateTestFile(file1, testContent)) << "Failed to create long path file";
    ASSERT_TRUE(std::filesystem::exists(file1)) << "Long path file not created";

    // 打包：使用先根遍历收集文件列表
    auto config = std::make_shared<CConfig>(baseDir, packDestDir);
    config->setRecursiveSearch(true);
    std::vector<std::string> files = collectFilesToBackup(baseDir, config);
    
    myPack packer;
    std::string packedFilePath = packer.pack(files, packDestDir);
    ASSERT_FALSE(packedFilePath.empty()) << "Long path pack failed";

    // 解包
    std::filesystem::create_directories(unpackDestDir);
    bool unpackResult = packer.unpack(packedFilePath, unpackDestDir);
    ASSERT_TRUE(unpackResult) << "Long path unpack failed";

    // 验证解包结果（路径结构保持一致）
    std::string unpackedFile = unpackDestDir + "/" + longSubDir + "/" + longFileName;
    ASSERT_TRUE(std::filesystem::exists(unpackedFile)) << "Unpacked long path file not found: " << unpackedFile;

    std::vector<char> content;
    ASSERT_TRUE(ReadTestFile(unpackedFile, content)) << "Failed to read unpacked long path file";
    ASSERT_EQ(std::string(content.begin(), content.end()), testContent) << "Long path file content mismatch";

    // 清理
    std::filesystem::remove_all(baseDir);
    std::filesystem::remove_all(packDestDir);
    std::filesystem::remove_all(packedFilePath);
    std::filesystem::remove_all(unpackDestDir);
}