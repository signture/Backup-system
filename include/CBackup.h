#ifndef CBACKUP_H
#define CBACKUP_H

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <set>  
<<<<<<< HEAD
=======
#include <fstream>
#include <iostream>
#include "Utils.h"

>>>>>>> encryptFunction
#include "CConfig.h"
#include "CBackupRecorder.h"

#include "PackFactory.h"  
#include "CompressFactory.h"
#include "EncryptFactory.h"
namespace fs = std::filesystem; 



class CBackup {
public: 
    CBackup();
    ~CBackup();

    // 拓展接口
    // 备份相关
<<<<<<< HEAD
    bool doBackup(const std::shared_ptr<CConfig>& config);

    // 恢复相关
    bool doRecovery(const BackupEntry& entry);
=======
    std::string doBackup(const std::shared_ptr<CConfig>& config);

    // 恢复相关
    bool doRecovery(const BackupEntry& entry, const std::string& destDir);

>>>>>>> encryptFunction

private:
    std::set<std::string> createdDirs;  // 用于记录已创建的目录，避免重复创建

};

<<<<<<< HEAD
// 辅助函数
bool ReadFile(const std::string& filePath, std::vector<char>& buffer);
bool WriteFile(const std::string& filePath, std::vector<char>& buffer);
// 避免与 Windows API 宏 CopyFile 冲突
bool CopyFileBinary(const std::string& srcPath, const std::string& destPath);
=======
std::vector<std::string> collectFilesToBackup(const std::string& rootPath, const std::shared_ptr<CConfig>& config);

>>>>>>> encryptFunction

#endif //CBACKUP_H