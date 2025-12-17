// Copyright [2025] <JiJun Lu, Linru Zhou>
#ifndef INCLUDE_TESTUTILS_H_
#define INCLUDE_TESTUTILS_H_

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// 辅助函数：创建测试目录
inline bool CreateTestDir(const std::string& dirPath) {
    try {
        if (!fs::exists(dirPath)) {
            fs::create_directories(dirPath);
        }
        return true;
    } catch (...) {
        return false;
    }
}

// 辅助函数：创建测试文件
inline bool CreateTestFile(const std::string& filePath, const std::string& content) {
    try {
        // 确保目录存在
        fs::path dir = fs::path(filePath).parent_path();
        if (!dir.empty() && !fs::exists(dir)) {
            fs::create_directories(dir);
        }

        // 创建并写入文件
        std::ofstream file(filePath, std::ios::binary);
        if (!file) return false;
        file.write(content.data(), content.size());
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

// 辅助函数：读取测试文件内容
inline bool ReadTestFile(const std::string& filePath, std::vector<char>& buffer) {
    try {
        if (!fs::exists(filePath)) return false;

        // 获取文件大小
        std::uintmax_t size = fs::file_size(filePath);
        buffer.resize(static_cast<size_t>(size));

        // 读取文件内容
        std::ifstream file(filePath, std::ios::binary);
        if (!file) return false;
        file.read(buffer.data(), buffer.size());
        return file.gcount() == static_cast<std::streamsize>(buffer.size());
    } catch (...) {
        return false;
    }
}

// 辅助函数：清理测试文件
inline void CleanupTestFile(const std::string& filePath) {
    try {
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }
    } catch (...) {
        // 忽略清理过程中的错误
    }
}

// 辅助函数：清理测试目录
inline void CleanupTestDir(const std::string& dirPath) {
    try {
        if (fs::exists(dirPath)) {
            fs::remove_all(dirPath);
        }
    } catch (...) {
        // 忽略清理过程中的错误
    }
}

// 辅助函数：比较两个目录结构是否一致
inline bool CompareDirs(const std::string& dir1, const std::string& dir2) {
    try {
        // 递归遍历目录1
        for (const auto& entry : fs::recursive_directory_iterator(dir1)) {
            // 构建对应路径
            std::string relativePath = fs::relative(entry.path(), dir1).string();
            std::string correspondingPath = (fs::path(dir2) / relativePath).string();

            // 检查对应路径是否存在
            if (!fs::exists(correspondingPath)) return false;

            // 检查是否为目录
            if (entry.is_directory() != fs::is_directory(correspondingPath)) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

#endif  // INCLUDE_TESTUTILS_H_
