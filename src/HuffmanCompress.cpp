#include "HuffmanCompress.h"
#include <cstdint>

namespace fs = std::filesystem;

bool HuffmanCompress::readFreqTable(const std::string& sourcePath, std::array<uint64_t, 256>& freqTable, uint64_t& originalSize){
    freqTable.fill(0);
    originalSize = 0;
    // 读取文件
    std::ifstream in(sourcePath, std::ios::binary);
    if(!in || !in.is_open()){
        std::cerr << "Error: Failed to open file " << sourcePath << " for reading.\n";
        return false;
    }

    // 统计词频
    std::vector<uint8_t> buffer(BUFF_SIZE);
    while(in){
        // 每次读取一个buffer的大小
        in.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(BUFF_SIZE));
        std::streamsize n = in.gcount();    //  可能没有满一个buffer的空间，需要查看实际读取的字节数
        if(n <= 0)  break;
        originalSize += static_cast<uint64_t>(n);
        // 统计词频
        for(std::streamsize i = 0;i < n; ++ i){
            ++freqTable[buffer[i]];
        }
    }
    return true;
}


HNode* HuffmanCompress::buildHuffmanTree(const std::array<uint64_t, 256>& freqTable){
    // 通过优先队列构造小顶堆
    std::priority_queue<HNode*, std::vector<HNode*>, HNodeCmp> pq;
    for(int i = 0;i < 256; i++){
        if(freqTable[i] > 0){
            pq.push(new HNode(freqTable[i], static_cast<uint8_t>(i)));
        }
    }

    // 特殊情况，队列为空
    if(pq.empty()) return new HNode(0, 0);

    // 特殊情况，队列只有一个符号
    // 但是不能直接返回，至少要有一个深度，得到一位的编码
    if(pq.size() == 1){
        HNode* only = pq.top(); pq.pop();
        HNode* fake = new HNode(0, 0);
        HNode* root = new HNode(only->freq + fake->freq, 0, only, fake);
        return root;
    }

    // 正常合并
    while(pq.size() > 1){
        HNode* a = pq.top(); pq.pop();
        HNode* b = pq.top(); pq.pop();
        HNode* parent = new HNode(a->freq + b->freq, 0, a, b);
        pq.push(parent);
    }
    return pq.top();
}

void HuffmanCompress::generateHuffmanCodes(HNode* node, std::vector<bool>& cur, std::array<std::vector<bool>, 256>& codes){
    if(!node) return;
    if(node->isLeaf()){
        // 将编码填入对应编码表中
        if(cur.empty()){
            codes[node->byte] = std::vector<bool>{false};
        }else{
            codes[node->byte] = cur;
        }
        return ;
    }
    // 左边是0
    cur.push_back(false);
    generateHuffmanCodes(node->left, cur, codes);
    cur.pop_back(); // 回溯

    // 右边是1
    cur.push_back(true);
    generateHuffmanCodes(node->right, cur, codes);
    cur.pop_back();
}

std::array<std::vector<bool>, 256> HuffmanCompress::generateHuffmanCodes(HNode* root){
    std::array<std::vector<bool>, 256> codes;
    std::vector<bool> cur;
    generateHuffmanCodes(root, cur, codes);
    return codes;
}


