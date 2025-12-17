// Copyright [2025] <JiJun Lu, Linru Zhou>
#include "CBackupRecorder.h"
#include <fstream>
#include <algorithm>
#include <iostream>
namespace fs = std::filesystem;

#if defined(_MSC_VER)
#include <windows.h>
// GBK(CP936)转UTF-8（极简版，处理用户输入的中文）
std::string gbk_to_utf8(const std::string& gbk_str) {
    // 第一步：GBK转宽字符
    int wlen = MultiByteToWideChar(CP_ACP, 0, gbk_str.c_str(), -1, nullptr, 0);
    if (wlen == 0) return gbk_str;  // 转换失败返回原字符串
    wchar_t* wbuf = new wchar_t[wlen];
    MultiByteToWideChar(CP_ACP, 0, gbk_str.c_str(), -1, wbuf, wlen);

    // 第二步：宽字符转UTF-8
    int ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, nullptr, 0, nullptr, nullptr);
    if (ulen == 0) { delete[] wbuf; return gbk_str; }
    char* ubuf = new char[ulen];
    WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, ubuf, ulen, nullptr, nullptr);

    std::string utf8_str(ubuf);
    delete[] wbuf;
    delete[] ubuf;
    return utf8_str;
}
#else
// Linux/macOS直接返回原字符串（默认UTF-8）
std::string gbk_to_utf8(const std::string& str) {
    return str;
}
# endif

CBackupRecorder::CBackupRecorder() : recorderFilePath("backup_records.json"), autoSaveEnabled(false) {
    // 先检查有没有这个文件
    std::ifstream checkFile(CBackupRecorder::recorderFilePath);
    if (!checkFile.is_open()) {
        // 如果文件不存在，创建一个空文件
        std::ofstream createFile(CBackupRecorder::recorderFilePath);
        createFile.close();
    }
    loadBackupRecordsFromFile(CBackupRecorder::recorderFilePath);
}

CBackupRecorder::CBackupRecorder(bool autoSave) :
    recorderFilePath("backup_records.json"),
    autoSaveEnabled(autoSave) {
    // 先检查有没有这个文件
    std::ifstream checkFile(recorderFilePath);
    if (!checkFile.is_open()) {
        // 如果文件不存在，创建一个空文件
        std::ofstream createFile(recorderFilePath);
        createFile.close();
    }
    loadBackupRecordsFromFile(recorderFilePath);
}

// 构造函数
CBackupRecorder::CBackupRecorder(const std::string& filePath) : autoSaveEnabled(true) {
    // 检查这个路径是文件还是目录
    if (fs::is_directory(filePath)) {
        // 如果是目录
        std::string tempPath = filePath + "/" + "backup_records.json";
        // 检查目录中是否存在同名文件
        if (fs::exists(tempPath)) {
            // 如果目录中存在同名文件，询问用户是否需要加载
            char response;
            std::cout << "Warning: Directory " << filePath << " already contains a file named 'backup_records.json'. "
                      << "Do you want to load this file? (y/n): ";
            std::cin >> response;
            if (response == 'y' || response == 'Y') {
                CBackupRecorder::recorderFilePath = tempPath;
            } else {
                std::cerr << "Operation canceled. No file will be loaded." << std::endl;
                return;
            }
    } else {
        CBackupRecorder::recorderFilePath = tempPath;
    }
    } else if (fs::is_regular_file(filePath)) {
        // 如果是文件，直接赋值
        CBackupRecorder::recorderFilePath = filePath;
    } else {
        // 如果既不是文件也不是目录，报错
        std::cerr << "Error: Invalid file path. It is neither a regular file nor a directory." << std::endl;
        return;
    }
    loadBackupRecordsFromFile(recorderFilePath);
}

CBackupRecorder::~CBackupRecorder() {
    if (autoSaveEnabled) {
        saveBackupRecordsToFile(CBackupRecorder::recorderFilePath);
    }
}


// 这个读取是会直接进行覆盖的
bool CBackupRecorder::loadBackupRecordsFromFile(const std::string& filePath) {
    try {
        // 读取文件
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Failed to open file " << filePath << " for reading." << std::endl;
            return false;
        }
        nlohmann::json j;
        file >> j;

        // 将原先的记录清空
        backupRecords.clear();

        // 将json数据格式转换
        if (j.is_array()) {
            backupRecords = j.get<std::vector<BackupEntry>>();
        }

        file.close();
        return true;
    }catch(const std::exception& e) {
        std::cerr << "Error: Failed to load backup records from file " <<
            filePath << ". Exception: " << e.what() << std::endl;
        return false;
    }
}


bool CBackupRecorder::saveBackupRecordsToFile(const std::string& filePath) {
    try {
        // 写入文件
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Failed to open file " << filePath << " for writing." << std::endl;
            return false;
        }
        nlohmann::json j = backupRecords;
        file << j.dump(4);  // 4 表示缩进空格数
        file.close();
        return true;
    }catch(const std::exception& e) {
        std::cerr << "Error: Failed to save backup records to file " <<
            filePath << ". Exception: " << e.what() << std::endl;
        return false;
    }
}

