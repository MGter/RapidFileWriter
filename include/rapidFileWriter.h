#ifndef _FILE_WRITER_H__
#define _FILE_WRITER_H__

#include <string>
#include <mutex>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define PAGE_SIZE   (4UL * 1024)                       // 默认一页4K
#define BUFFER_LEN  (PAGE_SIZE * 1)                    // buffer设置为一页
#define FALLOCATE_PACK_SIZE (512UL * 1024 * 1024)      // 默认预分配512MB，最好是4k的倍数

class RapidFileWriter{
public:
    RapidFileWriter();
    ~RapidFileWriter();

    bool openFile(const std::string& filePath);      // 返回fd，失败返回false
    bool setBufferLen(uint64_t bufferLen);          // 不设置默认是4K，开启文件不再允许设置
    bool isOpened();
    bool closeFile();
    uint64_t getActualSize();               // 获得实际的文件大小
    uint64_t getAllocatedSize();            // 获得申请的文件大小
    uint64_t writeFile(const char* payload, uint64_t len);   // 返回写入成功的量
private:
    uint64_t flushBuffer();

private:
    bool        m_isOpend;
    std::string m_filePath;
    std::mutex  m_writerMut;
    char*       m_buffer;           // buffer
    uint64_t    m_bufferLen;        // buffer大小
    uint64_t    m_bufferOffset;     // buffer当前位置
    uint64_t    m_allocateOnceSize; // 单次申请位置的大小
    uint64_t    m_allocatedSize;    // 当前申请的位置总大小
    uint64_t    m_actualSize;       // 实际使用大小
    int         m_fd;               // 文件fd
};


#endif  // _FILE_WRITER_H__