// Copyright [2025] <JiJun Lu, Linru Zhou>
#include "CConfig.h"

// ===== 构造函数与析构函数实现 =====
CConfig::CConfig() {
    // 调用 reset 初始化默认配置
    reset();
}

CConfig::CConfig(const std::string& sourcePath, const std::string& destinationPath) {
    reset();
    setSourcePath(sourcePath);    // 初始化单个源路径
    setDestinationPath(destinationPath);  // 初始化目标路径
}

CConfig::~CConfig() {
    // 无动态资源（如 new 分配的内存），无需手动释放
}

// ===== 基本配置接口实现 =====
CConfig& CConfig::setSourcePath(const std::string& path) {
    if (path.empty()) {
        throw std::invalid_argument("Source path cannot be empty");
    }
    m_sourcePath = path;  // 赋值给统一命名的成员变量
    return *this;  // 链式调用支持
}

const std::string& CConfig::getSourcePath() const {
    return m_sourcePath;  // 返回统一命名的成员变量
}

void CConfig::addSourcePath(const std::string& path) {
    if (path.empty()) {
        std::cerr << "Warning: Source path cannot be empty, skip adding" << std::endl;
        return;
    }
    m_sourcePaths.push_back(path);  // 追加到多源路径列表
}

const std::vector<std::string>& CConfig::getSourcePaths() const {
    return m_sourcePaths;  // 返回多源路径列表
}

CConfig& CConfig::setDestinationPath(const std::string& path) {
    if (path.empty()) {
        throw std::invalid_argument("Destination path cannot be empty");
    }
    m_destinationPath = path;  // 赋值给统一命名的成员变量
    return *this;  // 链式调用支持
}

const std::string& CConfig::getDestinationPath() const {
    return m_destinationPath;  // 返回统一命名的成员变量
}

// ===== 文件筛选配置接口实现 =====
CConfig& CConfig::setRecursiveSearch(bool value) {
    m_recursiveSearch = value;  // 赋值给统一命名的成员变量
    return *this;
}

bool CConfig::isRecursiveSearch() const {
    return m_recursiveSearch;  // 返回统一命名的成员变量
}

CConfig& CConfig::setFollowSymlinks(bool value) {
    m_followSymlinks = value;  // 赋值给统一命名的成员变量
    return *this;
}

bool CConfig::isFollowSymlinks() const {
    return m_followSymlinks;  // 返回统一命名的成员变量
}

CConfig& CConfig::addIncludePattern(const std::string& pattern) {
    try {
        m_includePatterns.emplace_back(pattern);  // 追加到统一命名的列表
    } catch (const std::regex_error& e) {
        std::cerr << "Warning: Invalid include regex pattern: " << e.what() << std::endl;
    }
    return *this;
}

const std::vector<std::regex>& CConfig::getIncludePatterns() const {
    return m_includePatterns;  // 返回统一命名的列表
}

CConfig& CConfig::addExcludePattern(const std::string& pattern) {
    try {
        m_excludePatterns.emplace_back(pattern);  // 追加到统一命名的列表
    } catch (const std::regex_error& e) {
        std::cerr << "Warning: Invalid exclude regex pattern: " << e.what() << std::endl;
    }
    return *this;
}

const std::vector<std::regex>& CConfig::getExcludePatterns() const {
    return m_excludePatterns;  // 返回统一命名的列表
}

// ===== 备份行为配置接口实现 =====
CConfig& CConfig::setPackingEnabled(bool value) {
    m_enablePacking = value;  // 赋值给统一命名的成员变量
    return *this;
}

bool CConfig::isPackingEnabled() const {
    return m_enablePacking;  // 返回统一命名的成员变量
}

CConfig& CConfig::setPackType(const std::string& type) {
    m_packType = type;  // 赋值给统一命名的成员变量
    return *this;
}

const std::string& CConfig::getPackType() const {
    return m_packType;  // 返回统一命名的成员变量
}

CConfig& CConfig::setCompressionEnabled(bool value) {
    m_enableCompression = value;  // 赋值给统一命名的成员变量
    return *this;
}

bool CConfig::isCompressionEnabled() const {
    return m_enableCompression;  // 返回统一命名的成员变量
}

