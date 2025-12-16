<<<<<<< HEAD
#include <iostream>
=======
﻿#include <iostream>
>>>>>>> encryptFunction
#include <filesystem>
#include <fstream>
#include <vector>

#include "CBackup.h"
#include "CConfig.h"
<<<<<<< HEAD

namespace fs = std::filesystem;

static bool writeTextFile(const std::string& path, const std::string& content) {
    try {
        fs::create_directories(fs::path(path).parent_path());
        std::ofstream out(path, std::ios::binary);
        if (!out) return false;
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        return true;
    } catch (...) {
        return false;
    }
}

static bool readFileAll(const std::string& path, std::vector<char>& buf) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    in.seekg(0, std::ifstream::end);
    const auto sz = in.tellg();
    in.seekg(0);
    buf.resize(static_cast<size_t>(sz));
    return static_cast<bool>(in.read(buf.data(), sz));
}

// 全局备份仓库路径配置
static std::string BACKUP_REPOSITORY_ROOT = "./backup_repository";
=======
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
    if (wlen == 0) return utf8_str; // 转换失败返回原字符串
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

// 全局备份仓库路径配置
static std::string BACKUP_REPOSITORY_ROOT = ".\\backup_repository";
>>>>>>> encryptFunction

//--mode backup --src "F:\courser_project\software_development\testBox\ori_A" --dst "./"
//--mode recover --dst "./" --to "./restore_repository/ori_A"

static void printHelp(){
    std::cout << "Usage (pseudo CLI):\n"
<<<<<<< HEAD
              << "--mode backup --src <path> --dst <relative_path> [--include \".*\\.txt\"]\n"
              << "--mode recover --dst <relative_path> --to <target_path>\n";
}

// 目录恢复函数：递归恢复整个目录结构
static bool recoverDirectory(CBackup& backup, const std::string& backupRoot, 
                           const std::string& backupDirPath, const std::string& targetPath) {
    try {
        // 创建目标目录
        fs::create_directories(targetPath);
        
        // 遍历备份目录中的所有文件
        std::string fullBackupDirPath = (fs::path(backupRoot) / backupDirPath).string();
        for (auto it = fs::recursive_directory_iterator(fullBackupDirPath); 
             it != fs::recursive_directory_iterator(); ++it) {
            
            if (it->is_regular_file()) {
                // 计算相对路径
                std::string relativePath = fs::relative(it->path(), fullBackupDirPath).string();
                std::string targetFilePath = (fs::path(targetPath) / relativePath).string();
                
                // 创建目标文件的父目录
                fs::create_directories(fs::path(targetFilePath).parent_path());
                
                // 恢复单个文件
                BackupEntry entry;
                entry.fileName = it->path().filename().string();
                entry.sourceFullPath = targetFilePath;
                entry.destDirectory = backupRoot;
                entry.backupFileName = (fs::path(backupDirPath) / relativePath).string();
                entry.backupTime = "";
                entry.isEncrypted = false;
                entry.isPacked = false;
                entry.isCompressed = false;
                
                if (!backup.doRecovery(entry)) {
                    std::cerr << "Failed to recover file: " << relativePath << std::endl;
                    return false;
                }
                
                std::cout << "  Recovered: " << relativePath << std::endl;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during directory recovery: " << e.what() << std::endl;
        return false;
    }
}
=======
              << "--mode backup  --src <path> --dst <relative_path> [--include \".*\\.txt\"  --pack <packType>(default: none)\n  --compress <compressType>(default: none)  --encrypt <encryptType>(default: none)  --key <encryptKey>  --desc <description>]\n"
              << "--mode recover --fn <filename> --to <target_path>\n";
}

>>>>>>> encryptFunction

static std::vector<std::string> tokenize(const std::string& line){
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

static int runParsed(const std::vector<std::string>& args){
    std::string mode;
<<<<<<< HEAD
=======
    std::string packType = "none";  // 新增加一个参数用于指定打包算法,默认不打包
    std::string compressType = "none";  // 新增加一个参数用于指定压缩算法,默认不压缩
    std::string encryptType = "none";  // 新增加一个参数用于指定加密算法,默认不加密
    std::string encryptKey;
>>>>>>> encryptFunction
    std::string srcPath;
    std::string dstPath;
    std::string includeRegex;
    std::string restoreTo;
    std::string repoPath;
<<<<<<< HEAD
=======
    std::string description = "";  // 新增一个参数用于指定备份行为描述,默认空字符串
>>>>>>> encryptFunction

    auto nextVal = [&](size_t& i, std::string& out){ if (i + 1 < args.size()) { out = args[++i]; return true; } return false; };
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "--mode") { nextVal(i, mode); }
<<<<<<< HEAD
=======
        else if (arg == "--pack") { nextVal(i, packType); }
        else if (arg == "--encrypt") { nextVal(i, encryptType); }
        else if (arg == "--compress") { nextVal(i, compressType); }
        else if (arg == "--key") { nextVal(i, encryptKey); }
>>>>>>> encryptFunction
        else if (arg == "--src") { nextVal(i, srcPath); }
        else if (arg == "--dst") { nextVal(i, dstPath); }
        else if (arg == "--include") { nextVal(i, includeRegex); }
        else if (arg == "--to") { nextVal(i, restoreTo); }
        else if (arg == "--repo") { nextVal(i, repoPath); }
        else if (arg == "--help" || arg == "-h") { printHelp(); return 0; }
<<<<<<< HEAD
    }
    
    // 设置备份仓库路径
=======
        else if (arg == "--desc") { nextVal(i, description); }  // 新增参数处理
    }
    
    // 设置备份仓库路径  -- 这里目前的唯一功能是用于设置备份记录的路径
