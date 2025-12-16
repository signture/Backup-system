#ifndef SIMPLEXORENCRYPT_H
#define SIMPLEXORENCRYPT_H

#include "IEncrypt.h"
#include "CRC32.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>

#define BUFFER_SIZE (1 << 16) // 缓冲区大小 64KB

struct EncHead{
    uint8_t isEncrypt; // 是否加密，0x31为加密，0x30为不加密，1字节
    EncryptType encryptType; // 加密算法类型，固定为1字节
    uint32_t headerSize; // 头大小， 4字节
    uint32_t crc32; // CRC32校验值，4字节
};


class SimpleXOREncrypt : public IEncrypt {
public:
    EncryptType getEncryptType() const override { return EncryptType::SimXOR; }
    std::string getEncryptTypeName() const override { return "SimXOR"; }

    // 加密文件
    std::string encryptFile(const std::string& sourcePath, const std::string& key) override;

    // 解密文件
    bool decryptFile(const std::string& sourcePath, const std::string& destPath, const std::string& key) override;
};

#endif