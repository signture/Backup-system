# 备份系统项目交接文档

## 目录
1. [项目概述](#项目概述)
2. [技术架构](#技术架构)
3. [项目结构](#项目结构)
4. [核心组件详解](#核心组件详解)
5. [组件间调用关系](#组件间调用关系)
6. [数据流与处理流程](#数据流与处理流程)
7. [构建与部署](#构建与部署)
8. [使用指南](#使用指南)
9. [扩展开发指南](#扩展开发指南)
10. [已知问题与注意事项](#已知问题与注意事项)

---

## 项目概述

### 项目简介
本项目是一个功能完整的文件备份与恢复系统，支持文件/目录的备份、恢复、打包、压缩、加密等功能。系统采用C++17开发，提供命令行界面（CLI）和图形用户界面（GUI）两种交互方式。

### 主要特性
- ✅ **文件备份与恢复**：支持单个文件或整个目录结构的备份和恢复
- ✅ **打包功能**：支持将多个文件打包成单个文件，保留目录结构
- ✅ **压缩功能**：支持Huffman压缩算法，减少存储空间
- ✅ **加密功能**：支持XOR对称加密，保护备份数据安全
- ✅ **备份记录管理**：自动记录备份历史，支持查询、删除等操作
- ✅ **文件筛选**：支持正则表达式筛选需要备份的文件
- ✅ **图形界面**：基于ImGui和GLFW的现代化GUI界面
- ✅ **跨平台支持**：支持Windows、Linux、macOS

### 版本信息
- **当前版本**：v1.2
- **开发语言**：C++17
- **构建系统**：CMake 3.14+
- **开发日期**：2025年

---

## 技术架构

### 技术栈
| 技术/库 | 版本/说明 | 用途 |
|---------|----------|------|
| C++ | C++17标准 | 核心开发语言 |
| CMake | 3.14+ | 构建系统 |
| GLFW | 3.x | 窗口管理和OpenGL上下文 |
| ImGui | 最新版 | 即时模式GUI框架 |
| nlohmann/json | 3.11.3+ | JSON序列化/反序列化 |
| Google Test | 最新版 | 单元测试框架 |
| OpenGL | - | 图形渲染（ImGui后端） |

### 设计模式
1. **工厂模式（Factory Pattern）**
   - `PackFactory`：创建不同类型的打包器
   - `CompressFactory`：创建不同类型的压缩器
   - `EncryptFactory`：创建不同类型的加密器

2. **策略模式（Strategy Pattern）**
   - `IPack`、`ICompress`、`IEncrypt`接口定义算法策略
   - 具体实现类可插拔替换

3. **配置模式（Configuration Pattern）**
   - `CConfig`类统一管理所有配置项
   - 支持链式调用，便于配置构建

4. **单例模式（Singleton Pattern）**
   - 备份记录器采用单例模式管理全局备份记录

### 架构层次
```
┌─────────────────────────────────────┐
│         用户交互层 (UI Layer)        │
│  ┌─────────────┐  ┌──────────────┐  │
│  │   CLI界面   │  │   GUI界面     │  │
│  │  (main.cpp) │  │  (gui.cpp)    │  │
│  └─────────────┘  └──────────────┘  │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│        业务逻辑层 (Business Layer)    │
│  ┌─────────────┐  ┌──────────────┐  │
│  │  CBackup    │  │ CBackupRecorder│ │
│  │  (核心引擎) │  │  (记录管理)   │  │
│  └─────────────┘  └──────────────┘  │
│  ┌─────────────────────────────────┐ │
│  │      CConfig (配置管理)         │ │
│  └─────────────────────────────────┘ │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│        算法层 (Algorithm Layer)      │
│  ┌──────────┐  ┌──────────┐  ┌─────┐│
│  │ PackFactory│ │CompressFactory│ │EncryptFactory││
│  └──────────┘  └──────────┘  └─────┘│
│  ┌──────────┐  ┌──────────┐  ┌─────┐│
│  │  myPack  │  │HuffmanCompress│ │SimpleXOREncrypt││
│  └──────────┘  └──────────┘  └─────┘│
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│        工具层 (Utility Layer)        │
│  ┌─────────────┐  ┌──────────────┐  │
│  │   Utils.h   │  │   CRC32.h    │  │
│  │ (路径处理等) │  │ (校验和计算) │  │
│  └─────────────┘  └──────────────┘  │
└─────────────────────────────────────┘
```

---

## 项目结构

### 目录结构
```
Backup-system-final/
├── bin/                    # 编译输出目录（可执行文件）
│   └── Debug/
│       ├── backup_app.exe  # 主程序可执行文件
│       └── test_app.exe    # 测试程序可执行文件
├── build/                  # CMake构建目录
├── include/                # 头文件目录
│   ├── CBackup.h          # 核心备份类
│   ├── CConfig.h          # 配置管理类
│   ├── CBackupRecorder.h  # 备份记录管理类
│   ├── PackFactory.h      # 打包器工厂
│   ├── CompressFactory.h  # 压缩器工厂
│   ├── EncryptFactory.h   # 加密器工厂
│   ├── IPack.h            # 打包接口
│   ├── ICompress.h        # 压缩接口
│   ├── IEncrypt.h         # 加密接口
│   ├── myPack.h           # 基础打包实现
│   ├── HuffmanCompress.h  # Huffman压缩实现
│   ├── SimpleXOREncrypt.h # XOR加密实现
│   ├── gui.h              # GUI界面接口
│   ├── Utils.h            # 工具函数
│   └── CRC32.h            # CRC32校验
├── src/                    # 源文件目录
│   ├── main.cpp           # 程序入口（CLI）
│   ├── gui.cpp            # GUI实现
│   ├── CBackup.cpp        # 备份核心实现
│   ├── CConfig.cpp        # 配置管理实现
│   ├── CBackupRecorder.cpp # 记录管理实现
│   ├── PackFactory.cpp    # 打包工厂实现
│   ├── CompressFactory.cpp # 压缩工厂实现
│   ├── EncryptFactory.cpp # 加密工厂实现
│   ├── myPack.cpp         # 打包算法实现
│   ├── HuffmanCompress.cpp # 压缩算法实现
│   ├── SimpleXOREncrypt.cpp # 加密算法实现
│   └── Utils.cpp          # 工具函数实现
├── test/                   # 测试目录
│   ├── test_main.cpp      # 测试入口
│   ├── test_backup.cpp    # 备份功能测试
│   ├── test_compress.cpp  # 压缩功能测试
│   ├── test_encrypt.cpp   # 加密功能测试
│   ├── test_pack.cpp      # 打包功能测试
│   ├── test_recorder.cpp  # 记录管理测试
│   └── test_integration.cpp # 集成测试
├── lib/                    # 第三方库目录
│   ├── glfw/              # GLFW窗口库
│   ├── imgui/             # ImGui GUI库
│   ├── json/              # nlohmann/json库
│   └── googletest/        # Google Test框架
├── CMakeLists.txt         # 根CMake配置
├── build_project.bat      # Windows构建脚本
└── README.md              # 项目说明文档
```

### 文件说明

#### 核心文件
- **main.cpp**：程序入口，提供交互式CLI界面，解析命令行参数，调用备份/恢复功能
- **gui.cpp**：图形界面实现，基于ImGui提供三个标签页（备份、恢复、记录管理）

#### 业务逻辑文件
- **CBackup.cpp**：核心备份引擎，实现`doBackup()`和`doRecovery()`方法
- **CConfig.cpp**：配置管理，提供链式调用的配置接口
- **CBackupRecorder.cpp**：备份记录管理，使用JSON持久化存储

#### 算法实现文件
- **myPack.cpp**：基础打包算法，支持目录结构打包
- **HuffmanCompress.cpp**：Huffman压缩算法实现
- **SimpleXOREncrypt.cpp**：XOR对称加密实现

#### 工厂类文件
- **PackFactory.cpp**：打包器工厂，根据类型字符串创建对应打包器
- **CompressFactory.cpp**：压缩器工厂，根据类型字符串创建对应压缩器
- **EncryptFactory.cpp**：加密器工厂，根据类型字符串创建对应加密器

---

## 核心组件详解

### 1. CBackup（核心备份类）

**职责**：执行备份和恢复操作的核心引擎

**主要方法**：
```cpp
std::string doBackup(const std::shared_ptr<CConfig>& config);
bool doRecovery(const BackupEntry& entry, const std::string& destDir);
bool doRecovery(const BackupEntry& entry, const std::string& destDir, const std::string& password);
```

**工作流程**：
1. **备份流程**：
   - 收集需要备份的文件列表（`collectFilesToBackup`）
   - 根据配置决定是否打包
   - 如果打包，调用`PackFactory`创建打包器并执行打包
   - 如果压缩，调用`CompressFactory`创建压缩器并执行压缩
   - 如果加密，调用`EncryptFactory`创建加密器并执行加密
   - 返回最终备份文件路径

2. **恢复流程**：
   - 读取备份记录（`BackupEntry`）
   - 按相反顺序执行：解密 → 解压缩 → 解包
   - 恢复文件到目标目录，保持原始目录结构

**关键特性**：
- 支持递归目录遍历
- 支持文件类型筛选（正则表达式）
- 自动创建目标目录
- 保留文件目录结构

### 2. CConfig（配置管理类）

**职责**：统一管理所有备份配置项

**主要配置项**：
- **路径配置**：源路径、目标路径
- **文件筛选**：递归搜索、包含/排除模式（正则表达式）
- **打包配置**：是否启用、打包类型
- **压缩配置**：是否启用、压缩类型、压缩级别
- **加密配置**：是否启用、加密类型、加密密钥
- **描述信息**：备份行为描述

**设计特点**：
- 支持链式调用：`config->setSourcePath(...).setDestinationPath(...)`
- 提供配置验证：`isValid()`方法
- 支持配置克隆：`clone()`方法
- 提供配置摘要：`toString()`方法

**使用示例**：
```cpp
auto config = std::make_shared<CConfig>();
config->setSourcePath("/path/to/source")
      .setDestinationPath("/path/to/dest")
      .setRecursiveSearch(true)
      .setPackingEnabled(true)
      .setPackType("Basic")
      .setCompressionEnabled(true)
      .setCompressionType("Huffman")
      .setEncryptionEnabled(true)
      .setEncryptType("SimXOR")
      .setEncryptionKey("mykey123")
      .setDescription("手动备份");
```

### 3. CBackupRecorder（备份记录管理类）

**职责**：管理备份历史记录，提供查询、删除、修改功能

**数据结构**：
```cpp
struct BackupEntry {
    std::string fileName;        // 源文件名
    std::string sourceFullPath;  // 源文件完整路径
    std::string destDirectory;   // 备份目标目录
    std::string backupFileName;  // 最终备份文件名
    std::string backupTime;      // 备份时间（精确到秒）
    bool isEncrypted;            // 是否加密
    bool isPacked;               // 是否打包
    bool isCompressed;           // 是否压缩
    std::string description;     // 备份描述
};
```

**主要方法**：
- `addBackupRecord()`：添加备份记录
- `getBackupRecords()`：获取所有备份记录
- `findBackupRecordsByFileName()`：根据文件名查找记录
- `findBackupRecordsByBackupTime()`：根据时间范围查找记录
- `deleteBackupRecord()`：删除备份记录
- `modifyBackupRecord()`：修改备份记录

**持久化**：
- 使用JSON格式存储备份记录
- 默认存储路径：`./backup_repository/backup_records.json`
- 支持自动保存和手动保存

### 4. 工厂类（Factory Classes）

#### PackFactory（打包器工厂）
**支持的打包类型**：
- `"Basic"`：基础打包（myPack实现）

**主要方法**：
```cpp
static std::unique_ptr<IPack> createPacker(const std::string& packType);
static bool isPackTypeSupported(const std::string& packType);
static std::string getPackType(const std::string& filePath);
```

#### CompressFactory（压缩器工厂）
**支持的压缩类型**：
- `"Huffman"`：Huffman压缩算法

**主要方法**：
```cpp
static std::unique_ptr<ICompress> createCompress(const std::string& compressType);
static bool isCompressTypeSupported(const std::string& compressType);
static std::string getCompressType(const std::string& filePath);
```

#### EncryptFactory（加密器工厂）
**支持的加密类型**：
- `"SimXOR"`：简单XOR对称加密

**主要方法**：
```cpp
static std::unique_ptr<IEncrypt> createEncryptor(const std::string& encryptType);
static bool isEncryptTypeSupported(const std::string& encryptType);
static std::string getEncryptType(const std::string& filePath);
```

### 5. 算法接口与实现

#### IPack（打包接口）
**接口定义**：
```cpp
virtual std::string pack(const std::vector<std::string>& files, const std::string& destPath) = 0;
virtual bool unpack(const std::string& srcPath, const std::string& destDir) = 0;
virtual PackType getPackType() const = 0;
virtual std::string getPackTypeName() const = 0;
```

**实现类**：`myPack`
- **打包格式**：
  - 打包标志位（1字节）：0x01
  - 打包算法（1字节）
  - 文件数量（4字节）
  - 元数据区长度（4字节）
  - 文件元信息（变长）：文件名长度、文件名、文件大小、偏移量、文件类型
  - 文件内容（按顺序排列）

#### ICompress（压缩接口）
**接口定义**：
```cpp
virtual std::string compressFile(const std::string& sourcePath) = 0;
virtual bool decompressFile(const std::string& sourcePath, const std::string& destPath) = 0;
virtual CompressType getCompressType() const = 0;
virtual std::string getCompressTypeName() const = 0;
```

**实现类**：`HuffmanCompress`
- **压缩格式**：
  - 压缩标志位（1字节）：0x21（压缩）/ 0x20（未压缩）
  - 压缩算法类型（1字节）
  - 有效位数（1字节）
  - 保留位（1字节）
  - 头大小（4字节）
  - 词频表大小（4字节）
  - 原始文件大小（8字节）
  - CRC32校验值（4字节）
  - 词频表（变长）
  - 压缩数据（变长）

#### IEncrypt（加密接口）
**接口定义**：
```cpp
virtual std::string encryptFile(const std::string& sourcePath, const std::string& key) = 0;
virtual bool decryptFile(const std::string& sourcePath, const std::string& destPath, const std::string& key) = 0;
virtual EncryptType getEncryptType() const = 0;
virtual std::string getEncryptTypeName() const = 0;
```

**实现类**：`SimpleXOREncrypt`
- **加密格式**：
  - 加密标志位（1字节）：0x31（加密）/ 0x30（未加密）
  - 加密算法类型（1字节）
  - 头大小（4字节）
  - CRC32校验值（4字节）
  - 加密数据（变长）

### 6. GUI模块（gui.cpp）

**技术栈**：ImGui + GLFW + OpenGL

**主要功能**：
1. **备份标签页**：
   - 源路径选择（支持文件/文件夹选择对话框）
   - 目标路径输入
   - 打包/压缩/加密选项配置
   - 备份描述输入
   - 执行备份按钮

2. **恢复标签页**：
   - 备份文件选择（从记录中选择）
   - 恢复目标路径输入
   - 密码输入（加密备份需要）
   - 执行恢复按钮

3. **记录管理标签页**：
   - 显示所有备份记录列表
   - 按文件名/时间筛选
   - 删除记录功能
   - 记录详情查看

**启动方式**：
- 命令行：`backup_app.exe` 然后输入 `gui`
- 或直接：`backup_app.exe --mode gui`

---

## 组件间调用关系

### 调用关系图

```
main.cpp (CLI入口)
    │
    ├─→ runParsed() [解析命令行参数]
    │       │
    │       ├─→ CConfig::setSourcePath() [配置源路径]
    │       ├─→ CConfig::setDestinationPath() [配置目标路径]
    │       ├─→ PackFactory::isPackTypeSupported() [检查打包类型]
    │       ├─→ CompressFactory::isCompressTypeSupported() [检查压缩类型]
    │       ├─→ EncryptFactory::isEncryptTypeSupported() [检查加密类型]
    │       │
    │       └─→ CBackup::doBackup() [执行备份]
    │               │
    │               ├─→ collectFilesToBackup() [收集文件列表]
    │               │       └─→ CConfig::shouldIncludeFile() [文件筛选]
    │               │
    │               ├─→ PackFactory::createPacker() [创建打包器]
    │               │       └─→ IPack::pack() [执行打包]
    │               │
    │               ├─→ CompressFactory::createCompress() [创建压缩器]
    │               │       └─→ ICompress::compressFile() [执行压缩]
    │               │
    │               ├─→ EncryptFactory::createEncryptor() [创建加密器]
    │               │       └─→ IEncrypt::encryptFile() [执行加密]
    │               │
    │               └─→ CBackupRecorder::addBackupRecord() [记录备份]
    │
    └─→ CBackup::doRecovery() [执行恢复]
            │
            ├─→ EncryptFactory::isFileEncrypted() [检查是否加密]
            ├─→ EncryptFactory::createEncryptor() [创建解密器]
            │       └─→ IEncrypt::decryptFile() [执行解密]
            │
            ├─→ CompressFactory::isCompressedFile() [检查是否压缩]
            ├─→ CompressFactory::createCompress() [创建解压器]
            │       └─→ ICompress::decompressFile() [执行解压]
            │
            ├─→ PackFactory::isPackedFile() [检查是否打包]
            ├─→ PackFactory::createPacker() [创建解包器]
            │       └─→ IPack::unpack() [执行解包]
            │
            └─→ Utils::copyFile() [复制文件到目标目录]

gui.cpp (GUI入口)
    │
    └─→ runBackupGUI() [启动GUI]
            │
            ├─→ CBackupRecorder::getBackupRecords() [获取备份记录]
            ├─→ CBackupRecorder::findBackupRecordsByFileName() [查找记录]
            ├─→ CBackup::doBackup() [执行备份]
            └─→ CBackup::doRecovery() [执行恢复]
```

### 数据流图

#### 备份数据流
```
源文件/目录
    ↓
[文件收集] collectFilesToBackup()
    ↓
文件列表 (vector<string>)
    ↓
[可选] 打包 → IPack::pack()
    ↓
打包文件
    ↓
[可选] 压缩 → ICompress::compressFile()
    ↓
压缩文件
    ↓
[可选] 加密 → IEncrypt::encryptFile()
    ↓
最终备份文件
    ↓
[记录] CBackupRecorder::addBackupRecord()
    ↓
备份记录 (JSON)
```

#### 恢复数据流
```
备份记录 (BackupEntry)
    ↓
备份文件路径
    ↓
[如果加密] 解密 → IEncrypt::decryptFile()
    ↓
[如果压缩] 解压 → ICompress::decompressFile()
    ↓
[如果打包] 解包 → IPack::unpack()
    ↓
原始文件/目录
    ↓
[文件复制] Utils::copyFile()
    ↓
目标目录
```

### 依赖关系

```
main.cpp
    ├─→ CBackup.h
    ├─→ CConfig.h
    ├─→ CBackupRecorder.h
    ├─→ PackFactory.h
    ├─→ CompressFactory.h
    ├─→ EncryptFactory.h
    └─→ gui.h

CBackup.cpp
    ├─→ CConfig.h
    ├─→ CBackupRecorder.h
    ├─→ PackFactory.h
    ├─→ CompressFactory.h
    ├─→ EncryptFactory.h
    └─→ Utils.h

CBackupRecorder.cpp
    ├─→ CConfig.h
    └─→ nlohmann/json.hpp

PackFactory.cpp
    ├─→ IPack.h
    └─→ myPack.h

CompressFactory.cpp
    ├─→ ICompress.h
    └─→ HuffmanCompress.h

EncryptFactory.cpp
    ├─→ IEncrypt.h
    └─→ SimpleXOREncrypt.h

myPack.cpp
    └─→ IPack.h

HuffmanCompress.cpp
    ├─→ ICompress.h
    └─→ CRC32.h

SimpleXOREncrypt.cpp
    ├─→ IEncrypt.h
    └─→ CRC32.h

gui.cpp
    ├─→ CBackup.h
    ├─→ CConfig.h
    ├─→ CBackupRecorder.h
    ├─→ imgui.h
    └─→ GLFW/glfw3.h
```

---

## 数据流与处理流程

### 备份流程详解

1. **初始化阶段**
   ```
   用户输入 → main.cpp::runParsed()
   ↓
   解析命令行参数
   ↓
   创建 CConfig 对象
   ↓
   设置配置项（路径、打包、压缩、加密等）
   ```

2. **文件收集阶段**
   ```
   CBackup::doBackup()
   ↓
   collectFilesToBackup()
   ↓
   递归遍历源目录（如果启用递归）
   ↓
   对每个文件调用 CConfig::shouldIncludeFile()
   ↓
   根据包含/排除模式筛选文件
   ↓
   返回文件列表
   ```

3. **打包阶段**（如果启用）
   ```
   PackFactory::createPacker("Basic")
   ↓
   创建 myPack 实例
   ↓
   myPack::pack(files, destPath)
   ↓
   写入文件头（标志位、算法类型、文件数量等）
   ↓
   写入文件元数据（文件名、大小、偏移、类型）
   ↓
   写入文件内容
   ↓
   返回打包文件路径
   ```

4. **压缩阶段**（如果启用）
   ```
   CompressFactory::createCompress("Huffman")
   ↓
   创建 HuffmanCompress 实例
   ↓
   HuffmanCompress::compressFile(sourcePath)
   ↓
   统计字节频率
   ↓
   构建Huffman树
   ↓
   生成Huffman编码表
   ↓
   写入压缩头（标志位、算法类型、词频表等）
   ↓
   写入压缩数据
   ↓
   计算并写入CRC32校验值
   ↓
   返回压缩文件路径
   ```

5. **加密阶段**（如果启用）
   ```
   EncryptFactory::createEncryptor("SimXOR")
   ↓
   创建 SimpleXOREncrypt 实例
   ↓
   SimpleXOREncrypt::encryptFile(sourcePath, key)
   ↓
   写入加密头（标志位、算法类型等）
   ↓
   使用密钥对文件内容进行XOR加密
   ↓
   计算并写入CRC32校验值
   ↓
   返回加密文件路径
   ```

6. **记录阶段**
   ```
   CBackupRecorder::addBackupRecord(config, destPath)
   ↓
   创建 BackupEntry 对象
   ↓
   填充备份信息（文件名、路径、时间、描述等）
   ↓
   添加到内存记录列表
   ↓
   保存到JSON文件（如果启用自动保存）
   ```

### 恢复流程详解

1. **初始化阶段**
   ```
   用户输入恢复命令
   ↓
   CBackupRecorder::findBackupRecordsByFileName()
   ↓
   如果多个记录，用户选择
   ↓
   获取 BackupEntry 对象
   ```

2. **解密阶段**（如果加密）
   ```
   EncryptFactory::isFileEncrypted()
   ↓
   读取文件头，检查加密标志位（0x31）
   ↓
   用户输入密码（或从GUI传入）
   ↓
   EncryptFactory::createEncryptor("SimXOR")
   ↓
   SimpleXOREncrypt::decryptFile(sourcePath, destPath, key)
   ↓
   验证CRC32校验值
   ↓
   使用密钥进行XOR解密
   ↓
   写入解密文件
   ```

3. **解压阶段**（如果压缩）
   ```
   CompressFactory::isCompressedFile()
   ↓
   读取文件头，检查压缩标志位（0x21）
   ↓
   CompressFactory::createCompress("Huffman")
   ↓
   HuffmanCompress::decompressFile(sourcePath, destPath)
   ↓
   读取词频表
   ↓
   重建Huffman树
   ↓
   解码压缩数据
   ↓
   验证CRC32校验值
   ↓
   写入解压文件
   ```

4. **解包阶段**（如果打包）
   ```
   PackFactory::isPackedFile()
   ↓
   读取文件头，检查打包标志位（0x01）
   ↓
   PackFactory::createPacker("Basic")
   ↓
   myPack::unpack(sourcePath, destDir)
   ↓
   读取文件头信息
   ↓
   读取文件元数据
   ↓
   根据元数据创建目录结构
   ↓
   读取文件内容并写入对应位置
   ```

5. **文件恢复阶段**
   ```
   如果未打包，直接复制文件
   ↓
   如果已打包，解包后文件已在目标目录
   ↓
   保持原始目录结构
   ```

---

## 构建与部署

### 环境要求
- **操作系统**：Windows 10+, Linux, macOS
- **编译器**：支持C++17的编译器（MSVC 2017+, GCC 7+, Clang 5+）
- **CMake**：3.14或更高版本
- **第三方库**：已包含在`lib/`目录中

### 构建步骤

#### Windows平台
```bash
# 1. 确保CMake已安装并添加到PATH

# 2. 运行构建脚本
build_project.bat

# 或者手动构建
mkdir build
cd build
cmake ..
cmake --build . --config Debug

# 3. 可执行文件位于
bin/Debug/backup_app.exe
bin/Debug/test_app.exe
```

#### Linux/macOS平台
```bash
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置CMake
cmake ..

# 3. 编译
make

# 4. 可执行文件位于
../bin/backup_app
../bin/test_app
```

### CMake配置说明

#### 根CMakeLists.txt
- 设置C++17标准
- 配置MSVC运行时库（避免链接错误）
- 添加第三方库（GLFW、ImGui、JSON、Google Test）
- 设置输出目录

#### src/CMakeLists.txt
- 创建`backup_core`静态库（包含所有业务逻辑）
- 创建`imgui`静态库（ImGui实现）
- 创建`backup_app`可执行文件（链接所有库）

#### test/CMakeLists.txt
- 引入Google Test
- 创建`test_app`可执行文件
- 注册CTest测试用例

### 依赖库说明

#### GLFW
- **位置**：`lib/glfw/`
- **用途**：窗口管理和OpenGL上下文
- **配置**：已禁用示例、测试、文档构建

#### ImGui
- **位置**：`lib/imgui/`
- **用途**：即时模式GUI框架
- **后端**：GLFW + OpenGL3

#### nlohmann/json
- **位置**：`lib/json/include/`
- **用途**：JSON序列化/反序列化
- **使用方式**：仅头文件库

#### Google Test
- **位置**：`lib/googletest/`
- **用途**：单元测试框架
- **部署**：通过CMake的`add_subdirectory`自动构建

---

## 使用指南

### 命令行界面（CLI）

#### 启动程序
```bash
backup_app.exe
```

#### 备份操作
```bash
# 基本备份
--mode backup --src "F:\source" --dst "F:\backup"

# 带文件筛选的备份
--mode backup --src "F:\source" --dst "F:\backup" --include ".*\\.txt"

# 带打包的备份
--mode backup --src "F:\source" --dst "F:\backup" --pack Basic

# 带压缩的备份
--mode backup --src "F:\source" --dst "F:\backup" --pack Basic --compress Huffman

# 带加密的备份
--mode backup --src "F:\source" --dst "F:\backup" --pack Basic --compress Huffman --encrypt SimXOR --key "mykey123"

# 带描述的备份
--mode backup --src "F:\source" --dst "F:\backup" --pack Basic --desc "手动备份"
```

#### 恢复操作
```bash
# 基本恢复
--mode recover --fn "backup_file.bak" --to "F:\restore"

# 如果有多个同名备份，程序会提示选择
```

#### 图形界面
```bash
# 方式1：在交互式CLI中输入
gui

# 方式2：直接启动GUI
--mode gui
```

### 图形界面（GUI）

#### 启动GUI
- 运行程序后输入`gui`命令
- 或使用`--mode gui`参数

#### 备份标签页
1. 点击"选择源路径"按钮，选择要备份的文件/文件夹
2. 在"目标路径"输入框中输入备份目标路径
3. 勾选需要的选项：
   - 打包（选择打包类型）
   - 压缩（选择压缩类型）
   - 加密（选择加密类型，输入密钥）
4. 输入备份描述（可选）
5. 点击"执行备份"按钮

#### 恢复标签页
1. 在"备份文件"下拉框中选择要恢复的备份文件
2. 在"恢复目标路径"输入框中输入恢复目标路径
3. 如果备份已加密，在"密码"输入框中输入密码
4. 点击"执行恢复"按钮

#### 记录管理标签页
1. 查看所有备份记录列表
2. 使用筛选功能查找特定记录
3. 点击"删除"按钮删除不需要的记录

### 备份记录文件格式

备份记录存储在JSON文件中，默认路径：`./backup_repository/backup_records.json`

**JSON格式示例**：
```json
[
  {
    "file_name": "test.txt",
    "source_full_path": "F:\\source\\test.txt",
    "destination_directory": "F:\\backup",
    "backup_file_name": "test_20250101_120000.bak",
    "backup_time": "2025-01-01 12:00:00",
    "is_encrypted": true,
    "is_packed": true,
    "is_compressed": true,
    "description": "手动备份"
  }
]
```

---

## 扩展开发指南

### 添加新的打包算法

1. **实现IPack接口**
   ```cpp
   // include/NewPack.h
   #include "IPack.h"
   
   class NewPack : public IPack {
   public:
       std::string pack(const std::vector<std::string>& files, 
                       const std::string& destPath) override;
       bool unpack(const std::string& srcPath, 
                  const std::string& destDir) override;
       PackType getPackType() const override;
       std::string getPackTypeName() const override;
   };
   ```

2. **在PackFactory中注册**
   ```cpp
   // src/PackFactory.cpp
   std::unique_ptr<IPack> PackFactory::createPacker(const std::string& packType) {
       if (packType == "NewPack") {
           return std::make_unique<NewPack>();
       }
       // ... 其他类型
   }
   ```

3. **更新支持的打包类型列表**
   ```cpp
   std::vector<std::string> PackFactory::getSupportedPackTypes() {
       return {"Basic", "NewPack"};  // 添加新类型
   }
   ```

### 添加新的压缩算法

1. **实现ICompress接口**
   ```cpp
   // include/NewCompress.h
   #include "ICompress.h"
   
   class NewCompress : public ICompress {
   public:
       std::string compressFile(const std::string& sourcePath) override;
       bool decompressFile(const std::string& sourcePath, 
                          const std::string& destPath) override;
       CompressType getCompressType() const override;
       std::string getCompressTypeName() const override;
   };
   ```

2. **在CompressFactory中注册**
   ```cpp
   // src/CompressFactory.cpp
   std::unique_ptr<ICompress> CompressFactory::createCompress(const std::string& compressType) {
       if (compressType == "NewCompress") {
           return std::make_unique<NewCompress>();
       }
       // ... 其他类型
   }
   ```

3. **更新CompressType枚举**
   ```cpp
   // include/ICompress.h
   enum class CompressType : uint8_t {
       None = 0,
       Huffman = 1,
       NewCompress = 2,  // 添加新类型
   };
   ```

### 添加新的加密算法

1. **实现IEncrypt接口**
   ```cpp
   // include/NewEncrypt.h
   #include "IEncrypt.h"
   
   class NewEncrypt : public IEncrypt {
   public:
       std::string encryptFile(const std::string& sourcePath, 
                              const std::string& key) override;
       bool decryptFile(const std::string& sourcePath, 
                       const std::string& destPath, 
                       const std::string& key) override;
       EncryptType getEncryptType() const override;
       std::string getEncryptTypeName() const override;
   };
   ```

2. **在EncryptFactory中注册**
   ```cpp
   // src/EncryptFactory.cpp
   std::unique_ptr<IEncrypt> EncryptFactory::createEncryptor(const std::string& encryptType) {
       if (encryptType == "NewEncrypt") {
           return std::make_unique<NewEncrypt>();
       }
       // ... 其他类型
   }
   ```

3. **更新EncryptType枚举**
   ```cpp
   // include/IEncrypt.h
   enum class EncryptType : uint8_t {
       None = 0,
       SimXOR = 1,
       NewEncrypt = 2,  // 添加新类型
   };
   ```

### 文件格式规范

#### 打包文件格式
- **标志位**：必须为0x01
- **算法类型**：1字节，对应PackType枚举值
- **文件数量**：4字节，小端序
- **元数据区长度**：4字节，小端序
- **文件元信息**：变长，包含文件名长度、文件名、文件大小、偏移量、文件类型
- **文件内容**：按顺序排列

#### 压缩文件格式
- **标志位**：0x21（压缩）/ 0x20（未压缩）
- **算法类型**：1字节，对应CompressType枚举值
- **有效位数**：1字节，最后一个字节的有效位数
- **保留位**：1字节，填充到字节边界
- **头大小**：4字节，小端序
- **词频表大小**：4字节，小端序
- **原始文件大小**：8字节，小端序
- **CRC32校验值**：4字节，小端序
- **词频表**：变长
- **压缩数据**：变长

#### 加密文件格式
- **标志位**：0x31（加密）/ 0x30（未加密）
- **算法类型**：1字节，对应EncryptType枚举值
- **头大小**：4字节，小端序
- **CRC32校验值**：4字节，小端序
- **加密数据**：变长

---

## 已知问题与注意事项

### 已知问题

1. **文件格式兼容性**
   - 不同算法头结构不同，结构重复命名是设计缺陷
   - 未来需要统一文件格式设计

2. **路径处理**
   - Windows路径分隔符为`\`，Linux/macOS为`/`
   - 使用`std::filesystem`自动处理，但需要注意跨平台兼容性

3. **字符编码**
   - Windows下需要UTF-8到GBK的转换（用于控制台显示）
   - Linux/macOS默认使用UTF-8

4. **内存管理**
   - 大文件处理时需要注意内存使用
   - 当前使用64KB缓冲区，对于超大文件可能需要优化

### 注意事项

1. **备份路径**
   - 备份路径完全由用户指定，不依赖任何仓库概念
   - 建议使用绝对路径，避免相对路径带来的问题

2. **加密密钥**
   - 加密密钥必须妥善保管，丢失密钥将无法恢复数据
   - 当前使用简单XOR加密，安全性有限，仅适用于一般场景

3. **备份记录**
   - 备份记录文件损坏可能导致无法恢复
   - 建议定期备份`backup_records.json`文件

4. **文件覆盖**
   - 恢复时如果目标路径已存在文件，会进行覆盖
   - 建议在恢复前检查目标路径

5. **符号链接**
   - 默认不跟随符号链接，避免循环引用
   - 可通过配置启用符号链接跟随

6. **权限问题**
   - 某些系统文件可能需要管理员权限才能访问
   - 备份/恢复时可能遇到权限错误

### 性能优化建议

1. **大文件处理**
   - 对于超大文件，考虑使用流式处理
   - 增加进度显示功能

2. **多线程支持**
   - 文件收集和复制可以并行处理
   - 压缩/加密操作可以并行执行

3. **增量备份**
   - 当前仅支持全量备份
   - 未来可以添加增量备份功能

4. **压缩优化**
   - 当前仅支持Huffman压缩
   - 可以添加更高效的压缩算法（如LZ77、LZMA等）

### 安全建议

1. **加密强度**
   - 当前XOR加密仅适用于一般场景
   - 敏感数据建议使用AES等强加密算法

2. **密钥管理**
   - 密钥不应硬编码在代码中
   - 建议使用密钥管理系统

3. **文件完整性**
   - 当前使用CRC32校验，可以升级为SHA-256等更安全的哈希算法

---

## 附录

### 关键常量定义

```cpp
// 打包标志位
#define PACK_FLAG 0x01

// 压缩标志位
#define COMPRESS_FLAG 0x21
#define UNCOMPRESS_FLAG 0x20

// 加密标志位
#define ENCRYPT_FLAG 0x31
#define UNENCRYPT_FLAG 0x30

// 缓冲区大小
#define BUFFER_SIZE (1 << 16)  // 64KB
```

### 错误码说明

- **0**：成功
- **1**：参数错误或配置无效
- **2**：备份/恢复操作失败

### 测试覆盖

当前测试覆盖以下功能：
- ✅ 基本备份功能
- ✅ 基本恢复功能
- ✅ 打包功能
- ✅ 解包功能
- ✅ 压缩功能
- ✅ 解压功能
- ✅ 加密功能
- ✅ 解密功能
- ✅ 备份记录管理

### 联系方式

如有问题或需要技术支持，请联系项目维护者。

---

**文档版本**：v1.0  
**最后更新**：2025年1月  
**维护者**：JiJun Lu, Linru Zhou

