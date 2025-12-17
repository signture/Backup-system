// Copyright [2025] <JiJun Lu, Linru Zhou>
#ifndef INCLUDE_ICOMPRESS_H_
#define INCLUDE_ICOMPRESS_H_

#include <string>
#include <vector>
#include <memory>

enum class CompressType : uint8_t {
    None = 0,
    Huffman = 1,
    LZ77 = 2,
};


// 压缩算法抽象接口
class ICompress {
 public:
    // 虚析构函数（确保子类析构正常调用）
    virtual ~ICompress() = default;

    // 压缩文件（源路径）  输出压缩后的文件路径
    virtual std::string compressFile(const std::string& sourcePath) = 0;

    // 解压缩文件（源路径→目标路径）
    virtual bool decompressFile(const std::string& sourcePath, const std::string& destPath) = 0;

    // 获取压缩算法类型
    virtual CompressType getCompressType() const = 0;

    // 获取压缩算法名称
    virtual std::string getCompressTypeName() const = 0;

    // // 压缩内存数据（源数据→目标数据）
    // virtual bool compressData(const std::vector<char>& sourceData, std::vector<char>& destData) = 0;

    // // 解压缩内存数据（源数据→目标数据）
    // virtual bool decompressData(const std::vector<char>& sourceData, std::vector<char>& destData) = 0;

    // // 设置压缩级别（1-9，级别越高压缩率越高）
    // virtual void setCompressionLevel(int level) = 0;

    // // 获取压缩文件的扩展名（如"gz"、"zip"）
    // virtual std::string getFileExtension() const = 0;
};

#endif  // INCLUDE_ICOMPRESS_H_
