// Copyright [2025] <JiJun Lu, Linru Zhou>
#include "CBackup.h"

/**
 * CBackup implementation
 */

// build 去搭建项目框架  Cmake  --> 项目一些开发环境

// 构造函数
CBackup::CBackup() {
}

// 析构函数
CBackup::~CBackup() {
}


// 收集需要备份的文件列表
std::vector<std::string> collectFilesToBackup(const std::string& rootPath, const std::shared_ptr<CConfig>& config) {
    std::vector<std::string> filesList;
    // 检查配置是否有效
    if (!config || !fs::exists(rootPath)) {
        return filesList;
    }

    // 先根遍历：先添加当前路径
    filesList.push_back(rootPath);

    // 如果是目录，递归遍历其子目录和文件
    if (fs::is_directory(rootPath)) {
        // 根据配置决定是否递归遍历
        if (config->isRecursiveSearch()) {
            // 递归遍历目录（先根遍历）
            for (auto it = fs::directory_iterator(rootPath,
                    fs::directory_options::skip_permission_denied);
                 it != fs::directory_iterator(); ++it) {
                const auto& p = *it;
                const std::string pathStr = p.path().string();

                // 递归调用，继续先根遍历
                std::vector<std::string> subFiles = collectFilesToBackup(pathStr, config);
                filesList.insert(filesList.end(), subFiles.begin(), subFiles.end());
            }
        } else {
            // 非递归遍历，只添加当前目录下的直接子项
            for (auto it = fs::directory_iterator(rootPath,
                    fs::directory_options::skip_permission_denied);
                 it != fs::directory_iterator(); ++it) {
                const auto& p = *it;
                const std::string pathStr = p.path().string();
                filesList.push_back(pathStr);
            }
        }
    }
    return filesList;
}

