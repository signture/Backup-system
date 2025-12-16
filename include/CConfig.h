#ifndef CCONFIG_H
#define CCONFIG_H

// 引入必要的标准库头文件
#include <string>
#include <vector>
#include <regex>
#include <memory>
#include <map>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "Utils.h"
/**
 * @brief 配置类，负责存储和管理备份系统的所有配置项
 * @details 涵盖源路径、目标路径、文件筛选、备份行为（打包/压缩/加密）等配置，提供完整的 setter/getter 接口
 */
class CConfig {
public:
    // ===== 构造函数与析构函数 =====
    /** 默认构造函数，初始化默认配置 */
    CConfig();
    
    /** 
     * 完整构造函数，指定单个源路径和目标路径
     * @param sourcePath 单个源文件/目录路径
     * @param destinationPath 备份目标路径
     */
    CConfig(const std::string& sourcePath, const std::string& destinationPath);
    
    /** 析构函数（空实现，无动态资源需手动释放） */
    ~CConfig();
    
    // ===== 拷贝与移动语义 =====
    /** 允许拷贝构造 */
    CConfig(const CConfig&) = default;
    /** 允许拷贝赋值 */
    CConfig& operator=(const CConfig&) = default;
    /** 允许移动构造 */
    CConfig(CConfig&&) noexcept = default;
    /** 允许移动赋值 */
    CConfig& operator=(CConfig&&) noexcept = default;
    
    // ===== 基本配置接口（源路径/目标路径） =====
    /**
     * 设置单个源路径（覆盖原有源路径）
     * @param path 源文件/目录路径（非空）
     * @return 返回自身引用，支持链式调用
     * @throw std::invalid_argument 若路径为空
     */
    CConfig& setSourcePath(const std::string& path);
    
    /**
     * 获取单个源路径
     * @return 源路径（const 引用，避免拷贝）
     */
    const std::string& getSourcePath() const;
    
    /**
     * 添加多个源路径（追加到现有列表，不覆盖）
     * @param path 新增的源文件/目录路径（非空）
     */
    void addSourcePath(const std::string& path);
    
    /**
     * 获取所有源路径列表
     * @return 源路径列表（const 引用，避免拷贝）
     */
    const std::vector<std::string>& getSourcePaths() const;
    
    /**
     * 设置备份目标路径
     * @param path 目标路径（非空）
     * @return 返回自身引用，支持链式调用
     * @throw std::invalid_argument 若路径为空
     */
    CConfig& setDestinationPath(const std::string& path);
    
    /**
     * 获取备份目标路径
     * @return 目标路径（const 引用，避免拷贝）
     */
    const std::string& getDestinationPath() const;
    
    // ===== 文件筛选配置接口 =====
    /**
     * 设置是否递归搜索源目录下的子目录
     * @param value true=递归，false=不递归（默认false）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setRecursiveSearch(bool value);
    
    /**
     * 获取是否递归搜索的配置
     * @return true=递归，false=不递归
     */
    bool isRecursiveSearch() const;
    
    /**
     * 设置是否跟随符号链接（避免循环引用风险）
     * @param value true=跟随，false=不跟随（默认false）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setFollowSymlinks(bool value);
    
    /**
     * 获取是否跟随符号链接的配置
     * @return true=跟随，false=不跟随
     */
    bool isFollowSymlinks() const;
    
    /**
     * 添加文件包含模式（正则表达式，匹配的文件才备份）
     * @param pattern 正则表达式（如 ".*\\.txt" 匹配所有txt文件）
     * @return 返回自身引用，支持链式调用
     * @note 无效正则会打印警告并忽略
     */
    CConfig& addIncludePattern(const std::string& pattern);
    
    /**
     * 获取所有文件包含模式
     * @return 包含模式列表（const 引用，避免拷贝）
     */
    const std::vector<std::regex>& getIncludePatterns() const;
    
    /**
     * 添加文件排除模式（正则表达式，匹配的文件不备份）
     * @param pattern 正则表达式（如 ".*\\.log" 排除所有log文件）
     * @return 返回自身引用，支持链式调用
     * @note 无效正则会打印警告并忽略
     */
    CConfig& addExcludePattern(const std::string& pattern);
    
    /**
     * 获取所有文件排除模式
     * @return 排除模式列表（const 引用，避免拷贝）
     */
    const std::vector<std::regex>& getExcludePatterns() const;
    
    // ===== 备份行为配置接口（打包/压缩/加密） =====
    /**
     * 设置是否启用打包（如tar打包）
     * @param value true=启用，false=禁用（默认false）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setPackingEnabled(bool value);
    
    /**
     * 获取是否启用打包的配置
     * @return true=启用，false=禁用
     */
    bool isPackingEnabled() const;
    
    /**
     * 设置打包类型（如 "tar"、"zip"）
     * @param type 打包类型字符串（默认 "tar"）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setPackType(const std::string& type);
    
    /**
     * 获取打包类型
     * @return 打包类型（const 引用，避免拷贝）
     */
    const std::string& getPackType() const;
    
    /**
     * 设置是否启用压缩（如gzip压缩）
     * @param value true=启用，false=禁用（默认false）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setCompressionEnabled(bool value);
    
    /**
     * 获取是否启用压缩的配置
     * @return true=启用，false=禁用
     */
    bool isCompressionEnabled() const;
    
