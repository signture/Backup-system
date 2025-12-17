// Copyright [2025] <JiJun Lu, Linru Zhou>
#ifndef INCLUDE_IENCRYPT_H_
#define INCLUDE_IENCRYPT_H_

#include <string>
#include <vector>

#define DEFAULT_KEY "default_key"

// 加密器类型枚举
enum class EncryptType : uint8_t{
    None = 0,
    SimXOR = 1,
    AES = 2,
    DES = 3,
};


// IEncryptor抽象类
class IEncrypt {
 public:
    // 虚析构函数
    virtual ~IEncrypt() = default;

    // 加密文件，输出加密后的文件路径
    virtual std::string encryptFile(const std::string& sourcePath, const std::string& key) = 0;

    // 解密文件，将文件从源路径解压至目标路径
    virtual bool decryptFile(const std::string& sourcePath, const std::string& destPath, const std::string& key) = 0;

    // 获取加密器类型
    virtual EncryptType getEncryptType() const = 0;

    // 获取加密算法名称
    virtual std::string getEncryptTypeName() const = 0;
};

#endif  // INCLUDE_IENCRYPT_H_