std::string HuffmanCompress::compressFile(const std::string& sourcePath){
    // 检查目标文件路径是否可以访问
    // 直接检查是不是存在不太对，因为还没创建一定不存在
    // TODO: 检查目标文件路径是否可以访问

    // 读取内容统计每个字节的出现频率
    std::array<uint64_t, 256> freq;
    uint64_t originalSize = 0;
    if(!readFreqTable(sourcePath, freq, originalSize)){
        std::cerr << "Error: Failed to read frequency table from file " << sourcePath << ".\n";
        return "";
    }
    // 构造哈夫曼树
    HNode* root = buildHuffmanTree(freq);
    if(!root){
        std::cerr << "Error: Failed to build Huffman tree.\n";
        return "";
    }

    // 生成编码表
    auto codes = generateHuffmanCodes(root);
    // 释放资源
    deleteHuffmanTree(root);
    if(codes.empty()){
        std::cerr << "Error: Failed to generate Huffman codes.\n";
        return "";
    }
    
    // 随后创建文件头，存储CRC offset 压缩算法类型
    // 在原先文件基础上增加后缀即可
    std::string destPath = sourcePath + ".huff";
    std::ofstream out(destPath, std::ios::binary);
    if(!out|| !out.is_open()){
        std::cerr << "Error: Failed to open file " << destPath << " for writing.\n";
        return "";
    }

    // 计算词频表大小
    uint32_t freqTableSize = 0;
    for(int i = 0;i < 256; i++){
        if(freq[i] > 0){
            freqTableSize += 1 + 8; // 1字节字节值 + 8字节频率
        }
    }

    // 创建文件头信息
    Head header;
    header.isCompress = 0x21;
    header.compressType = CompressType::Huffman;
    header.validBits = 0;
    header.reservedBits = 0;
    header.headerSize = sizeof(Head);
    header.freqTableSize = freqTableSize;
    header.originalSize = originalSize;
    header.crc32 = 0; // 先设为0，后续计算

    // 写入文件头
    out.write(reinterpret_cast<const char*>(&header), sizeof(Head));



    // 之后将词频表内容写入文件头
    for(int i = 0 ;i < 256; i++){
        if(freq[i] > 0){
            out.write(reinterpret_cast<const char*>(&i), 1);
            out.write(reinterpret_cast<const char*>(&freq[i]), 8);
        }
    }

    // 将内容写入文件中
    // 打开文件
    std::ifstream in(sourcePath, std::ios::binary);
    if(!in || !in.is_open()){
        std::cerr << "Error: Failed to open file " << sourcePath << " for reading.\n";
        return "";
    }

    uint32_t crcValue = 0xFFFFFFFF;

    // 准备buffer（压缩缓冲区）
    std::vector<uint8_t> readBuffer(BUFF_SIZE);
    std::vector<uint8_t> writeBuffer(BUFF_SIZE);
    std::vector<bool> bitBuffer;
    size_t writePos = 0;
    uint8_t currentByte = 0;
    int bitPosition = 0;


    // 辅助函数：将当前字节写入输出缓冲区
    auto flushCurrentByte = [&]() {
        if (writePos >= writeBuffer.size()) {
        // 输出缓冲区已满，写入文件
        out.write(reinterpret_cast<const char*>(writeBuffer.data()), writePos);
        writePos = 0;
        }
        writeBuffer[writePos++] = currentByte;
        currentByte = 0;
        bitPosition = 0;
    };


    // 压缩
    while(in){
        // 读取一块数据
        in.read(reinterpret_cast<char*>(readBuffer.data()), static_cast<std::streamsize>(BUFF_SIZE));
        std::streamsize n = in.gcount();
        if(n <= 0)  break;

        // 计算CRC
        for(std::streamsize i = 0; i < n; i++){
            crcValue = CRC32::update(crcValue, readBuffer[i]);
        }

        // 压缩数据
        for(std::streamsize i = 0;i < n; i++){
            uint8_t byte = readBuffer[i];
            const std::vector<bool>& code = codes[byte];

            for(bool bit : code){
                // 使用位运算将对应位进行填充
                currentByte |= (bit << (7 - bitPosition));
                bitPosition++;

                if(bitPosition == 8){
                    // 满了8字节，写入输出缓冲区
                    flushCurrentByte();
                }
            }
        }
    }
    // 有可能最后没满1字节
    if(bitPosition > 0){
        header.validBits = 8 - bitPosition;
        // 写入最后一个字节
        flushCurrentByte();
    }

    // 写入剩余的缓冲区内容
    if (writePos > 0) {
        out.write(reinterpret_cast<const char*>(writeBuffer.data()), writePos);
    }

    header.crc32 = crcValue ^ 0xFFFFFFFF;

    // 回写文件头
    out.seekp(0, std::ios::beg);
    out.write(reinterpret_cast<const char*>(&header), sizeof(Head));
    out.close();
    in.close();
    return destPath;
}