    /**
     * 设置压缩类型（如 "gzip"、"zip"）
     * @param type 压缩类型字符串（默认 "gzip"）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setCompressionType(const std::string& type);
    
    /**
     * 获取压缩类型
     * @return 压缩类型（const 引用，避免拷贝）
     */
    const std::string& getCompressionType() const;
    
    /**
     * 设置压缩级别（1-9，级别越高压缩率越高但速度越慢）
     * @param level 压缩级别（1-9，默认1）
     * @return 返回自身引用，支持链式调用
     * @throw std::invalid_argument 若级别超出1-9范围
     */
    CConfig& setCompressionLevel(int level);
    
    /**
     * 获取压缩级别
     * @return 压缩级别（1-9）
     */
    int getCompressionLevel() const;
    
    /**
     * 设置是否启用加密（备份文件加密存储）
     * @param value true=启用，false=禁用（默认false）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setEncryptionEnabled(bool value);
    
    /**
     * 获取是否启用加密的配置
     * @return true=启用，false=禁用
     */
    bool isEncryptionEnabled() const;
    
    /**
     * 设置加密密钥（启用加密时必须设置）
     * @param key 加密密钥字符串
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setEncryptionKey(const std::string& key);
    
    /**
     * 获取加密密钥
     * @return 加密密钥（const 引用，避免拷贝）
     */
    const std::string& getEncryptionKey() const;

    /**
     * 设置加密类型（启用加密时必须设置）
     * @param type 加密类型字符串（默认 "SimXOR"）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setEncryptType(const std::string& type);

     /**
     * 获取加密类型
     * @return 加密类型（const 引用，避免拷贝）
     */
    const std::string& getEncryptType() const;

     /**
     * 设置备份行为描述（如 "手动备份"）
     * @param desc 备份行为描述字符串（默认空字符串）
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setDescription(const std::string& desc);
    
    /**
     * 获取备份行为描述
     * @return 备份行为描述（const 引用，避免拷贝）
     */
    const std::string& getDescription() const;
    
    // ===== 高级配置接口（自定义选项） =====
    /**
     * 设置自定义配置项（键值对，如 "timeout=30"）
     * @param key 配置项键名（非空）
     * @param value 配置项值
     * @return 返回自身引用，支持链式调用
     */
    CConfig& setCustomOption(const std::string& key, const std::string& value);
    
    /**
     * 获取自定义配置项
     * @param key 配置项键名
     * @param defaultValue 键不存在时的默认值（默认空字符串）
     * @return 配置项值（若键不存在，返回默认值）
     */
    std::string getCustomOption(const std::string& key, const std::string& defaultValue = "") const;
    
    // ===== 便捷工具方法 =====
    /**
     * 检查文件是否符合筛选规则（应被包含在备份中）
     * @param filePath 待检查的文件路径
     * @return true=符合规则（应备份），false=不符合（应排除）
     */
    bool shouldIncludeFile(const std::string& filePath) const;
    
    /**
     * 验证当前配置的有效性（必要参数是否完整、路径是否存在等）
     * @return true=配置有效，false=配置无效（并打印错误信息）
     */
    bool isValid() const;
    
    /**
     * 重置配置为默认状态（清空所有路径和选项，恢复初始值）
     */
    void reset();
    
    /**
     * 深拷贝当前配置（返回智能指针，自动管理内存）
     * @return 新的配置对象智能指针
     */
    std::shared_ptr<CConfig> clone() const;
    
    /**
     * 生成配置摘要字符串（便于日志输出或调试）
     * @return 配置摘要（包含所有关键配置项）
     */
    std::string toString() const;

private:
    // ===== 成员变量（统一加 m_ 前缀） =====
    // 基本配置
    std::string m_sourcePath;                  // 单个源路径（兼容旧逻辑）
    std::vector<std::string> m_sourcePaths;    // 多个源路径列表（支持批量备份）
    std::string m_destinationPath;             // 备份目标路径
    
    // 文件筛选配置
    bool m_recursiveSearch = false;            // 是否递归搜索子目录
    bool m_followSymlinks = false;             // 是否跟随符号链接
    std::vector<std::regex> m_includePatterns; // 文件包含模式（正则列表）
    std::vector<std::regex> m_excludePatterns; // 文件排除模式（正则列表）
    
    // 备份行为配置
    bool m_enablePacking = false;              // 是否启用打包
    std::string m_packType = "tar";            // 打包类型（默认 tar）
    bool m_enableCompression = false;          // 是否启用压缩
    std::string m_compressionType = "gzip";    // 压缩类型（默认 gzip）
    int m_compressionLevel = 1;                // 压缩级别（默认 1，1-9）
    bool m_enableEncryption = false;           // 是否启用加密
    std::string m_encryptionKey;               // 加密密钥
    std::string m_encryptType = "SimXOR";      // 加密类型（默认 SimXOR）

    // 备份行为描述配置
    std::string m_description = "";                // 备份行为描述
    
    // 高级配置
    std::map<std::string, std::string> m_customOptions; // 自定义键值对配置
};

#endif // CCONFIG_H