// 直接将条目添加进来
void CBackupRecorder::addBackupRecord(const BackupEntry& entry) {
    backupRecords.push_back(entry);
}

const std::vector<BackupEntry>& CBackupRecorder::getBackupRecords() const {
    return backupRecords;
}

std::vector<BackupEntry> CBackupRecorder::findBackupRecordsByFileName(const std::string& queryFileName) const {
    std::vector<BackupEntry> result;
    std::copy_if(backupRecords.begin(), backupRecords.end(), std::back_inserter(result),
                 [&queryFileName](const BackupEntry& entry) { return entry.fileName == queryFileName; });
    return result;
}


std::vector<BackupEntry> CBackupRecorder::findBackupRecordsByBackupTime(const std::string& startime,
                                                                      const std::string& endTime) const {
    std::vector<BackupEntry> result;
    std::copy_if(backupRecords.begin(), backupRecords.end(), std::back_inserter(result),
                 [&startime, &endTime](const BackupEntry& entry) {
                     return entry.backupTime >= startime && entry.backupTime <= endTime;
                 });
    return result;
}

// 检查索引是否有效
bool CBackupRecorder::isIndexValid(size_t index) const {
    return index < backupRecords.size();
}

// 根据备份记录获取全局索引
size_t CBackupRecorder::getBackupRecordIndex(const BackupEntry& entry) const {
    auto it = std::find(backupRecords.begin(), backupRecords.end(), entry);
    if (it != backupRecords.end()) {
        return std::distance(backupRecords.begin(), it);
    }
    return std::string::npos;
}

bool CBackupRecorder::deleteBackupRecord(size_t index) {
    if (isIndexValid(index)) {
        backupRecords.erase(backupRecords.begin() + index);
        return true;
    }
    std::cerr << "Error: Invalid index " << index << " for deleting backup record." << std::endl;
    return false;
}

bool CBackupRecorder::deleteBackupRecord(const BackupEntry& entry) {
    size_t index = getBackupRecordIndex(entry);
    if (index != std::string::npos) {
        backupRecords.erase(backupRecords.begin() + index);
        return true;
    }
    std::cerr << "Error: Backup record not found for deletion." << std::endl;
    return false;
}

bool CBackupRecorder::modifyBackupRecord(size_t index, const BackupEntry& newEntry) {
    if (isIndexValid(index)) {
        backupRecords[index] = newEntry;
        return true;
    }
    std::cerr << "Error: Invalid index " << index << " for modifying backup record." << std::endl;
    return false;
}

bool CBackupRecorder::modifyBackupRecord(const BackupEntry& oldEntry, const BackupEntry& newEntry) {
    size_t index = getBackupRecordIndex(oldEntry);
    if (index != std::string::npos) {
        return modifyBackupRecord(index, newEntry);
    }
    std::cerr << "Error: Backup record not found for modification." << std::endl;
    return false;
}

const std::string& CBackupRecorder::getRecorderFilePath() const {
    return recorderFilePath;
}

void CBackupRecorder::addBackupRecord(const std::shared_ptr<CConfig>& config, const std::string& destPath) {
    BackupEntry entry;
    // 之前已经将配置中的路径转换为了绝对路径
    std::string sourcePath = config->getSourcePath();
    // 文件名就是绝对路径的文件名
    std::string fileName = fs::path(sourcePath).filename().string();
    // 完整的目标路径
    std::string destDir = config->getDestinationPath();
    // 最后备份的文件名通过destPath得到
    std::string backupFileName = fs::path(destPath).filename().string();
    // 记录时间，精确到秒，防止统一分钟的两次备份在时间查询中出现问题
    std::time_t now = std::time(nullptr);
    const std::tm* tm = std::localtime(&now);
    char timeBuffer[64];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%M", tm);
    std::string backupTime = timeBuffer;
    // 通过配置文件查看是否有加密等设置
    bool isEncrypted = config->isEncryptionEnabled();
    bool isPacked = config->isPackingEnabled();
    bool isCompressed = config->isCompressionEnabled();
    // 查看配置中有没有相关描述
    std::string description = config->getDescription();
    // 测试的时候发现中文描述之后在保存的时候会有问题，这里修复一下
    description = gbk_to_utf8(description);
    entry = BackupEntry(fileName, sourcePath, destDir, backupFileName,
            backupTime, isEncrypted, isPacked, isCompressed, description);
    // 增加备份记录
    backupRecords.push_back(entry);
}


    // std::string fileName;        // 源文件名
    // std::string sourceFullPath;  // 源文件完整路径
    // std::string destDirectory;   // 备份目标目录
    // std::string backupFileName;  // 最终备份文件名
    // std::string backupTime;      // 备份时间
    // bool isEncrypted;            // 是否加密
    // bool isPacked;               // 是否打包
    // bool isCompressed;           // 是否压缩
