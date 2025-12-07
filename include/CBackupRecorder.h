#ifndef CBACKUPRECORDER_H
#define CBACKUPRECORDER_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "CConfig.h"

// 定义结构体，用于记录备份记录
struct BackupEntry {
    std::string fileName;        // 源文件名
    std::string sourceFullPath;  // 源文件完整路径
    std::string destDirectory;   // 备份目标目录
    std::string backupFileName;  // 最终备份文件名
    std::string backupTime;      // 备份时间
    bool isEncrypted;            // 是否加密
    bool isPacked;               // 是否打包
    bool isCompressed;           // 是否压缩
    std::string description;     // 备份描述
    
    // 默认构造函数
    BackupEntry() = default;
    
    // 完整构造函数
    BackupEntry(const std::string& fn, const std::string& sfp, const std::string& dd, 
                const std::string& bfn, const std::string& bt, bool ie, bool ip, bool ic, const std::string& desc)
        : fileName(fn), sourceFullPath(sfp), destDirectory(dd), backupFileName(bfn), 
          backupTime(bt), isEncrypted(ie), isPacked(ip), isCompressed(ic), description(desc) {}
    
    // 为了向后兼容，添加destPath别名
    std::string& destPath() { return destDirectory; }
    const std::string& destPath() const { return destDirectory; }
};

// 为 BackupEntry 提供 nlohmann/json 所需的序列化支持
namespace nlohmann {
    template <>
    struct adl_serializer<BackupEntry> {
        static void to_json(nlohmann::json& j, const BackupEntry& entry) {
            j = nlohmann::json{{"file_name", entry.fileName},
                     {"source_full_path", entry.sourceFullPath},
                     {"destination_directory", entry.destDirectory},
                     {"backup_file_name", entry.backupFileName},
                     {"backup_time", entry.backupTime},
                     {"is_encrypted", entry.isEncrypted},
                     {"is_packed", entry.isPacked},
                     {"is_compressed", entry.isCompressed},
                     {"description", entry.description}};
        }

        static void from_json(const nlohmann::json& j, BackupEntry& entry) {
            j.at("file_name").get_to(entry.fileName);
            j.at("source_full_path").get_to(entry.sourceFullPath);
            j.at("destination_directory").get_to(entry.destDirectory);
            j.at("backup_file_name").get_to(entry.backupFileName);
            j.at("backup_time").get_to(entry.backupTime);
            j.at("is_encrypted").get_to(entry.isEncrypted);
            j.at("is_packed").get_to(entry.isPacked);
            j.at("is_compressed").get_to(entry.isCompressed);
            j.at("description").get_to(entry.description);
        }
    };
}

// 重载相等运算符，用于比较两个BackupEntry对象
inline bool operator==(const BackupEntry& lhs, const BackupEntry& rhs) {
    return lhs.fileName == rhs.fileName && lhs.backupTime == rhs.backupTime;
}

class CBackupRecorder {
public:
    CBackupRecorder();
    CBackupRecorder(bool autoSave);
    CBackupRecorder(const std::string& filePath);
    ~CBackupRecorder();

    // 从文件中加载备份目录（这里假定程序有一个固定的备份记录文件）
    bool loadBackupRecordsFromFile(const std::string& filePath);

    // 保存备份记录
    bool saveBackupRecordsToFile(const std::string& filePath);

    // 添加备份记录
    void addBackupRecord(const BackupEntry& entry);

    // 获取备份记录
    const std::vector<BackupEntry>& getBackupRecords() const;

    // 根据文件名查找备份记录
    std::vector<BackupEntry> findBackupRecordsByFileName(const std::string& queryFileName) const;

    // 根据备份时间查找条目
    std::vector<BackupEntry> findBackupRecordsByBackupTime(const std::string& startime, const std::string& endTime) const;

    // 获取备份条目的全局索引
    size_t getBackupRecordIndex(const BackupEntry& entry) const;

    // 根据索引删除备份记录
    bool deleteBackupRecord(size_t index);

    // 根据备份记录删除条目
    bool deleteBackupRecord(const BackupEntry& entry);

    // 修改备份记录
    bool modifyBackupRecord(size_t index, const BackupEntry& newEntry);

    // 检查索引是否有效
    bool isIndexValid(size_t index) const;

    bool modifyBackupRecord(const BackupEntry& oldEntry, const BackupEntry& newEntry);

    // 获取默认的备份记录文件路径
    std::string getRecorderFilePath() const;

    // 增加备份记录
    void addBackupRecord(const std::shared_ptr<CConfig>& config, const std::string& destPath);

private:
    std::vector<BackupEntry> backupRecords; // 备份记录容器
    std::string recorderFilePath; // 备份记录文件路径
    bool autoSaveEnabled; // 是否自动保存,默认为false
};

#endif