CConfig& CConfig::setCompressionType(const std::string& type) {
    m_compressionType = type;  // 赋值给统一命名的成员变量
    return *this;
}

const std::string& CConfig::getCompressionType() const {
    return m_compressionType;  // 返回统一命名的成员变量
}

CConfig& CConfig::setCompressionLevel(int level) {
    if (level < 1 || level > 9) {
        throw std::invalid_argument("Compression level must be between 1 and 9");
    }
    m_compressionLevel = level;  // 赋值给统一命名的成员变量
    return *this;
}

int CConfig::getCompressionLevel() const {
    return m_compressionLevel;  // 返回统一命名的成员变量
}

CConfig& CConfig::setEncryptionEnabled(bool value) {
    m_enableEncryption = value;  // 赋值给统一命名的成员变量
    return *this;
}

bool CConfig::isEncryptionEnabled() const {
    return m_enableEncryption;  // 返回统一命名的成员变量
}

CConfig& CConfig::setEncryptionKey(const std::string& key) {
    m_encryptionKey = key;  // 赋值给统一命名的成员变量
    return *this;
}

const std::string& CConfig::getEncryptionKey() const {
    return m_encryptionKey;  // 返回统一命名的成员变量
}

CConfig& CConfig::setEncryptType(const std::string& type) {
    m_encryptType = type;  // 赋值给统一命名的成员变量
    return *this;
}

const std::string& CConfig::getEncryptType() const {
    return m_encryptType;  // 返回统一命名的成员变量
}

CConfig& CConfig::setDescription(const std::string& desc) {
    m_description = desc;  // 赋值给统一命名的成员变量
    return *this;
}

const std::string& CConfig::getDescription() const {
    return m_description;  // 返回统一命名的成员变量
}

// ===== 高级配置接口实现 =====
CConfig& CConfig::setCustomOption(const std::string& key, const std::string& value) {
    if (key.empty()) {
        std::cerr << "Warning: Custom option key cannot be empty, skip setting" << std::endl;
        return *this;
    }
    m_customOptions[key] = value;  // 赋值给统一命名的 map
    return *this;
}

std::string CConfig::getCustomOption(const std::string& key, const std::string& defaultValue) const {
    auto it = m_customOptions.find(key);  // 搜索统一命名的 map
    return it != m_customOptions.end() ? it->second : defaultValue;
}

// ===== 便捷工具方法实现 =====
bool CConfig::shouldIncludeFile(const std::string& filePath) const {
    // 第一步：检查排除模式（优先级高于包含模式）
    if (!m_excludePatterns.empty()) {
        if (std::any_of(m_excludePatterns.begin(), m_excludePatterns.end(),
            [&filePath](const auto& pattern) { return std::regex_match(filePath, pattern); })) {
            return false;  // 匹配排除模式，不备份
        }
    }
    // 第二步：检查包含模式（仅当包含模式非空时生效）
    if (!m_includePatterns.empty()) {
        return std::any_of(m_includePatterns.begin(), m_includePatterns.end(),
            [&filePath](const auto& pattern) { return std::regex_match(filePath, pattern); });
    }

    // 第三步：默认规则（无排除/包含模式时，默认备份）
    return true;
}

