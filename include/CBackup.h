#ifndef CBACKUP_H
#define CBACKUP_H

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


private:
    std::set<std::string> createdDirs;  // 用于记录已创建的目录，避免重复创建

};

std::vector<std::string> collectFilesToBackup(const std::string& rootPath, const std::shared_ptr<CConfig>& config);


#endif //CBACKUP_H