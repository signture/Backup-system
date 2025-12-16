#include "PackFactory.h"
<<<<<<< HEAD
#include <iostream>

// 基础空实现的打包器：仅占位，返回成功/失败标志
class BasicPacker : public IPack {
public:
    bool pack(const std::vector<std::string>& /*files*/, const std::string& /*destPath*/) override {
        // 占位实现：直接提示未真正实现，返回false表示未打包
        std::cout << "[BasicPacker] pack() not implemented, skip.\n";
        return false;
    }

    bool unpack(const std::string& /*srcPath*/, const std::string& /*destDir*/) override {
        std::cout << "[BasicPacker] unpack() not implemented, skip.\n";
        return false;
    }
};

std::unique_ptr<IPack> PackFactory::createPacker(const std::string& /*packType*/){
    // 目前统一返回占位实现，避免链接错误。
    return std::make_unique<BasicPacker>();
}

std::vector<std::string> PackFactory::getSupportedPackTypes(){
    return {"basic"};
}

bool PackFactory::isPackTypeSupported(const std::string& packType){
    return packType == "basic";
}

=======


std::unique_ptr<IPack> PackFactory::createPacker(const std::string& packType){
    if(!isPackTypeSupported(packType)){
        throw std::runtime_error("Unknown pack type: " + packType);
    }
    if(packType == "Basic"){
        return std::make_unique<myPack>();
    }
    return nullptr;
}

std::vector<std::string> PackFactory::getSupportedPackTypes(){
    return {"Basic"};
}

bool PackFactory::isPackTypeSupported(const std::string& packType){
    const auto& supportTypes = getSupportedPackTypes();
    return std::find(supportTypes.begin(), supportTypes.end(), packType) != supportTypes.end();
}

std::string PackFactory::getPackType(const std::string& filePath) {
    // 根据文件的第二个字节判断打包类型，第一个字节适用于判断当前文件是否为打包文件的标志位
    std::ifstream in(filePath, std::ios::binary);
    if(!in){
        std::cerr << "Error: Failed to open file " << filePath << " for reading.\n";
        return "";
    }
    // 跳过第一个字节
    in.seekg(1, std::ios::beg);
    PackType type;
    in.read(reinterpret_cast<char*>(&type), sizeof(type));
    if(type == PackType::Basic){
        return "Basic";
    }
    return "";
}

bool PackFactory::isPackedFile(const std::string& filePath) {
    // 添加目录检查
    if(std::filesystem::is_directory(filePath)){
        return false;
    }
// 然后再尝试打开文件
    if(!std::filesystem::exists(filePath)){
        std::cerr << "Error: File " << filePath << " does not exist.\n";
        return false;
    }
    std::ifstream in(filePath, std::ios::binary);
    if(!in){
        std::cerr << "Error: Failed to open file " << filePath << " for reading.\n";
        return false;
    }
    uint8_t firstByte;
    in.read(reinterpret_cast<char*>(&firstByte), sizeof(firstByte));
    return firstByte == 0x01 && getPackType(filePath) != "";
}
>>>>>>> encryptFunction

