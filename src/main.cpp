// Copyright [2025] <JiJun Lu, Linru Zhou>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

#include "CBackup.h"
#include "CConfig.h"
#include "PackFactory.h"
#include "CompressFactory.h"
#include "EncryptFactory.h"
#include "CBackupRecorder.h"

namespace fs = std::filesystem;

#if defined(_MSC_VER)
#include <windows.h>
// UTF-8 转 GBK(CP936)（用于 Windows 显示）
std::string utf8_to_gbk(const std::string& utf8_str) {
    // 第一步：UTF-8 转宽字符
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
    if (wlen == 0) return utf8_str;   // 转换失败返回原字符串
    wchar_t* wbuf = new wchar_t[wlen];
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, wbuf, wlen);

    // 第二步：宽字符转 GBK
    int glen = WideCharToMultiByte(CP_ACP, 0, wbuf, -1, nullptr, 0, nullptr, nullptr);
    if (glen == 0) { delete[] wbuf; return utf8_str; }
    char* gbuf = new char[glen];
    WideCharToMultiByte(CP_ACP, 0, wbuf, -1, gbuf, glen, nullptr, nullptr);

    std::string gbk_str(gbuf);
    delete[] wbuf;
    delete[] gbuf;
    return gbk_str;
}
#else
// Linux/macOS 直接返回原字符串（默认 UTF-8 显示）
std::string utf8_to_gbk(const std::string& str) {
    return str;
}
# endif

// --mode backup --src "F:\courser_project\software_development\testBox\ori_A" --dst "./"
// --mode recover --dst "./" --to "./restore_repository/ori_A"

static void printHelp() {
    std::cout << "Usage (pseudo CLI):\n"
              << "--mode backup  --src <path> --dst <relative_path> [--include \".*\\.txt\" "
               "--pack <packType>(default: none)\n --compress <compressType>(default: none)   "
               "--encrypt <encryptType>(default: none)  "
               "--key <encryptKey>  --desc <description>]\n"
              << "--mode recover --fn <filename> --to <target_path>\n";
}


static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') { inQuotes = !inQuotes; continue; }
        if (!inQuotes && std::isspace(static_cast<unsigned char>(c))) {
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) tokens.push_back(cur);
    return tokens;
}

