#include "EncryptFactory.h"


// 将类型从string类型转换为EncryptType枚举类型
EncryptType EncryptFactory::stringToEncryptType(const std::string& encryptType){
    if(encryptType == "SimXOR"){
        return EncryptType::SimXOR;
    }
    // else if(){

    // }
    return EncryptType::None;
}


// 将类型从EncryptType枚举类型转换为string类型
std::string EncryptFactory::encryptTypeToString(EncryptType encryptType){
    switch(encryptType){
        case EncryptType::SimXOR:
            return "SimXOR";
        default:
            return "None";
    }
}



std::unique_ptr<IEncrypt> EncryptFactory::createEncryptor(const std::string& encryptType){
    // 类型转换
    EncryptType entype = stringToEncryptType(encryptType);
    switch(entype){
        case EncryptType::SimXOR:
            return std::make_unique<SimpleXOREncrypt>();
        default:
            std::cerr << "Error: Unknown encrypt type." << std::endl;
            return nullptr;
    }
}

// 获取支持的加密类型
std::vector<std::string> EncryptFactory::getSupportedEncryptTypes(){
    // 但是这样每次都要维护，有没有更为高效的方案
    return {"SimXOR"};
}


// 检查指定类型是否支持
bool EncryptFactory::isEncryptTypeSupported(const std::string& encryptType){
    const auto& supportTypes = getSupportedEncryptTypes();
    return std::find(supportTypes.begin(), supportTypes.end(), encryptType) != supportTypes.end();
}


// 检查文件加密类型
std::string EncryptFactory::getEncryptType(const std::string& filePath){
    // 根据文件的第二个字节判断压缩类型，第一个字节适用于判断当前文件是否为压缩文件的标志位
    std::ifstream in(filePath, std::ios::binary);
    if(!in){
        std::cerr << "Error: Failed to open file " << filePath << " for reading.\n";
        return "";
    }
    // 跳过第一个字节
    in.seekg(1, std::ios::beg);
    EncryptType type;
    in.read(reinterpret_cast<char*>(&type), sizeof(type));
    return encryptTypeToString(type);
}


// 检查文件是否为加密文件
bool EncryptFactory::isFileEncrypted(const std::string& filePath){
    // 应该是第一个位既是标志位，第二个字节也是符合条件的压缩类型
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
    return firstByte == 0x31 && getEncryptType(filePath) != "";
}