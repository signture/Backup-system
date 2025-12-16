// Encoding: UTF-8
#ifndef COMPRESSFACTORY_H
#define COMPRESSFACTORY_H

#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include "ICompress.h"  // 依赖ICompress抽象类
#include "HuffmanCompress.h"

// 压缩工厂类：负责责创建不同类型的压缩器实例
class CompressFactory {
public:
    // 根据静态压缩缩类型创建对应的压缩器（返回ICompress智能指针）
    static std::unique_ptr<ICompress> createCompress(const std::string& compressType);

    // 获取支持的压缩类型
    static std::vector<std::string> getSupportedCompressTypes();

    // 检测指定压缩类型是否支持
    static bool isCompressTypeSupported(const std::string& compressType);

    // 检查文件压缩类型
    static std::string getCompressType(const std::string& filePath);

    // 检查文件是否为压缩文件
    static bool isCompressedFile(const std::string& filePath);

    // 将字符串类型转换为CompressType枚举类型
    static CompressType stringToCompressType(const std::string& compressType);

    // 将CompressType枚举类型转换为字符串类型
    static std::string compressTypeToString(CompressType compressType);
};

#endif // COMPRESSFACTORY_H