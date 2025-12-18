// Copyright [2025] <JiJun Lu, Linru Zhou>
#include "CompressFactory.h"
#include <stdexcept>

// 字符串压缩类型转换为枚举类型
CompressType CompressFactory::stringToCompressType(const std::string& compressType) {
    if (compressType == "Huffman") {
        return CompressType::Huffman;
    }
    // 后续继续补充
    throw std::runtime_error("Unknown compress type: " + compressType);
}

// 枚举压缩类型转换为字符串类型
std::string CompressFactory::compressTypeToString(CompressType compressType) {
    if (compressType == CompressType::Huffman) {
        return "Huffman";
    }
    // 后续继续补充
    throw std::runtime_error("Unknown compress type");
}

// 根据压缩类型创建实例（工厂模式核心逻辑）
std::unique_ptr<ICompress> CompressFactory::createCompress(const std::string& compressType) {
    if (!isCompressTypeSupported(compressType)) {
        throw std::runtime_error("Unknown compress type: " + compressType);
    }
    CompressType type = stringToCompressType(compressType);
    switch (type) {
        case CompressType::Huffman:
            return std::make_unique<HuffmanCompress>();
        default:
            throw std::runtime_error("Unknown compress type: " + compressType);
    }
    // 后续继续补充
    return nullptr;
}

std::vector<std::string> CompressFactory::getSupportedCompressTypes() {
    // 后续继续补充
    return {"Huffman"};
}


bool CompressFactory::isCompressTypeSupported(const std::string& compressType) {
    const auto& supportTypes = getSupportedCompressTypes();
    return std::find(supportTypes.begin(), supportTypes.end(), compressType) != supportTypes.end();
}

std::string CompressFactory::getCompressType(const std::string& filePath) {
    // 根据文件的第二个字节判断压缩类型，第一个字节适用于判断当前文件是否为压缩文件的标志位
    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Failed to open file " << filePath << " for reading.\n";
        return "";
    }
    // 跳过第一个字节
    in.seekg(1, std::ios::beg);
    CompressType type;
    in.read(reinterpret_cast<char*>(&type), sizeof(type));
    return compressTypeToString(type);
}

bool CompressFactory::isCompressedFile(const std::string& filePath) {
    // 应该是第一个位既是标志位，第二个字节也是符合条件的压缩类型
    // 添加目录检查
    if (std::filesystem::is_directory(filePath)) {
        return false;
    }
// 然后再尝试打开文件
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Error: File " << filePath << " does not exist.\n";
        return false;
    }
    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Failed to open file " << filePath << " for reading.\n";
        return false;
    }
    uint8_t firstByte;
    in.read(reinterpret_cast<char*>(&firstByte), sizeof(firstByte));
    return firstByte == 0x21 && getCompressType(filePath) != "";
}
