// Encoding: UTF-8
#ifndef IPACK_H
#define IPACK_H

#define PATH_MAX 260

#include <string>
#include <vector>

// 打包器类型枚举
enum class PackType : uint8_t{
    Basic = 0,
    Zip = 1,
    Tar = 2,
};

// IPack 抽象类 - 文件打包与解包接口
class IPack {
public:
    virtual ~IPack() = default;

    // 打包：输入文件列表，输出打包目标路径（不含扩展名由具体实现决定）
    virtual std::string pack(const std::vector<std::string>& files, const std::string& destPath) = 0;

    // 解包：输入打包文件，输出解包目录
    virtual bool unpack(const std::string& srcPath, const std::string& destDir) = 0;

    // 获取打包器类型
    virtual PackType getPackType() const = 0;

    // 获取打包器类型名字
    virtual std::string getPackTypeName() const = 0;
};

#endif