bool CConfig::isValid() const {
    // 1. 检查必要路径（单个源路径或多源路径至少有一个非空）
    bool hasValidSource = !m_sourcePath.empty() || !m_sourcePaths.empty();
    if (!hasValidSource) {
        std::cerr << "Error: No valid source path (single or multiple paths)" << std::endl;
        return false;
    }
    if (m_destinationPath.empty()) {
        std::cerr << "Error: Destination path cannot be empty" << std::endl;
        return false;
    }

    // 2. 检查单个源路径是否存在（若有）
    if (!m_sourcePath.empty() && !std::filesystem::exists(m_sourcePath)) {
        std::cerr << "Error: Single source path does not exist: " << m_sourcePath << std::endl;
        return false;
    }

    // 3. 检查多源路径是否存在（若有）
    if (!std::all_of(m_sourcePaths.begin(), m_sourcePaths.end(),
        [](const auto& path) { return std::filesystem::exists(path); })) {
        // 错误信息已在循环中输出，直接返回false
        return false;
    }

    // 4. 检查目标路径（不存在则尝试创建）
    if (!std::filesystem::exists(m_destinationPath)) {
        try {
            if (!std::filesystem::create_directories(m_destinationPath)) {
                std::cerr << "Error: Failed to create destination directory: " << m_destinationPath << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: Create destination directory failed: " << e.what() << std::endl;
            return false;
        }
    }

    // 5. 检查压缩级别有效性（仅当启用压缩时）
    if (m_enableCompression && (m_compressionLevel < 1 || m_compressionLevel > 9)) {
        std::cerr << "Error: Compression level must be between 1 and 9 (current: "
                    << m_compressionLevel << ")" << std::endl;
        return false;
    }

    // 6. 检查加密配置（仅当启用加密时，密钥非空）
    if (m_enableEncryption && m_encryptionKey.empty()) {
        std::cerr << "Error: Encryption enabled but encryption key is empty" << std::endl;
        return false;
    }

    // 7. 增加：目标路径是否可写
    if (!isPathWritable(m_destinationPath)) {
        std::cerr << "Error: Destination path is not writable: " << m_destinationPath << std::endl;
        return false;
    }

    // 所有检查通过，配置有效
    return true;
}

void CConfig::reset() {
    // 重置基本配置
    m_sourcePath.clear();
    m_sourcePaths.clear();
    m_destinationPath.clear();

    // 重置文件筛选配置
    m_recursiveSearch = false;
    m_followSymlinks = false;
    m_includePatterns.clear();
    m_excludePatterns.clear();

    // 重置备份行为配置
    m_enablePacking = false;
    m_packType = "tar";
    m_enableCompression = false;
    m_compressionType = "gzip";
    m_compressionLevel = 1;
    m_enableEncryption = false;
    m_encryptionKey.clear();

    // 重置高级配置
    m_customOptions.clear();
}

std::shared_ptr<CConfig> CConfig::clone() const {
    // 利用拷贝构造函数深拷贝（所有成员变量均为值类型或支持深拷贝）
    return std::make_shared<CConfig>(*this);
}

std::string CConfig::toString() const {
    std::ostringstream oss;
    oss << "==================== Backup Configuration ====================" << std::endl;

    // 基本配置
    oss << "1. Basic Config:" << std::endl;
    oss << "   - Single Source Path: " << (m_sourcePath.empty() ? "None" : m_sourcePath) << std::endl;
    oss << "   - Multiple Source Paths: " << m_sourcePaths.size() << " items" << std::endl;
    for (size_t i = 0; i < m_sourcePaths.size(); ++i) {
        oss << "     [" << i + 1 << "] " << m_sourcePaths[i] << std::endl;
    }
    oss << "   - Destination Path: " << m_destinationPath << std::endl;

    // 文件筛选配置
    oss << "2. File Filter Config:" << std::endl;
    oss << "   - Recursive Search: " << (m_recursiveSearch ? "Enabled" : "Disabled") << std::endl;
    oss << "   - Follow Symlinks: " << (m_followSymlinks ? "Enabled" : "Disabled") << std::endl;
    oss << "   - Include Patterns: " << m_includePatterns.size() << " regex(s)" << std::endl;
    oss << "   - Exclude Patterns: " << m_excludePatterns.size() << " regex(s)" << std::endl;

    // 备份行为配置
    oss << "3. Backup Behavior Config:" << std::endl;
    oss << "   - Packing: " << (m_enablePacking ? "Enabled (" + m_packType + ")" : "Disabled") << std::endl;
    oss << "   - Compression: " << (m_enableCompression ?
        "Enabled (" + m_compressionType + ", Level " + std::to_string(m_compressionLevel) + ")" :
        "Disabled") << std::endl;
    oss << "   - Encryption: " << (m_enableEncryption ? "Enabled" : "Disabled") << std::endl;

    // 高级配置
    oss << "4. Advanced Config:" << std::endl;
    oss << "   - Custom Options: " << m_customOptions.size() << " key-value pair(s)" << std::endl;
    for (const auto& [key, value] : m_customOptions) {
        oss << "     " << key << " = " << value << std::endl;
    }

    oss << "==============================================================" << std::endl;
    return oss.str();
}