>>>>>>> encryptFunction
    if (!repoPath.empty()) {
        BACKUP_REPOSITORY_ROOT = repoPath;
        std::cout << "Backup repository set to: " << BACKUP_REPOSITORY_ROOT << std::endl;
        return 0;
    }

<<<<<<< HEAD
    if (mode == "backup") {
        if (srcPath.empty() || dstPath.empty()) { printHelp(); return 1; }
        
        // 计算实际备份路径：仓库路径 + 相对路径 + 源文件夹名（如果是目录）
        std::string actualBackupPath = BACKUP_REPOSITORY_ROOT;
        if (!dstPath.empty()) {
            actualBackupPath = (fs::path(BACKUP_REPOSITORY_ROOT) / dstPath).string();
        }
        
        // 如果源路径是目录，添加源目录名
        if (fs::is_directory(srcPath)) {
            std::string sourceDirName = fs::path(srcPath).filename().string();
            actualBackupPath = (fs::path(actualBackupPath) / sourceDirName).string();
        }
        
        std::cout << "Source: " << srcPath << std::endl;
        std::cout << "Relative path: " << dstPath << std::endl;
        std::cout << "Actual backup path: " << actualBackupPath << std::endl;
        
        auto config = std::make_shared<CConfig>();
        config->setSourcePath(srcPath)
              .setDestinationPath(actualBackupPath)
              .setRecursiveSearch(true);
        if (!includeRegex.empty()) config->addIncludePattern(includeRegex);
        CBackup backup;
        if (!backup.doBackup(config)) { std::cerr << "Backup failed" << std::endl; return 2; }
        std::cout << "Backup finished -> " << actualBackupPath << std::endl;
        return 0;
    } else if (mode == "recover") {
        if (dstPath.empty() || restoreTo.empty()) { printHelp(); return 1; }
        
        // 计算实际备份路径：仓库路径 + 相对路径
        std::string actualBackupPath = (fs::path(BACKUP_REPOSITORY_ROOT) / dstPath).string();
        
        std::cout << "Relative path: " << dstPath << std::endl;
        std::cout << "Actual backup path: " << actualBackupPath << std::endl;
        
        // 智能恢复：支持文件和目录恢复
        fs::path targetPath(restoreTo);
        std::string targetName = targetPath.filename().string();
        
        // 收集备份目录中的所有文件和目录信息
        std::vector<std::string> allFiles;
        std::vector<std::string> allDirs;
        try {
            for (auto it = fs::recursive_directory_iterator(actualBackupPath); it != fs::recursive_directory_iterator(); ++it) {
                std::string relativePath = fs::relative(it->path(), actualBackupPath).string();
                if (it->is_regular_file()) {
                    allFiles.push_back(relativePath);
                } else if (it->is_directory()) {
                    allDirs.push_back(relativePath);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error searching backup directory: " << e.what() << std::endl;
            return 2;
        }
        
        // 检查是否要恢复整个目录
        bool isDirectoryRecovery = false;
        std::string foundBackupPath;
        
        // 首先检查是否是目录恢复（目标名称在备份目录中作为目录存在）
        for (const auto& dir : allDirs) {
            if (fs::path(dir).filename().string() == targetName) {
                isDirectoryRecovery = true;
                foundBackupPath = dir;
                break;
            }
        }
        
        // 如果不是目录恢复，检查是否是文件恢复
        if (!isDirectoryRecovery) {
            for (const auto& file : allFiles) {
                if (fs::path(file).filename().string() == targetName) {
                    foundBackupPath = file;
                    break;
                }
            }
        }
        
        if (foundBackupPath.empty()) {
            std::cerr << "Error: '" << targetName << "' not found in backup directory" << std::endl;
            std::cerr << "Available items in backup directory:" << std::endl;
            std::cerr << "Files:" << std::endl;
            for (const auto& file : allFiles) {
                std::cerr << "  " << file << std::endl;
            }
            std::cerr << "Directories:" << std::endl;
            for (const auto& dir : allDirs) {
                std::cerr << "  " << dir << std::endl;
            }
            return 2;
        }
        
        // 执行恢复
        CBackup backup;
        bool success = false;
        
        if (isDirectoryRecovery) {
            // 目录恢复：恢复整个目录结构
            std::cout << "Recovering directory: " << targetName << std::endl;
            success = recoverDirectory(backup, actualBackupPath, foundBackupPath, restoreTo);
        } else {
            // 文件恢复：恢复单个文件
            std::cout << "Recovering file: " << targetName << std::endl;
            BackupEntry entry;
            entry.fileName = targetName;
            entry.sourceFullPath = restoreTo;
            entry.destDirectory = actualBackupPath;
            entry.backupFileName = foundBackupPath;
            entry.backupTime = "";
            entry.isEncrypted = false;
            entry.isPacked = false;
            entry.isCompressed = false;
            
            success = backup.doRecovery(entry);
        }
=======
    // 创建备份记录器
    CBackupRecorder backupRecorder(BACKUP_REPOSITORY_ROOT);

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
        if(packType != "none"){
            // 检查指定的打包器类型是否支持
            if(!PackFactory::isPackTypeSupported(packType)){
                std::cerr << "Error: Packing algorithm type " << packType << " is not supported.\n";
                return 1;
            }
            else{
                // 设置打包器类型
                config->setPackType(packType)
                        .setPackingEnabled(true);

                // 判断是否需要压缩
                if(compressType != "none"){
                    // 检查指定的压缩器类型是否支持
                    if(!CompressFactory::isCompressTypeSupported(compressType)){
                        std::cerr << "Error: Compress algorithm type " << compressType << " is not supported.\n";
                        return 1;
                    }
                    else{
                        // 设置压缩器类型
                        config->setCompressionType(compressType)
                                .setCompressionEnabled(true);
                    }
                }

                // 判断是否需要加密
                if(encryptType != "none"){
                    // 检查指定的加密器类型是否支持
                    if(!EncryptFactory::isEncryptTypeSupported(encryptType)){
                        std::cerr << "Error: Encrypt algorithm type " << encryptType << " is not supported.\n";
                        return 1;
                    }
                    else{
                        // 判断密码是不是空的，如果是空的需要报错
                        if(encryptKey.empty()){
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
        if (dstPath.empty() || restoreTo.empty()) { printHelp(); return 1; }
        // 把目标路径转换为绝对路径
        restoreTo = fs::absolute(fs::path(restoreTo)).string();
        
        // 这个时候就查看有没有备份记录，如果是空的，就提示用户没有备份记录
        if(backupRecorder.getBackupRecords().empty()){
            std::cerr << "Error: No backup records found. Please run backup first.\n";
            return 1;
        }

        // 查找备份记录中是否有匹配的文件名
        auto records = backupRecorder.findBackupRecordsByFileName(dstPath);
        if(records.empty()){
            std::cerr << "Error: No backup records found for file " << dstPath << ". Please check the filename.\n";
            return 1;
        }

        // 可能有多个，需要让用户来选择
        BackupEntry entry; 
        if(records.size() > 1){
            // 此时进行交互式选择
            // 将全部记录进行打印出来
            std::cout << "Multiple back up records found for " << dstPath << ":\n";
            for(size_t i = 0; i < records.size(); ++i){
                // 现在先主要显示原先的备份路径和备份时间，还要展示是否有打包、压缩、加密等操作
                // 新增备份行为描述字段
                std::cout << "[" << i << "] " << records[i].fileName << " @ " << records[i].backupTime << " (Pack: " << records[i].isPacked << ", Compress: " << records[i].isCompressed << ", Encrypt: " << records[i].isEncrypted << ", Desc: " << utf8_to_gbk(records[i].description) << ")" << std::endl;
            }
            // 让用户选择
            int choice;
            while(true){
                std::cout << "Please select the backup record to recover (0-" << records.size() - 1 << "): ";
                std::cin >> choice;
                if (choice < 0 || choice >= records.size()) {
                    std::cerr << "Invalid choice. Please select a number between 0 and " << records.size() - 1 << ".\n";
                }
                else{
                    break;
                }
            }
            entry = records[choice];
        }
        else{
            // 此时直接选择第一个
            entry = records[0];
        }
            
        // 执行恢复
        CBackup backup;
        bool success = backup.doRecovery(entry, restoreTo);
>>>>>>> encryptFunction
        
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
<<<<<<< HEAD

static int runDemo(){
    std::cout << "[Demo] Backup Repository & Recover flow" << std::endl;
    std::cout << "Creating demo test files and directories..." << std::endl;
    
    const std::string demoRoot = "demo_src";
    const std::string fileA = (fs::path(demoRoot) / "fileA.txt").string();
    const std::string subDir = (fs::path(demoRoot) / "sub").string();
    const std::string fileB = (fs::path(subDir) / "fileB.txt").string();
    const std::string fileC = (fs::path(subDir) / "fileC.txt").string();
    const std::string restoreRoot = "demo_restore";
    
    // 清理旧文件（如果存在）
    std::cout << "Cleaning up previous demo files..." << std::endl;
    fs::remove_all(demoRoot); 
    fs::remove_all(BACKUP_REPOSITORY_ROOT); 
    fs::remove_all(restoreRoot);
    
    // 创建测试文件结构
    std::cout << "Creating demo source files..." << std::endl;
    writeTextFile(fileA, "Hello Backup A\n"); 
    fs::create_directories(subDir); 
    writeTextFile(fileB, "Hello Backup B\n");
    writeTextFile(fileC, "Hello Backup C\n");
    std::cout << "Demo files created successfully!" << std::endl;
    
    // 执行备份（使用相对路径）
    std::string relativePath = "test_backup";
    std::string actualBackupPath = (fs::path(BACKUP_REPOSITORY_ROOT) / relativePath / "demo_src").string();
    
    std::cout << "Repository: " << BACKUP_REPOSITORY_ROOT << std::endl;
    std::cout << "Relative path: " << relativePath << std::endl;
    std::cout << "Actual backup path: " << actualBackupPath << std::endl;
    
    auto config = std::make_shared<CConfig>();
    config->setSourcePath(demoRoot).setDestinationPath(actualBackupPath).setRecursiveSearch(true);
    CBackup backup;
    if (!backup.doBackup(config)) { std::cerr << "Backup failed" << std::endl; return 1; }
    std::cout << "Backup finished -> " << actualBackupPath << std::endl;
    
    // 测试文件恢复
    std::cout << "\n=== Testing File Recovery ===" << std::endl;
    const std::string restoreFile = (fs::path(restoreRoot) / "fileA.txt").string();
    BackupEntry entry; 
    entry.fileName = "fileA.txt"; 
    entry.sourceFullPath = restoreFile; 
    entry.destDirectory = actualBackupPath; 
    entry.backupFileName = "fileA.txt"; 
    entry.backupTime = ""; 
    entry.isEncrypted = false; 
    entry.isPacked = false; 
    entry.isCompressed = false;
    
    if (!backup.doRecovery(entry)) { std::cerr << "File recovery failed" << std::endl; return 2; }
    std::cout << "File recovery finished -> " << restoreFile << std::endl;
    
    // 验证文件恢复
    std::vector<char> aBuf, rBuf; 
    if (!readFileAll(fileA, aBuf) || !readFileAll(restoreFile, rBuf) || aBuf != rBuf) { 
        std::cerr << "File verify failed: restored file mismatch" << std::endl; 
        return 3; 
    }
    std::cout << "File verify OK" << std::endl;
    
    // 测试目录恢复
    std::cout << "\n=== Testing Directory Recovery ===" << std::endl;
    const std::string restoreDir = (fs::path(restoreRoot) / "sub").string();
    if (!recoverDirectory(backup, actualBackupPath, "sub", restoreDir)) {
        std::cerr << "Directory recovery failed" << std::endl;
        return 4;
    }
    std::cout << "Directory recovery finished -> " << restoreDir << std::endl;
    
    // 验证目录恢复
    const std::string restoredFileB = (fs::path(restoreDir) / "fileB.txt").string();
    const std::string restoredFileC = (fs::path(restoreDir) / "fileC.txt").string();
    
    std::vector<char> bBuf, rbBuf, cBuf, rcBuf;
    if (!readFileAll(fileB, bBuf) || !readFileAll(restoredFileB, rbBuf) || bBuf != rbBuf) {
        std::cerr << "Directory verify failed: fileB mismatch" << std::endl;
        return 5;
    }
    if (!readFileAll(fileC, cBuf) || !readFileAll(restoredFileC, rcBuf) || cBuf != rcBuf) {
        std::cerr << "Directory verify failed: fileC mismatch" << std::endl;
        return 6;
    }
    std::cout << "Directory verify OK" << std::endl;
    
    std::cout << "\n=== All tests passed! ===" << std::endl;
    std::cout << "Backup repository structure:" << std::endl;
    std::cout << "  " << BACKUP_REPOSITORY_ROOT << "/" << relativePath << "/demo_src/" << std::endl;
    std::cout << "\nDemo completed. Demo files will be cleaned up on next demo run." << std::endl;
    return 0;
}
=======
>>>>>>> encryptFunction

int main(){
    std::cout << "Backup interactive CLI. Type 'help' for usage, 'demo' for example, 'exit' to quit." << std::endl;
    printHelp();
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        // 去掉首尾空白
        auto trim = [](std::string s){ size_t a = s.find_first_not_of(" \t\r\n"); size_t b = s.find_last_not_of(" \t\r\n"); if (a==std::string::npos) return std::string(); return s.substr(a, b-a+1); };
        line = trim(line);
        if (line.empty()) continue;
        if (line == "exit" || line == "quit") break;
        if (line == "help" || line == "--help" || line == "-h") { printHelp(); continue; }
<<<<<<< HEAD
        if (line == "demo") { runDemo(); continue; }
=======
>>>>>>> encryptFunction
        auto tokens = tokenize(line);
        if (tokens.empty()) continue;
        runParsed(tokens);
    }
    return 0;
}