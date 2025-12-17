// Copyright [2025] <JiJun Lu, Linru Zhou>
#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <set>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;


bool ReadFile(const std::string& filePath, std::vector<char>& buffer);
bool WriteFile(const std::string& filePath, const std::vector<char>& buffer);
// 避免与 Windows API 宏 CopyFile 冲突
bool CopyFileBinary(const std::string& srcPath, const std::string& destPath);
// 检查目录是否存在并且是否可写
bool isPathWritable(const std::string& path);


#endif  // INCLUDE_UTILS_H_