bool CBackup::doRecovery(const BackupEntry& entry, const std::string& destDir) {
    // 基础恢复：
    // - 若是打包：调用解包器（此处保留输出提示，具体实现按打包器完成）
    // - 若非打包：从备份目录将文件按原始相对路径复制回去

    // 首先检查目标路径是否存在
    if (!fs::exists(destDir)) {
        std::cerr << "Error: Destination path does not exist: " << destDir << std::endl;
        return false;
    }

    // 检查目标路径是否合法
    if (!isPathWritable(destDir)) {
        std::cerr << "Error: Destination path is not writable: " << destDir << std::endl;
        return false;
    }

    const std::string backupRoot = entry.destDirectory;  // 记录中的目标目录（备份落地位置）
    std::string backupName = entry.backupFileName;  // 记录中的备份文件名或相对路径

    const fs::path backupPath = fs::path(backupRoot) / backupName;
    // 删除的话保留最初的备份文件，形成的中间文件都被删掉
    bool isDecrypted = false;
    bool isDecompressed = false;

    // 先解密
    EncryptFactory encryptFactory;
    if (encryptFactory.isFileEncrypted(backupRoot + "/" + backupName)) {
        std::cout << "Decrypting file:" << backupName << std::endl;

        // 向用户请求密码
        std::string password;
        std::cout << "Enter password for decrypting file " << backupName << ": ";
        std::cin >> password;

        // 创建对应类型加密器
        std::string encryptType = encryptFactory.getEncryptType(backupRoot + "/" + backupName);
        if (encryptType.empty()) {
            std::cerr << "Error: Unknown encrypt type for file: " << backupName << std::endl;
            return false;
        }
        std::unique_ptr<IEncrypt> decryptor = nullptr;
        try {
            decryptor = encryptFactory.createEncryptor(encryptType);
        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to create decryptor: " << e.what() << std::endl;
            return false;
        }
        // 解密到备份目录
        // 这里是懒得改了
        std::string sourcePath = backupRoot + "/" + backupName;
        // 将后缀去除
        backupName.resize(backupName.find_last_of('.'));
        if (!decryptor->decryptFile(sourcePath, backupRoot + "/" + backupName, password)) {
            std::cerr << "Error: Failed to decrypt file: " << backupName << std::endl;
            return false;
        }
        isDecrypted = true;
    }

    // 再解压缩
    CompressFactory compressFactory;
    if (compressFactory.isCompressedFile(backupRoot + "/" + backupName)) {
        std::cout << "Decompressing file:" << backupName << std::endl;
        // 创建对应类型打包器
        std::string decompressType = compressFactory.getCompressType(backupRoot + "/" + backupName);
        if (decompressType.empty()) {
            std::cerr << "Error: Unknown compress type for file: " << backupName << std::endl;
            return false;
        }
        std::unique_ptr<ICompress> decompressor = nullptr;
        try {
            decompressor = compressFactory.createCompress(decompressType);
        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to create decompressor: " << e.what() << std::endl;
            return false;
        }
        // 解压缩到备份目录
        // 这里是懒得改了
        std::string sourcePath = backupRoot + "/" + backupName;
        // 将后缀去除
        backupName.resize(backupName.find_last_of('.'));
        if (!decompressor->decompressFile(sourcePath, backupRoot + "/" + backupName)) {
            std::cerr << "Error: Failed to decompress file: " << backupName << std::endl;
            return false;
        }
        // 解压完成了，如果之前有解密过，那就删除解密后的文件
        if (isDecrypted) {
            if (!fs::remove(sourcePath)) {
                std::cerr << "Error: Failed to remove compressed file: " << sourcePath << std::endl;
                return false;
            }
        }
        isDecompressed = true;
    }


    // 最后解包
    PackFactory packFactory;
    if (packFactory.isPackedFile(backupRoot + "/" + backupName)) {
        std::cout << "Unpacking file: " << backupName << std::endl;
        // 创建对应类型打包器
        std::string packType = packFactory.getPackType(backupRoot + "/" + backupName);
        if (packType.empty()) {
            std::cerr << "Error: Unknown pack type for file: " << backupName << std::endl;
            return false;
        }
        std::unique_ptr<IPack> packer = nullptr;
        try {
            packer = PackFactory::createPacker(packType);
        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to create packer: " << e.what() << std::endl;
            return false;
        }
        // 解包到源文件目录
        if (!packer->unpack(backupRoot + "/" + backupName, destDir)) {
            std::cerr << "Error: Failed to unpack file: " << backupName << std::endl;
            return false;
        }
        // 解包完成了，如果之前有解密过或者压缩过，那就删除解密后的文件或者压缩后的文件
        if (isDecompressed || isDecrypted) {
            if (!fs::remove(backupRoot + "/" + backupName)) {
                std::cerr << "Error: Failed to remove packed file: " << backupRoot + "/" + backupName << std::endl;
                return false;
            }
        }
        return true;
    }

    // 非打包：按路径直接复制
    // 入口记录里 sourceFullPath 是原文件应恢复到的绝对路径
    // 这里改为恢复到 destDir 下的相对路径
    fs::path sourcePathObj(entry.sourceFullPath);
    const fs::path restorePath = fs::path(destDir) / sourcePathObj.filename();
    try {
        fs::create_directories(restorePath.parent_path());
    } catch (const std::exception& e) {
        std::cerr << "Error creating directory for restore: " << e.what() << std::endl;
        return false;
    }

    // 直接复制文件
    try {
        if (!fs::exists(backupPath)) {
            std::cerr << "Error: backup file not found: " << backupPath.string() << std::endl;
            return false;
        }
        // 直接复制就不太一样，backupPath是新增的一个目录路径，真正的路径还要再下一个文件
        fs::path actualBackupPath = fs::path(backupPath) / entry.fileName;
        if (fs::is_directory(actualBackupPath)) {
            // 目录复制：使用recursive选项
            fs::copy(actualBackupPath, restorePath,
                     fs::copy_options::recursive |
                     fs::copy_options::overwrite_existing |
                     fs::copy_options::copy_symlinks);
        } else if (fs::is_regular_file(actualBackupPath)) {
            // 文件复制：使用copy_file
            fs::copy_file(actualBackupPath, restorePath, fs::copy_options::overwrite_existing);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error restoring file: " << e.what() << std::endl;
        return false;
    }

    return true;
}


std::string CBackup::doBackup(const std::shared_ptr<CConfig>& config) {
    // 1) 基础校验
    if (!config || !config->isValid()) {
        std::cerr << "Error: Invalid backup configuration" << std::endl;
        return "";
    }

    // 2) 准备源集合（优先单源路径，如为空再尝试多源路径）
    std::vector<std::string> sourceRoots;
    if (!config->getSourcePath().empty()) {
        sourceRoots.push_back(config->getSourcePath());
    } else {
    std::copy(config->getSourcePaths().begin(), config->getSourcePaths().end(), std::back_inserter(sourceRoots));
    }
    if (sourceRoots.empty()) {
        std::cerr << "Error: No source specified" << std::endl;
        return "";
    }

    // 3) 收集需要备份的条目（含目录与文件；目录用于创建结构，文件用于拷贝）
    std::vector<std::string> filesToBackup;
    std::string destPath;

    // 理论上只需要设计一个源就好，这个是之前的设计漏洞，后面改进一下
    // TODO(Linru Zhou): 将这个修改为只有一个源路径，不需要设计为vector
    filesToBackup = collectFilesToBackup(sourceRoots[0], config);

    if (filesToBackup.empty()) {
        std::cerr << "Error: No files to backup" << std::endl;
        return "";
    }

    // 4) 创建目标根目录
    const std::string destinationRoot = config->getDestinationPath();
    try {
        fs::create_directories(destinationRoot);
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to create destination root: " << e.what() << std::endl;
        return "";
    }

    // 5) 是否打包（基础版：若未启用打包，则直接镜像拷贝；启用打包则调用打包器）
    if (config->isPackingEnabled()) {
        std::cout << "Packing files: " << filesToBackup.size() << std::endl;
        std::unique_ptr<IPack> packer = nullptr;
        try {
            packer = PackFactory::createPacker(config->getPackType());
        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to create packer: " << e.what() << std::endl;
            return "";
        }

        // 基础实现：将收集的文件直接打包到目标目录下（由具体打包器决定扩展名）
        // 调用打包器打包文件
        const std::string packedFilePath = packer->pack(filesToBackup, destinationRoot);
        std::string compressedFilePath;
        destPath = packedFilePath;
        if (packedFilePath.empty()) {
            std::cerr << "Error: Failed to pack files" << std::endl;
            return "";
        }

        // 5.1)  打包完判断是否需要压缩
        if (config->isCompressionEnabled()) {
            std::cout << "Compressing file: " << packedFilePath << std::endl;
            std::unique_ptr<ICompress> compress = nullptr;
            try {
                compress = CompressFactory::createCompress(config->getCompressionType());
            } catch (const std::exception& e) {
                std::cerr << "Error: Failed to create compress: " << e.what() << std::endl;
                return "";
            }
            // 这个路径需要斟酌一下，或许增加一个后缀，总之是要将之前的文件给覆盖了
            compressedFilePath = compress->compressFile(packedFilePath);
            destPath = compressedFilePath;
            std::cout << "Compressed file path: " << compressedFilePath << std::endl;
             if (compressedFilePath.empty()) {
                std::cerr << "Error: Failed to compress file" << std::endl;
                return "";
            }
            // 要是压缩成功的话就把之前的文件删掉
             if (fs::exists(compressedFilePath)) {
                try {
                    fs::remove(packedFilePath);
                } catch (const std::exception& e) {
                    std::cerr << "Error: Failed to remove packed file: " << e.what() << std::endl;
                    return "";
                }
            }
        }

        // 5.2) 是否需要加密（这个应该是和压缩平行的，但是执行上需要在压缩后面）
         if (config->isEncryptionEnabled()) {
            std::cout << "Encrypting file: " << (compressedFilePath.empty() ?
                packedFilePath : compressedFilePath) << std::endl;
            std::unique_ptr<IEncrypt> encrypt = nullptr;
            try {
                encrypt = EncryptFactory::createEncryptor(config->getEncryptType());
            } catch (const std::exception& e) {
                std::cerr << "Error: Failed to create encrypt: " << e.what() << std::endl;
                return "";
            }
            // 加密文件
            const std::string encryptedFilePath = encrypt->encryptFile((compressedFilePath.empty() ?
                packedFilePath : compressedFilePath), config->getEncryptionKey());
            destPath = encryptedFilePath;
            std::cout << "Encrypted file path: " << encryptedFilePath << std::endl;
            if (encryptedFilePath.empty()) {
                std::cerr << "Error: Failed to encrypt file" << std::endl;
                return "";
            }
            // 要是加密成功的话就把之前的文件删掉
            if (fs::exists(encryptedFilePath)) {
                try {
                    fs::remove((compressedFilePath.empty() ? packedFilePath : compressedFilePath));
                } catch (const std::exception& e) {
                    std::cerr << "Error: Failed to remove compressed file: " << e.what() << std::endl;
                    return "";
                }
            }
        }

        return destPath;
    }

    // 6) 非打包路径：直接拷贝。若是目录，保持相对路径结构拷贝到 destinationRoot
    // // 之前这个有问题，直接备份的话会导致不同备份的覆盖，所以这里在外面创建一个时间戳文件名的目录，里面再copy
    // // 在之前的基础上，再创建一个时间戳目录
    // std::string timestamp = std::to_string(std::time(nullptr));
    // std::string newDir = destinationRoot + "\\" + timestamp;
    // 更新的设计感觉还是有问题，主要问题在于常规思路如果有重名的话最好的方式应该是告知用户，由用户来处理冲突
    std::string checkPath = destinationRoot + "\\" + filesToBackup[0].substr(filesToBackup[0].find_last_of("\\") + 1);
    if (fs::exists(checkPath)) {
        // 如果目标目录存在同名文件，询问用户是否覆盖
        char choice;
        std::cout << "File '" << checkPath << "' already exists.\n";
        std::cout << "Do you want to overwrite it? (y/n): ";
        std::cin >> choice;

        if (choice != 'y' && choice != 'Y') {
            std::cout << "Backup cancelled.\n";
            return "";
        }

        // 选择覆盖，先删除原有目录
        try {
            fs::remove_all(checkPath);
        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to remove existing file: " << e.what() << std::endl;
            return "";
        }
    }

    try {
        // 确保目标根目录存在
        fs::create_directories(destinationRoot);

        // 获取源根目录路径
        const std::string& sourceRoot = sourceRoots[0];
        const fs::path sourceRootPath(sourceRoot);
        std::string backupRoot = destinationRoot;

        // 遍历所有收集的条目（先根遍历结果）
        for (const auto& entry : filesToBackup) {
            // 计算相对路径
            std::string relativePath;
            if (fs::is_directory(sourceRoot)) {
                // 应该是当前entry和sourceRoot父目录的相对路径
                relativePath = fs::relative(entry, sourceRootPath.parent_path()).string();
            } else {
                // 如果源是文件，直接使用文件名
                relativePath = sourceRootPath.filename().string();
            }

            // 构建目标路径
            fs::path destinationPath = fs::path(backupRoot) / relativePath;

            try {
                if (fs::is_directory(entry)) {
                    // 创建目录（如果不存在）
                    fs::create_directories(destinationPath);
                } else if (fs::is_regular_file(entry)) {
                    // 确保目标文件的父目录存在
                    fs::create_directories(destinationPath.parent_path());
                    // 复制文件
                    CopyFileBinary(entry, destinationPath.string());
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing " << entry << ": " << e.what() << std::endl;
                return "";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to create destination directory: " << e.what() << std::endl;
        return "";
    }

    destPath = checkPath;
    return destPath;
}
