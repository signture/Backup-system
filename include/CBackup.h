// Copyright [2025] <JiJun Lu, Linru Zhou>
#ifndef INCLUDE_CBACKUP_H_
#define INCLUDE_CBACKUP_H_

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <set>
#include <fstream>
#include <iostream>
#include "Utils.h"

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
    std::string doBackup(const std::shared_ptr<CConfig>& config);

    // 恢复相关
    bool doRecovery(const BackupEntry& entry, const std::string& destDir);
    // 重载：接收外部提供的密码（GUI 情况下传入），若传入空串则回退到控制台交互以保持 CLI 兼容
    bool doRecovery(const BackupEntry& entry, const std::string& destDir, const std::string& password);


 private:
    std::set<std::string> createdDirs;  // 用于记录已创建的目录，避免重复创建
};

std::vector<std::string> collectFilesToBackup(const std::string& rootPath, const std::shared_ptr<CConfig>& config);


#endif  // INCLUDE_CBACKUP_H_
