#include "rapidFileWriter.h"
#include <chrono>
#include <iostream>

int main(){
    RapidFileWriter writer;
    const uint64_t bufferSize = 5000; // 5 KB
    const uint64_t totalSize = 1024 * 1024 * 1024; // 1GB
    const uint64_t iterations = totalSize / bufferSize; // 计算迭代次数
    char buffer1[bufferSize];
    char buffer2[bufferSize];
    memset(buffer1, 'A', bufferSize);
    memset(buffer2, 'B', bufferSize);

    std::string fileName = "./test.txt";
    writer.openFile(fileName);

    auto start = std::chrono::high_resolution_clock::now();
    uint64_t totalWriteCnt = 0;

    int times = 5;
    for(int j = 0; j < times; j++){
        for(uint64_t i = 0; i < iterations; i++){
        if(i % 2 == 0)
            writer.writeFile(buffer1, bufferSize);
        else
            writer.writeFile(buffer2, bufferSize);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    writer.closeFile();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken to write " << times << "GB: " << elapsed.count() << " seconds." << std::endl;

    return 0;

}