bool HuffmanCompress::decompressFile(const std::string& sourcePath, const std::string& destPath){
    // 打开压缩文件
     std::ifstream in(sourcePath, std::ios::binary);
    if(!in || !in.is_open()){
        std::cerr << "Error: Failed to open file " << sourcePath << " for reading.\n";
        return false;
    }

    // 打开目标文件写入
    std::ofstream out(destPath, std::ios::binary);
    if(!out || !out.is_open()){
        std::cerr << "Error: Failed to open file " << destPath << " for writing.\n";
        in.close();
        return false;
    }

    // 读取头信息
    Head header;
    in.read(reinterpret_cast<char*>(&header), sizeof(Head));

    // 验证是否位压缩文件或压缩类型
    if(header.isCompress != 0x21 || header.compressType != CompressType::Huffman){
        std::cerr << "Error: File " << sourcePath << " is not a Huffman compressed file.\n";
        in.close();
        out.close();
        return false;
    }

    // 读取词频表
    std::array<uint64_t, 256> freqTable = {0};
    for(uint32_t i = 0 ;i < header.freqTableSize;){
        uint8_t byte;
        uint64_t freq;
        in.read(reinterpret_cast<char*>(&byte), 1);
        in.read(reinterpret_cast<char*>(&freq), 8);
        freqTable[byte] = freq;
        i += 1 + 8;
    }

    // 重建 Huffman 树
    HNode* root = buildHuffmanTree(freqTable);
    if(!root){
        std::cerr << "Error: Failed to build Huffman tree.\n";
        in.close();
        out.close();
        return false;
    }

    // 准备一块缓存区
    std::vector<uint8_t> readBuffer(BUFF_SIZE);
    std::vector<uint8_t> decompressedData;
    decompressedData.reserve(header.originalSize);

    // 进行解压
    HNode* currentNode = root;
    uint64_t decompressedCount = 0;
    bool isLastByte = false;

    while(in && decompressedCount < header.originalSize){
        // 读取一块数据
        in.read(reinterpret_cast<char*>(readBuffer.data()), static_cast<std::streamsize>(BUFF_SIZE));
        std::streamsize n = in.gcount();
        if(n <= 0)  break;
        
        // 遍历读取的数据
        for(std::streamsize i = 0;i < n && decompressedCount < header.originalSize; i++){
            uint8_t byte = readBuffer[i];
            int bitsToProcess = 8;

            // 检查是不是最后一个字节
            if(i == n - 1 && !in.peek()){
                isLastByte = true;
                bitsToProcess -= header.validBits;
            }

            for(int j = 0;j < bitsToProcess && decompressedCount < header.originalSize; j++){
                // 获取当前位的值
                bool bit = (byte >> (7 - j)) & 1;

                // 沿着huffman树移动
                if(bit){
                    // 1就右边
                    currentNode = currentNode->right;
                }else{
                    currentNode = currentNode->left;
                }
                // 到了叶子节点，输出对应字节
                if(currentNode->isLeaf()){
                    decompressedData.push_back(currentNode->byte);
                    decompressedCount++;
                    currentNode = root; // 重置节点
                }
            }
        }
    }

    // 释放资源
    deleteHuffmanTree(root);

    // 计算校验码
    uint32_t calculatedCRC = 0xFFFFFFFF;
    for(uint8_t byte : decompressedData){
        calculatedCRC = CRC32::update(calculatedCRC, byte);
    }
    calculatedCRC = CRC32::finalize(calculatedCRC);

    // 校验校验码
    if(calculatedCRC != header.crc32){
        std::cerr << "Error: CRC32 checksum mismatch. Decompressed data may be corrupted.\n";
        out.close();
        in.close();
        return false;
    }

    // 将解压后的数据写入目标文件
    out.write(reinterpret_cast<const char*>(decompressedData.data()), decompressedData.size());


    out.close();
    in.close();
    return true;
}


