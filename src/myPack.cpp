# include "myPack.h"

// 定义辅助函数，用于确认文件类型
FileType getFileType(const std::filesystem::path& path){
    try {
        if(std::filesystem::is_directory(path)){
            return FileType::Directory;
        }
        else if(std::filesystem::is_regular_file(path)){
            return FileType::Regular;
        }
        else{
            std::cerr << "Warning: File type of " << path.string() << " is not supported, but processed as Regular file.\n";
            return FileType::Regular;
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error checking file type of " << path.string() << ": " << e.what() << ", processed as Regular file.\n";
        return FileType::Regular;
    }
}


std::string myPack::pack(const std::vector<std::string>& files, const std::string& destPath) {
    // 首先检查是不是空的文件列表
    if(files.empty())   return "";

    std::vector<FileMeta> metas;
    // 包头长度 = 是否打包（1字节） + 算法类型(1字节) + 文件数量(4字节) + 内容区起始位置(4字节)
    size_t headerLen = 1 + 1 + 4 + 4;

    // 尝试从文件列表中确定根目录
    std::string rootPath = "";
    if (!files.empty()) {
        rootPath = std::filesystem::path(files[0]).parent_path().string();
    }

    // 元数据区长度
    size_t metaLen = 0;
    // 记录当前偏移量，初始为内容区起始位置s
    uint64_t currentOffset = 0;
    for(const auto& file : files){
        // 计算相对于根目录的路径
        std::string relativePath = file;
        if (!rootPath.empty()) {
            try {
                relativePath = std::filesystem::relative(std::filesystem::path(file), std::filesystem::path(rootPath)).string();
                // 对于根目录本身，使用空字符串或"."表示当前目录
                if (relativePath.empty() || relativePath == "..") {
                    relativePath = ".";
                }
            } catch (...) {
                // 如果无法计算相对路径，使用原始路径
                relativePath = file;
            }
        }

        metaLen += 4; // 文件名长度
        metaLen += relativePath.size(); // 文件名内容
        metaLen += 8; // 文件大小
        metaLen += 8; // 文件偏移量
        metaLen += 1; // 文件类型

        // 记录文件类型
        FileType type = getFileType(file);
        // 判断文件大小，目录文件大小为0
        uint64_t size = (type == FileType::Regular) ? 
                        (std::filesystem::exists(file) ? std::filesystem::file_size(file) : 0) : 0;
        // 记录文件名长度
        uint32_t nameLen = relativePath.size();
        // 记录文件类型
        metas.push_back({nameLen, relativePath, size, currentOffset, type});
        currentOffset += size;
    }
    uint32_t contentStart = headerLen + metaLen;


    const std::string baseName = "backup_" + std::to_string(time(nullptr)) + "." + getPackTypeName();
    std::filesystem::path destPackPath = std::filesystem::path(destPath) / baseName;

    // 确保目标目录存在
    try {
        std::filesystem::path destDirPath = destPackPath.parent_path();
        if (!std::filesystem::exists(destDirPath)) {
            std::filesystem::create_directories(destDirPath);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to create destination directory " << destPath << ": " << e.what() << "\n";
        return "";
    }

    const std::string destPackBase = destPackPath.string();


    // 接下来写入包头（包括打包算法，当前包包含的文件数量，文件的元信息）
    std::ofstream out(destPackBase, std::ios::binary);
    if(!out){
        std::cerr << "Error: Failed to open file " << destPackBase << " for writing.\n";
        return "";
    }
    // 写入是否打包（1字节）
    uint8_t isPacked = 1;
    out.write(reinterpret_cast<const char*>(&isPacked), sizeof(isPacked));

    // 写入打包算法（1字节）
    PackType type = PackType::Basic;
    out.write(reinterpret_cast<const char*>(&type), sizeof(type));

    // 写入当前包包含的文件数量（4字节）
    uint32_t fileCount = metas.size();
    out.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));

    // 写入头信息长度（4字节）
    out.write(reinterpret_cast<const char*>(&contentStart), sizeof(contentStart));

    // 写入文件元信息
    for(const auto& meta : metas){
        out.write(reinterpret_cast<const char*>(&meta.nameLen), sizeof(meta.nameLen));
        out.write(meta.name.c_str(), meta.nameLen);
        out.write(reinterpret_cast<const char*>(&meta.size), sizeof(meta.size));
        out.write(reinterpret_cast<const char*>(&meta.offset), sizeof(meta.offset));
        out.write(reinterpret_cast<const char*>(&meta.type), sizeof(meta.type)); 
    }

    // 写入文件内容（按顺序排列）（这里只写入普通文件的内容）
    for(const auto& meta : metas){
        // 只写入普通文件的内容
        if(meta.type != FileType::Regular) continue;

        std::filesystem::path fullFilePath = std::filesystem::path(rootPath) / meta.name;
        // 防止路径过长
        // 但是有可能文件路径过长，超过了系统限制
        #ifdef _WIN32
            std::string fullPath = fullFilePath.string();
            if(fullPath.size() > PATH_MAX){
                // 为本地路径添加 \\?\ 前缀
                if (fullPath.find("\\\\") != 0) {
                    fullPath = "\\\\?\\" + fullPath;
                }
                // 为UNC路径添加 \\?\UNC\ 前缀
                else {
                    fullPath = "\\\\?\\UNC\\" + fullPath.substr(2);
                }
                // 使用带有长路径前缀的路径重新创建filesystem::path对象
                fullFilePath = std::filesystem::path(fullPath);
            }
        #endif

        std::ifstream in(fullFilePath, std::ios::binary);
        if(!in){
            std::cerr << "Error: Failed to open file " << fullFilePath.string() << " for reading.\n";
            return "";
        }
        std::vector<char> buffer(meta.size);
        in.read(buffer.data(), meta.size);
        out.write(buffer.data(), meta.size);
    }

    out.close();
    std::cout << "Packing " << files.size() << " files to " << destPackBase << " using " << getPackTypeName() << "Packer.\n";
    return destPackBase;
}

