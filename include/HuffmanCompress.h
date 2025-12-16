#ifndef HUFFMAN_COMPRESS_H
#define HUFFMAN_COMPRESS_H

#include "ICompress.h"
#include "CRC32.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>
#include <array>
#include <queue>
#include <memory>
#include <vector>

#define BUFF_SIZE 1 << 16 // 缓冲区大小 64KB


struct HNode{
    uint64_t freq;
    uint8_t byte;
    HNode* left;
    HNode* right;
    HNode(uint64_t f, uint8_t b, HNode* l=nullptr, HNode* r=nullptr) : freq(f), byte(b), left(l), right(r) {}
    bool isLeaf() const {return left == nullptr && right == nullptr; }
};

struct HNodeCmp{
    bool operator()(const HNode* a, HNode* b) const{
        return a->freq > b->freq;
    }
};

struct Head{
    uint8_t isCompress; // 是否压缩，0x21为压缩，0x20为不压缩，1字节
    CompressType compressType; // 压缩算法类型，固定为1字节
    uint8_t validBits; // 最后一个字节的有效位，1字节
    uint8_t reservedBits; // 填充位，用于填充到字节边界，1字节
    uint32_t headerSize; // 头大小， 4字节
    uint32_t freqTableSize;   // 词频表大小，4字节
    uint64_t originalSize; // 原始文件大小, 8字节
    uint32_t crc32; // CRC32校验值，4字节
};  // 24字节

class HuffmanCompress : public ICompress {
public:
    CompressType getCompressType() const override { return CompressType::Huffman; }
    std::string getCompressTypeName() const override { return "Huffman"; }
    // 直接原地覆盖压缩，返回压缩后的文件路径
    std::string compressFile(const std::string& sourcePath) override;
    bool decompressFile(const std::string& sourcePath, const std::string& destPath) override;

private:
    //  统计字节形成的字符串词频（固定256个）
    static bool readFreqTable(const std::string& sourcePath, std::array<uint64_t, 256>& freqTable, uint64_t& originalSize);
    // 构造哈夫曼树
    static HNode* buildHuffmanTree(const std::array<uint64_t, 256>& freqTable);
    // 生成哈夫曼编码(递归构造)
    static void generateHuffmanCodes(HNode* node, std::vector<bool>& cur, 
                                    std::array<std::vector<bool>, 256>& codes);
    // 生成哈夫曼编码表
    static std::array<std::vector<bool>, 256> generateHuffmanCodes(HNode* root);
    // 删除树
    static void deleteHuffmanTree(HNode* node){
        if(!node) return;
        deleteHuffmanTree(node->left);
        deleteHuffmanTree(node->right);
        delete node;
    }
};


#endif
