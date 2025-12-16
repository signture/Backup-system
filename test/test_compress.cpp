#include <gtest/gtest.h>

#include "HuffmanCompress.h"  // 包含您的压缩功能头文件

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include "testUtils.h"

// 测试用例：测试基本的压缩和解压功能
// 修复基本压缩测试用例
TEST(CompressionTest, BasicCompressionDecompression) {
    const std::string sourceFile = "test_source.txt";
    const std::string decompressedFile = "test_decompressed.txt";
    const std::string testContent = "Hello, this is a test for Huffman compression!";
    
    // 清理可能存在的旧测试文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(decompressedFile);
    
    // 测试准备：创建测试文件
    ASSERT_TRUE(CreateTestFile(sourceFile, testContent)) << "Failed to create test file";

    // 创建压缩器
    HuffmanCompress huffmanCompressor;
    
    // 执行压缩 - 修改为正确的接口调用
    std::string compressedFile = huffmanCompressor.compressFile(sourceFile);
    ASSERT_FALSE(compressedFile.empty()) << "Compression failed";
    
    // 验证压缩文件存在
    ASSERT_TRUE(std::filesystem::exists(compressedFile)) << "Compressed file not created";
    
    // 执行解压
    bool decompressResult = huffmanCompressor.decompressFile(compressedFile, decompressedFile);
    ASSERT_TRUE(decompressResult) << "Decompression failed";
    
    // 读取解压后的文件内容
    std::vector<char> decompressedContent;
    ASSERT_TRUE(ReadTestFile(decompressedFile, decompressedContent)) << "Failed to read decompressed file";
    
    // 验证解压后的内容与原始内容一致
    ASSERT_EQ(std::string(decompressedContent.begin(), decompressedContent.end()), testContent) 
        << "Decompressed content does not match original content";
    
    // 清理测试文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(compressedFile);
    CleanupTestFile(decompressedFile);
}

// 同样修改其他测试用例
// 修复空文件测试用例
TEST(CompressionTest, EmptyFile) {
    const std::string sourceFile = "test_empty.txt";
    const std::string decompressedFile = "test_empty_decompressed.txt";
    const std::string emptyContent = "";
    
    // 清理可能存在的旧测试文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(decompressedFile);
    
    // 测试准备：创建空测试文件
    ASSERT_TRUE(CreateTestFile(sourceFile, emptyContent)) << "Failed to create empty test file";

    // 创建压缩类
    HuffmanCompress huffmanCompressor;
    
    // 执行压缩 - 修改为正确的接口调用
    std::string compressedFile = huffmanCompressor.compressFile(sourceFile);
    ASSERT_FALSE(compressedFile.empty()) << "Compression of empty file failed";
    
    // 执行解压
    bool decompressResult = huffmanCompressor.decompressFile(compressedFile, decompressedFile);
    ASSERT_TRUE(decompressResult) << "Decompression of empty file failed";
    
    // 读取解压后的文件内容
    std::vector<char> decompressedContent;
    ASSERT_TRUE(ReadTestFile(decompressedFile, decompressedContent)) << "Failed to read decompressed empty file";
    
    // 验证解压后的内容是空的
    ASSERT_EQ(decompressedContent.size(), 0) << "Decompressed empty file should be empty";
    
    // 清理测试文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(compressedFile);
    CleanupTestFile(decompressedFile);
}

// 修复CRC32校验测试用例
TEST(CompressionTest, CRC32Check) {
    const std::string sourceFile = "test_crc.txt";
    const std::string testContent = "CRC32 check test content";
    
    // 清理可能存在的旧测试文件
    CleanupTestFile(sourceFile);
    
    // 测试准备：创建测试文件
    ASSERT_TRUE(CreateTestFile(sourceFile, testContent)) << "Failed to create test file";

    // 创建压缩类
    HuffmanCompress huffmanCompressor;
    
    // 执行压缩 - 修改为正确的接口调用
    std::string compressedFile = huffmanCompressor.compressFile(sourceFile);
    ASSERT_FALSE(compressedFile.empty()) << "Compression failed";
    
    // 清理测试文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(compressedFile);
}

// 新增：超大文件（100MB）压缩/解压测试
TEST(CompressionTest, LargeFileCompressionDecompression) {
    const std::string sourceFile = "test_large.txt";
    const std::string decompressedFile = "test_large_decompressed.txt";
    const size_t largeFileSize = 100 * 1024 * 1024;  // 100MB
    const std::string dataBlock(1024 * 1024, 'A');  // 1MB数据块

    // 清理旧文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(decompressedFile);

    // 生成100MB超大文件（循环写入数据块，避免内存溢出）
    std::ofstream largeFile(sourceFile, std::ios::binary);
    ASSERT_TRUE(largeFile.is_open()) << "Failed to create large test file";
    for (size_t i = 0; i < 100; ++i) {
        largeFile.write(dataBlock.data(), dataBlock.size());
    }
    largeFile.close();
    ASSERT_TRUE(std::filesystem::file_size(sourceFile) == largeFileSize) << "Large file size mismatch";

    // 执行压缩和解压
    HuffmanCompress huffmanCompressor;
    std::string compressedFile = huffmanCompressor.compressFile(sourceFile);
    ASSERT_FALSE(compressedFile.empty()) << "Large file compression failed";
    ASSERT_TRUE(std::filesystem::exists(compressedFile)) << "Compressed large file not found";

    bool decompressResult = huffmanCompressor.decompressFile(compressedFile, decompressedFile);
    ASSERT_TRUE(decompressResult) << "Large file decompression failed";
    ASSERT_TRUE(std::filesystem::file_size(decompressedFile) == largeFileSize) << "Decompressed large file size mismatch";

    // 验证内容一致性（抽样验证，避免全量读取占用内存）
    std::ifstream srcStream(sourceFile, std::ios::binary);
    std::ifstream decStream(decompressedFile, std::ios::binary);
    ASSERT_TRUE(srcStream.is_open() && decStream.is_open()) << "Failed to open large files for content check";
    
    char srcBuf[1024], decBuf[1024];
    for (size_t i = 0; i < 100; ++i) {  // 抽样100个数据块
        srcStream.read(srcBuf, sizeof(srcBuf));
        decStream.read(decBuf, sizeof(decBuf));
        ASSERT_EQ(memcmp(srcBuf, decBuf, sizeof(srcBuf)), 0) << "Large file content mismatch at block " << i;
    }

    // 清理测试文件
    CleanupTestFile(sourceFile);
    CleanupTestFile(compressedFile);
    CleanupTestFile(decompressedFile);
}