#ifndef CFACTORY_H
#define CFACTORY_H

#include "ICompress.h"
#include "IPack.h"
#include "IEncrypt.h"
#include <memory>
#include <string>

// 工厂类，用于创建不同的压缩、打包和加密类
class CompressFactory {
public:
    // 创建压缩器
    static std::unique_ptr<ICompress> createCompressor(const std::string& compressType);
    
    // 创建打包器
    static std::unique_ptr<IPack> createPacker(const std::string& packType);
    
    // 创建加密器
    static std::unique_ptr<IEncrypt> createEncryptor(const std::string& encryptType);
};
#endif
