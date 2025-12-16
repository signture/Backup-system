#include <gtest/gtest.h>

#include "SimpleXOREncrypt.h"  // 包含您的加密功能头文件

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include "testUtils.h"

// 测试用例：测试基本的加密和解密功能
TEST(EncryptionTest, BasicEncryptionDecryption) {
    const std::string sourceFile = "test_source.txt";
    const std::string encryptedFile = "test_encrypted.txt";
    const std::string decryptedFile = "test_decrypted.txt";
    const std::string testContent = "Hello, World!";

    // 测试准备：创建测试文件
    // 创建原始文件
    CreateTestFile(sourceFile, testContent);
    // 创建密钥
    std::string key = "SecretKey123";
    // 创建加密文件
    SimpleXOREncrypt encryptor;
    std::string encryptedPath = encryptor.encryptFile(sourceFile, key);
    // 检查加密文件是否存在
    EXPECT_TRUE(std::filesystem::exists(encryptedPath)) << "Encrypted file not found";
    // 创建解密文件
    bool result = encryptor.decryptFile(encryptedPath, decryptedFile, key);
    // 检查解密文件是否存在
    EXPECT_TRUE(std::filesystem::exists(decryptedFile)) << "Decrypted file not found";
    // 检查解密是否成功
    EXPECT_TRUE(result) << "Decryption failed";
    std::vector<char> decryptedContent;
    // 读取解密文件内容
    EXPECT_TRUE(ReadTestFile(decryptedFile, decryptedContent)) << "Failed to read decrypted file";
    // 检查解密内容是否与原始内容一致
    EXPECT_EQ(std::string(decryptedContent.begin(), decryptedContent.end()), testContent)  << "Decrypted content mismatch";

    // 清理文件
    // 删除测试文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(encryptedPath);
    CleanupTestFile(decryptedFile);
}

// 新增：空密钥测试
TEST(EncryptionTest, EmptyKeyEncryptionDecryption) {
    const std::string sourceFile = "test_empty_key.txt";
    const std::string encryptedFile = "test_empty_key_encrypted.txt";
    const std::string decryptedFile = "test_empty_key_decrypted.txt";
    const std::string testContent = "Test empty key encryption!";
    const std::string emptyKey = "";  // 空密钥

    // 清理旧文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(encryptedFile);
    CleanupTestFile(decryptedFile);

    // 创建测试文件
    ASSERT_TRUE(CreateTestFile(sourceFile, testContent)) << "Failed to create test file for empty key";

    // 空密钥加密/解密
    SimpleXOREncrypt encryptor;
    std::string encryptedPath = encryptor.encryptFile(sourceFile, emptyKey);
    ASSERT_TRUE(std::filesystem::exists(encryptedPath)) << "Empty key encryption failed (file not created)";

    bool decryptResult = encryptor.decryptFile(encryptedPath, decryptedFile, emptyKey);
    ASSERT_TRUE(decryptResult) << "Empty key decryption failed";

    // 验证内容一致性（空密钥应兼容处理，如使用默认密钥或直接返回原文）
    std::vector<char> decryptedContent;
    ASSERT_TRUE(ReadTestFile(decryptedFile, decryptedContent)) << "Failed to read decrypted file with empty key";
    ASSERT_EQ(std::string(decryptedContent.begin(), decryptedContent.end()), testContent) 
        << "Empty key decrypted content mismatch";

    // 清理
    CleanupTestFile(sourceFile);
    CleanupTestFile(encryptedPath);
    CleanupTestFile(decryptedFile);
}

// 新增：超长密钥（1024位）测试
TEST(EncryptionTest, LongKeyEncryptionDecryption) {
    const std::string sourceFile = "test_long_key.txt";
    const std::string encryptedFile = "test_long_key_encrypted.txt";
    const std::string decryptedFile = "test_long_key_decrypted.txt";
    const std::string testContent = "Test 1024-bit long key encryption!";
    const std::string longKey(1024, 'K');  // 1024位超长密钥

    // 清理旧文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(encryptedFile);
    CleanupTestFile(decryptedFile);

    // 创建测试文件
    ASSERT_TRUE(CreateTestFile(sourceFile, testContent)) << "Failed to create test file for long key";

    // 超长密钥加密/解密
    SimpleXOREncrypt encryptor;
    std::string encryptedPath = encryptor.encryptFile(sourceFile, longKey);
    ASSERT_TRUE(std::filesystem::exists(encryptedPath)) << "Long key encryption failed (file not created)";

    bool decryptResult = encryptor.decryptFile(encryptedPath, decryptedFile, longKey);
    ASSERT_TRUE(decryptResult) << "Long key decryption failed";

    // 验证内容一致性
    std::vector<char> decryptedContent;
    ASSERT_TRUE(ReadTestFile(decryptedFile, decryptedContent)) << "Failed to read decrypted file with long key";
    ASSERT_EQ(std::string(decryptedContent.begin(), decryptedContent.end()), testContent) 
        << "Long key decrypted content mismatch";

    // 清理
    CleanupTestFile(sourceFile);
    CleanupTestFile(encryptedPath);
    CleanupTestFile(decryptedFile);
}