bool myPack::unpack(const std::string& srcPath, const std::string& destDir) {
    std::ifstream in(srcPath, std::ios::binary);
    if(!in){
        std::cerr << "Error: Failed to open file " << srcPath << " for reading.\n";
        return false;
    }

    // 检查是否是打包文件
    uint8_t isPacked;
    in.read(reinterpret_cast<char*>(&isPacked), sizeof(isPacked));
    if(isPacked != 1){
        // 不是打包文件，返回错误信息
        std::cerr << "Error: File " << srcPath << " is not packed.\n";
        return false;
    }

    // 读取包头
    // 读取打包算法类型(1字节)
    PackType type;
    in.read(reinterpret_cast<char*>(&type), sizeof(type));
    if(type != PackType::Basic){
        // 匹配失败，返回错误信息
        std::cerr << "Error: Packing algorithm type in " << srcPath << " is not Basic.\n";
        return false;
    }

    // 读取文件数量（4字节）
    uint32_t fileCount;
    in.read(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
    std::vector<FileMeta> metas(fileCount);
    std::cout << "Unpacking " << fileCount << " files from " << srcPath << " to " << destDir << ".\n";

    // 读取头信息长度（4字节）
    uint32_t contentStart;
    in.read(reinterpret_cast<char*>(&contentStart), sizeof(contentStart));

    // 读取文件元信息
    for(auto& meta : metas){
        in.read(reinterpret_cast<char*>(&meta.nameLen), sizeof(meta.nameLen));
        meta.name.resize(meta.nameLen);
        in.read(&meta.name[0], meta.nameLen);
        in.read(reinterpret_cast<char*>(&meta.size), sizeof(meta.size));
        in.read(reinterpret_cast<char*>(&meta.offset), sizeof(meta.offset));
        in.read(reinterpret_cast<char*>(&meta.type), sizeof(meta.type));
    }

    // 遍历构建目录结构，根据不同文件类型区分进行构建
    for(const auto& meta : metas){
        switch(meta.type){
            // 普通文件
            case FileType::Regular:{
                // 定义到对应的文件内容offset（必须是当前流位置开始，也就是元数据区末尾）
                in.seekg(meta.offset + contentStart, std::ios::beg);
                // 写入
                std::filesystem::path outPath = std::filesystem::path(destDir) / meta.name;
                // 但是有可能文件路径过长，超过了系统限制
                // 参考：https://learn.microsoft.com/zh-cn/windows/win32/fileio/maximum-file-path-limitation?tabs=registry
                #ifdef _WIN32
                    std::string fullPath = outPath.string();
                    if(fullPath.size() > PATH_MAX){
                        // 为本地路径添加 \\?\ 前缀
                        if (fullPath.find("\\\\") != 0) {
                            fullPath = "\\\\?\\" + fullPath;
                        }
                        // 为UNC路径添加 \\?\UNC\ 前缀
                        else {
                            fullPath = "\\\\?\\UNC\\" + fullPath.substr(2);
                        }
                        // 使用带有长路径前缀的路径重新创建filesystem::path对象
                        outPath = std::filesystem::path(fullPath);
                    }
                #endif

                // 确保目标目录存在
                if(!std::filesystem::exists(outPath.parent_path())){
                    // 输出目录路径
                    std::cout << "Unpacking directory " << meta.name << " to " << outPath.parent_path() << ".\n";
                    std::filesystem::create_directories(outPath.parent_path());
                }

                std::ofstream out(outPath, std::ios::binary);

                if(!out){
                    std::cerr << "Error: Failed to open file " << outPath << " for writing.\n";
                    return false;
                }

                const size_t MAX_BUFFER_SIZE = 1024 * 1024; // 1MB
                size_t remainingSize = meta.size;
                
                // 合理性检查：如果文件大小异常大，记录警告
                if(meta.size > 1024 * 1024 * 1024) { // 大于1GB
                    std::cerr << "Warning: Unexpectedly large file size (" << meta.size << ") for " << meta.name << ".\n";
                }
                
                std::vector<char> buffer(std::min(MAX_BUFFER_SIZE, remainingSize));
                while(remainingSize > 0) {
                    size_t toRead = std::min(buffer.size(), remainingSize);
                    in.read(buffer.data(), toRead);
                    size_t bytesRead = in.gcount();
                    if(bytesRead == 0 && remainingSize > 0) {
                        std::cerr << "Error: Unexpected end of file while reading " << meta.name << ".\n";
                        return false;
                    }
                    out.write(buffer.data(), bytesRead);
                    remainingSize -= bytesRead;
                }
                out.close();
                break;
            }

            // 目录文件
            case FileType::Directory:{
                // 构建目录
                std::filesystem::path outPath = std::filesystem::path(destDir) / meta.name;

                if(!std::filesystem::create_directory(outPath)){
                    std::cerr << "Error: Failed to create directory " << outPath << ".\n";
                    return false;
                }
                break;
            }

            default:{
                std::cerr << "Error: Unknown file type " << static_cast<int>(meta.type) << " in " << srcPath << ".\n";
                break;
            }
        }
    }

    in.close();
    std::cout << "Unpacking " << fileCount << " files from " << srcPath << " to " << destDir << " using BasicPacker.\n";
    return true;
}

