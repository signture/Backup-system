// Copyright [2025] <JiJun Lu, Linru Zhou>
#include "PackFactory.h"


std::unique_ptr<IPack> PackFactory::createPacker(const std::string& packType) {
    if (!isPackTypeSupported(packType)) {
        throw std::runtime_error("Unknown pack type: " + packType);
    }
    if (packType == "Basic") {
        return std::make_unique<myPack>();
    }
    return nullptr;
}

std::vector<std::string> PackFactory::getSupportedPackTypes() {
    return {"Basic"};
}

bool PackFactory::isPackTypeSupported(const std::string& packType) {
    const auto& supportTypes = getSupportedPackTypes();
    return std::find(supportTypes.begin(), supportTypes.end(), packType) != supportTypes.end();
}

std::string PackFactory::getPackType(const std::string& filePath) {
    // 根据文件的第二个字节判断打包类型，第一个字节适用于判断当前文件是否为打包文件的标志位
    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Failed to open file " << filePath << " for reading.\n";
        return "";
    }
    // 跳过第一个字节
    in.seekg(1, std::ios::beg);
    PackType type;
    in.read(reinterpret_cast<char*>(&type), sizeof(type));
    if (type == PackType::Basic) {
        return "Basic";
    }
    return "";
}

bool PackFactory::isPackedFile(const std::string& filePath) {
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
    return firstByte == 0x01 && getPackType(filePath) != "";
}
