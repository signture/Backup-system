#ifndef ENCRYPTFACTORY_H
#define ENCRYPTFACTORY_H

#include "IEncrypt.h"
#include "SimpleXOREncrypt.h"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

class EncryptFactory {
public:
    // 创建加密器
    static std::unique_ptr<IEncrypt> createEncryptor(const std::string& encryptType);

    // 获取支持的加密类型
    static std::vector<std::string> getSupportedEncryptTypes();

    // 检查指定类型是否支持
    static bool isEncryptTypeSupported(const std::string& encryptType);

    // 检查文件加密类型
    static std::string getEncryptType(const std::string& filePath);

    // 检查文件是否为加密文件
    static bool isFileEncrypted(const std::string& filePath);

    // 将类型从string类型转换为EncryptType枚举类型
    static EncryptType stringToEncryptType(const std::string& encryptType);

    // 将类型从EncryptType枚举类型转换为string类型
    static std::string encryptTypeToString(EncryptType encryptType);
};


#endif