static int runParsed(const std::vector<std::string>& args) {
    std::string mode;
    std::string packType = "none";  // 新增加一个参数用于指定打包算法,默认不打包
    std::string compressType = "none";  // 新增加一个参数用于指定压缩算法,默认不压缩
    std::string encryptType = "none";  // 新增加一个参数用于指定加密算法,默认不加密
    std::string encryptKey;
    std::string srcPath;
    std::string dstPath;
    std::string includeRegex;
    std::string restoreTo;
    std::string backupFileName;
    std::string repoPath;
    std::string description = "";  // 新增一个参数用于指定备份行为描述,默认空字符串

    auto nextVal = [&](size_t& i, std::string& out){ if (i + 1 < args.size())
                    { out = args[++i]; return true; } return false; };
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "--mode") { nextVal(i, mode);
        } else if (arg == "--pack") { nextVal(i, packType);
        } else if (arg == "--encrypt") { nextVal(i, encryptType);
        } else if (arg == "--compress") { nextVal(i, compressType);
        } else if (arg == "--key") { nextVal(i, encryptKey);
        } else if (arg == "--src") { nextVal(i, srcPath);
        } else if (arg == "--dst") { nextVal(i, dstPath);
        } else if (arg == "--fn") { nextVal(i, backupFileName);
        } else if (arg == "--include") { nextVal(i, includeRegex);
        } else if (arg == "--to") { nextVal(i, restoreTo);
        } else if (arg == "--repo") { nextVal(i, repoPath);
        } else if (arg == "--help" || arg == "-h") { printHelp(); return 0;
        } else if (arg == "--desc") { nextVal(i, description);
        }  // 新增参数处理
    }

    // 设置备份仓库路径  -- 这里目前的唯一功能是用于设置备份记录的路径
    // if (!repoPath.empty()) {
    //     BACKUP_REPOSITORY_ROOT = repoPath;
    //     std::cout << "Backup repository set to: " << BACKUP_REPOSITORY_ROOT << std::endl;
    //     return 0;
    // }

    // 创建备份记录器
    const std::string defaultRepoPath = ".\\backup_repository";
    // 创建目录
    if (!fs::exists(defaultRepoPath)) {
        fs::create_directory(defaultRepoPath);
    }
    CBackupRecorder backupRecorder(defaultRepoPath);

    if (mode == "backup") {
        if (srcPath.empty() || dstPath.empty()) { printHelp(); return 1; }

        // // 计算实际备份路径：仓库路径 + 相对路径
        // std::string actualBackupPath = BACKUP_REPOSITORY_ROOT;
        // if (!dstPath.empty()) {
        //     fs::path dst = fs::path(dstPath);
        //     actualBackupPath = fs::absolute(fs::path(BACKUP_REPOSITORY_ROOT) / dst).string();
        // }

        // 修改一下设计，直接使用用户输入的目标路径
        std::string actualBackupPath = fs::absolute(fs::path(dstPath)).string();

        std::cout << "Source: " << srcPath << std::endl;
        // std::cout << "Relative path: " << dstPath << std::endl;
        // std::cout << "Actual backup path: " << actualBackupPath << std::endl;
        std::cout << "Destination: " << actualBackupPath << std::endl;

        // 将这里的路径设置为绝对路径
        auto config = std::make_shared<CConfig>();
        config->setSourcePath(fs::absolute(fs::path(srcPath)).string())
              .setDestinationPath(fs::absolute(fs::path(actualBackupPath)).string())
              .setRecursiveSearch(true)
              .setDescription(description);  // 设置备份行为描述

        // 判断是否需要打包
        if (packType != "none") {
            // 检查指定的打包器类型是否支持
            if (!PackFactory::isPackTypeSupported(packType)) {
                std::cerr << "Error: Packing algorithm type " << packType << " is not supported.\n";
                return 1;
            } else {
                // 设置打包器类型
                config->setPackType(packType)
                        .setPackingEnabled(true);

                // 判断是否需要压缩
                if (compressType != "none") {
                    // 检查指定的压缩器类型是否支持
                    if (!CompressFactory::isCompressTypeSupported(compressType)) {
                        std::cerr << "Error: Compress algorithm type " << compressType << " is not supported.\n";
                        return 1;
                    } else {
                        // 设置压缩器类型
                        config->setCompressionType(compressType)
                                .setCompressionEnabled(true);
                    }
                }

                // 判断是否需要加密
                if (encryptType != "none") {
                    // 检查指定的加密器类型是否支持
                    if (!EncryptFactory::isEncryptTypeSupported(encryptType)) {
                        std::cerr << "Error: Encrypt algorithm type " << encryptType << " is not supported.\n";
                        return 1;
                    } else {
                        // 判断密码是不是空的，如果是空的需要报错
                        if (encryptKey.empty()) {
                            std::cerr << "Error: Encrypt key is empty.\n";
                            return 1;
                        }
                        // 设置加密器类型
                        config->setEncryptType(encryptType)
                                .setEncryptionEnabled(true)
                                .setEncryptionKey(encryptKey);
                    }
                }
            }
        }

        // 备份执行
        if (!includeRegex.empty()) config->addIncludePattern(includeRegex);
        CBackup backup;
        std::string destPath = backup.doBackup(config);
        if (destPath.empty()) { std::cerr << "Backup failed" << std::endl; return 2; }
        std::cout << "Backup finished -> " << destPath << std::endl;
        // 备份记录
        backupRecorder.addBackupRecord(config, destPath);
        return 0;
    } else if (mode == "recover") {
        if (backupFileName.empty() || restoreTo.empty()) { printHelp(); return 1; }
        // 把目标路径转换为绝对路径
        restoreTo = fs::absolute(fs::path(restoreTo)).string();

        // 这个时候就查看有没有备份记录，如果是空的，就提示用户没有备份记录
        if (backupRecorder.getBackupRecords().empty()) {
            std::cerr << "Error: No backup records found. Please run backup first.\n";
            return 1;
        }

        // 查找备份记录中是否有匹配的文件名
        auto records = backupRecorder.findBackupRecordsByFileName(backupFileName);
        if (records.empty()) {
            std::cerr << "Error: No backup records found for file " << backupFileName
            << ". Please check the filename.\n";
            return 1;
        }

        // 可能有多个，需要让用户来选择
        BackupEntry entry;
        if (records.size() > 1) {
            // 此时进行交互式选择
            // 将全部记录进行打印出来
            std::cout << "Multiple back up records found for " << backupFileName << ":\n";
            for (size_t i = 0; i < records.size(); ++i) {
                // 现在先主要显示原先的备份路径和备份时间，还要展示是否有打包、压缩、加密等操作
                // 新增备份行为描述字段
                std::cout << "[" << i << "] " << records[i].fileName << " @ "
                << records[i].backupTime << " (Pack: " << records[i].isPacked << ", Compress: "
                << records[i].isCompressed << ", Encrypt: " << records[i].isEncrypted << ", Desc: "
                << utf8_to_gbk(records[i].description) << ")" << std::endl;
            }
            // 让用户选择
            int choice;
            while (true) {
                std::cout << "Please select the backup record to recover (0-" << records.size() - 1 << "): ";
                std::cin >> choice;
                if (choice < 0 || choice >= records.size()) {
                    std::cerr << "Invalid choice. Please select a number between 0 and " << records.size() - 1 << ".\n";
                } else {
                    break;
                }
            }
            entry = records[choice];
        } else {
            // 此时直接选择第一个
            entry = records[0];
        }
        // 执行恢复
        CBackup backup;
        bool success = backup.doRecovery(entry, restoreTo);
        if (!success) {
            std::cerr << "Recovery failed" << std::endl;
            return 2;
        }
        std::cout << "Recovery finished -> " << restoreTo << std::endl;
        return 0;
    }
    printHelp();
    return 1;
}

int main() {
    std::cout << "Backup interactive CLI. Type 'help' for usage, 'demo' for example, 'exit' to quit." << std::endl;
    printHelp();
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        // 去掉首尾空白
        auto trim = [](const std::string& s){ size_t a = s.find_first_not_of(" \t\r\n");
            size_t b = s.find_last_not_of(" \t\r\n");
            if (a == std::string::npos) return std::string();
            return s.substr(a, b-a+1); };
        line = trim(line);
        if (line.empty()) continue;
        if (line == "exit" || line == "quit") break;
        if (line == "help" || line == "--help" || line == "-h") { printHelp(); continue; }
        auto tokens = tokenize(line);
        if (tokens.empty()) continue;
        runParsed(tokens);
    }
    return 0;
}
