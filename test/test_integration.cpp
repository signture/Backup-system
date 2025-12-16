#include <gtest/gtest.h>
#include "CBackup.h"
#include "CConfig.h"
#include "HuffmanCompress.h"
#include "SimpleXOREncrypt.h"
#include "myPack.h"
#include "testUtils.h"
#include <filesystem>
#include <vector>

// 集成测试：备份→压缩→加密→解密→解压→验证全流程
TEST(IntegrationTest, BackupCompressEncryptDecryptDecompress) {
    // 测试文件配置
    const std::string sourceFile = "test_integration_source.txt";
    const std::string backupDir = "test_integration_backup";
    const std::string compressedFile = "test_integration_compressed";
    const std::string encryptedFile = "test_integration_encrypted.enc";
    const std::string decryptedFile = "test_integration_decrypted";
    const std::string decompressedFile = "test_integration_decompressed.txt";
    const std::string testContent = "Integration test: Backup → Compress → Encrypt → Decrypt → Decompress";
    const std::string encryptKey = "IntegrationTestKey123";

    // 清理旧文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(backupDir);
    CleanupTestFile(compressedFile);
    CleanupTestFile(encryptedFile);
    CleanupTestFile(decryptedFile);
    CleanupTestFile(decompressedFile);

    try {
        // 1. 备份：源文件 → 备份文件
        ASSERT_TRUE(CreateTestFile(sourceFile, testContent)) << "Step 1: Create source file failed";
        auto config = std::make_shared<CConfig>(sourceFile, backupDir);
        CBackup backup;
        std::string backupResult = backup.doBackup(config);
        ASSERT_FALSE(backupResult.empty()) << "Step 1: Backup failed";
        std::string backupFilePath = backupDir + "/" + sourceFile;
        ASSERT_TRUE(std::filesystem::exists(backupFilePath)) << "Step 1: Backup file not found";

        // 2. 压缩：备份文件 → 压缩文件
        HuffmanCompress compressor;
        std::string actualCompressedFile = compressor.compressFile(backupFilePath);
        ASSERT_FALSE(actualCompressedFile.empty()) << "Step 2: Compress failed";
        ASSERT_TRUE(std::filesystem::exists(actualCompressedFile)) << "Step 2: Compressed file not found";

        // 3. 加密：压缩文件 → 加密文件
        SimpleXOREncrypt encryptor;
        std::string actualEncryptedFile = encryptor.encryptFile(actualCompressedFile, encryptKey);
        ASSERT_TRUE(std::filesystem::exists(actualEncryptedFile)) << "Step 3: Encrypt failed";

        // 4. 解密：加密文件 → 解密后的压缩文件
        bool decryptResult = encryptor.decryptFile(actualEncryptedFile, decryptedFile, encryptKey);
        ASSERT_TRUE(decryptResult) << "Step 4: Decrypt failed";
        ASSERT_TRUE(std::filesystem::exists(decryptedFile)) << "Step 4: Decrypted file not found";

        // 5. 解压：解密后的压缩文件 → 最终文件
        bool decompressResult = compressor.decompressFile(decryptedFile, decompressedFile);
        ASSERT_TRUE(decompressResult) << "Step 5: Decompress failed";
        ASSERT_TRUE(std::filesystem::exists(decompressedFile)) << "Step 5: Decompressed file not found";

        // 6. 验证：最终文件与源文件内容一致
        std::vector<char> sourceContent, finalContent;
        ASSERT_TRUE(ReadTestFile(sourceFile, sourceContent)) << "Step 6: Read source file failed";
        ASSERT_TRUE(ReadTestFile(decompressedFile, finalContent)) << "Step 6: Read final file failed";
        ASSERT_EQ(sourceContent, finalContent) << "Step 6: Final content mismatch with source";
    } catch (const std::exception& e) {
        FAIL() << "Integration test failed with exception: " << e.what();
    }

    // 清理测试文件
    CleanupTestFile(sourceFile);
    std::filesystem::remove_all(backupDir);
    CleanupTestFile(compressedFile);
    CleanupTestFile(encryptedFile);
    CleanupTestFile(decryptedFile);
    CleanupTestFile(decompressedFile);
}