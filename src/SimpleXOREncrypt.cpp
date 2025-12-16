#include "SimpleXOREncrypt.h"

std::string SimpleXOREncrypt::encryptFile(const std::string& sourcePath, const std::string& key){
    // 首先检查文件是否存在
    if(!std::filesystem::exists(sourcePath)){
        std::cerr << "Error: File " << sourcePath << " does not exist." << std::endl;
        return "";
    }

    // 设置加密后路径
    std::string destPath = sourcePath + ".enc";

    // 打开文件
    std::ifstream inFile(sourcePath, std::ios::binary);
    if(!inFile.is_open()){
        std::cerr << "Error: Failed to open file " << sourcePath << " for reading." << std::endl;
        return "";
    }

    // 打开加密文件
    std::ofstream outFile(destPath, std::ios::binary);
    if(!outFile.is_open()){
        std::cerr << "Error: Failed to open file " << destPath << " for writing." << std::endl;
        return "";
    }

    // 写入头信息
    EncHead head;
    head.isEncrypt = 0x31;
    head.encryptType = EncryptType::SimXOR;
    head.headerSize = sizeof(EncHead);
    head.crc32 = 0; // 后续计算
    outFile.write(reinterpret_cast<const char*>(&head), sizeof(EncHead));

    // 执行简单加密
    std::string password = key;
    std::vector<char> buffer(BUFFER_SIZE);
    size_t keyIndex = 0;
    size_t bytesRead;
    // 添加空密钥处理
    size_t keySize = key.size();
    // 空密钥处理：使用默认密钥
    if (keySize == 0) {
        // 使用默认密钥
        password = DEFAULT_KEY;
        keySize = password.size();
    }
    
    uint32_t crc32 = CRC32::getInitialValue();

    while((bytesRead = inFile.read(buffer.data(), BUFFER_SIZE).gcount())){
        for(size_t i = 0; i < bytesRead; ++i){
            // 计算crc
            crc32 = CRC32::update(crc32, static_cast<uint8_t>(buffer[i]));

            // 加密内容
            buffer[i] ^= password[keyIndex];
            keyIndex = (keyIndex + 1) % keySize;
        }
        outFile.write(buffer.data(), bytesRead);
    }

    // 写入crc32
    head.crc32 = CRC32::finalize(crc32);
    outFile.seekp(0, std::ios::beg);
    outFile.write(reinterpret_cast<const char*>(&head), sizeof(EncHead));



    // 关闭文件
    inFile.close();
    outFile.close();

    return destPath;
}


// 解密文件
bool SimpleXOREncrypt::decryptFile(const std::string& sourcePath, const std::string& destPath, const std::string& key){
    // 首先检查文件是否存在
    if(!std::filesystem::exists(sourcePath)){
        std::cerr << "Error: File " << sourcePath << " does not exist." << std::endl;
        return false;
    }

    // 打开文件
    std::ifstream inFile(sourcePath, std::ios::binary);
    if(!inFile.is_open()){
        std::cerr << "Error: Failed to open file " << sourcePath << " for reading." << std::endl;
        return false;
    }

    // 读取头信息
    EncHead head;
    inFile.read(reinterpret_cast<char*>(&head), sizeof(EncHead));
    if(!inFile.good()){
        std::cerr << "Error: Failed to read header from file " << sourcePath << "." << std::endl;
        return false;
    }

    // 检查是否为加密文件
    if(head.isEncrypt != 0x31 || head.encryptType != EncryptType::SimXOR){
        std::cerr << "Error: File " << sourcePath << " is not an encrypted file." << std::endl;
        return false;
    }

    // 打开解密文件
    std::ofstream outFile(destPath, std::ios::binary);
    if(!outFile.is_open()){
        std::cerr << "Error: Failed to open file " << destPath << " for writing." << std::endl;
        return false;
    }

    // 执行XOR解密
    std::vector<char> buffer(BUFFER_SIZE);
    std::string password = key;
    size_t keySize = key.size();
    size_t keyIndex = 0;
    size_t bytesRead;
    uint32_t crc32 = CRC32::getInitialValue(); 

    // 添加空密钥处理
    if (keySize == 0) {
        // 使用默认密钥
        password = DEFAULT_KEY;
        keySize = password.size();
    }

    while((bytesRead = inFile.read(buffer.data(), BUFFER_SIZE).gcount()) > 0) {
        // 对数据进行CRC计算
        for(size_t i = 0; i < bytesRead; ++i) {  
            // 先解密，再计算crc32
            // 解密数据
            buffer[i] ^= password[keyIndex];
            keyIndex = (keyIndex + 1) % keySize;
            // 计算crc
            crc32 = CRC32::update(crc32, static_cast<uint8_t>(buffer[i]));
        }
        
        // 写入解密后的数据
        outFile.write(buffer.data(), bytesRead);
    }

    // 完成CRC计算，获取最终结果
    crc32 = CRC32::finalize(crc32);
    if(crc32 != head.crc32) {
        std::cerr << "Error: CRC32 checksum mismatch. File may be corrupted." << std::endl;
        return false;
    }

    // 关闭文件
    inFile.close();
    outFile.close();

    return true;
}