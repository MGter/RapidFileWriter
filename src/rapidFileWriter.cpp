#include "rapidFileWriter.h"

RapidFileWriter::RapidFileWriter() :
    m_isOpend(false),
    m_filePath(""), 
    m_buffer(nullptr), 
    m_bufferOffset(0), 
    m_bufferLen(BUFFER_LEN), 
    m_allocateOnceSize(FALLOCATE_PACK_SIZE),
    m_fd(-1), 
    m_allocatedSize(0), 
    m_actualSize(0){;}

RapidFileWriter::~RapidFileWriter(){
    closeFile();
}


bool RapidFileWriter::closeFile(){
    // 还有很多事情做
    if(m_buffer){
        // 写入未写入的内容
        if((m_bufferOffset > 0) && (m_fd >= 0)){
            flushBuffer();
        }
        free(m_buffer);
        m_buffer = nullptr;
    }

    if(m_fd >= 0){
        // 重新分配大小
        ftruncate(m_fd, m_actualSize);
        close(m_fd);
        m_fd = -1;
    }

    m_isOpend = false;
    return true;
}

bool RapidFileWriter::openFile(const std::string& filePath){
    if(m_isOpend) return false;

    // 打开文件
    m_filePath = filePath;
    // 不能用append的模式
    // append模式会导致posix_fallocate重分配大小后，write前移动指针位置
    m_fd = open(m_filePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0645);
    if(m_fd < 0){
        return false;
    }

    // 预分配内存空间
    if(posix_fallocate(m_fd, 0, m_allocateOnceSize) != 0){
        close(m_fd);
        return false;
    }
    m_allocatedSize += m_allocateOnceSize;

    // 创建buffer
    m_buffer = (char *)malloc(m_bufferLen);
    memset(m_buffer, 0, m_bufferLen);

    // 启动成功
    m_isOpend = true;
    return true;
}

bool RapidFileWriter::isOpened(){
    return m_isOpend;
}

bool RapidFileWriter::setBufferLen(uint64_t bufferLen){
    if(bufferLen <= 0 || m_isOpend) return false;
    m_bufferLen = bufferLen;
    return true;
}

uint64_t RapidFileWriter::writeFile(const char* payload, uint64_t len) {
    if(!m_isOpend || !payload || (len == 0)){
        return 0;
    }

    std::lock_guard<std::mutex> lock(m_writerMut);
    uint64_t payloadOffset = 0;
    uint64_t totalWriteLen = 0;
    uint64_t toWriteLen = 0;

    while(len > 0){
        toWriteLen = 0;
        // buffer写满，先放入buffer再写入文件
        if(m_bufferOffset + len >= m_bufferLen){
            toWriteLen = m_bufferLen - m_bufferOffset;
            memcpy(m_buffer + m_bufferOffset, payload + payloadOffset, toWriteLen);
            len -= toWriteLen;
            m_bufferOffset += toWriteLen;
            payloadOffset += toWriteLen;
            flushBuffer();
        }
        // buffer够用，复制入buffer
        else{
            toWriteLen = len;
            memcpy(m_buffer + m_bufferOffset, payload + payloadOffset, toWriteLen);
            len -= toWriteLen;
            m_bufferOffset += toWriteLen;
            payloadOffset += toWriteLen;
        }
        totalWriteLen += toWriteLen;
    }
    return totalWriteLen;
}

uint64_t RapidFileWriter::flushBuffer(){
    if(!m_isOpend || !m_buffer || (m_fd < 0) || (m_bufferOffset == 0)){
        return 0;
    }

    uint64_t ans = 0;
    // 大小超过预分配大小，重新分配
    while(m_actualSize + m_bufferOffset >= m_allocatedSize){
        if(posix_fallocate(m_fd, m_allocatedSize, m_allocateOnceSize) != 0){
            return 0;
        }
        m_allocatedSize += m_allocateOnceSize;
    }

    // 将数据写入文件
    if(write(m_fd, (const void*)m_buffer, (size_t)m_bufferOffset) != m_bufferOffset){
        return 0;
    }

    // 更新状态
    ans = m_bufferOffset;
    m_actualSize += m_bufferOffset;
    m_bufferOffset = 0;
    return ans;
}

uint64_t RapidFileWriter::getActualSize(){
    return m_actualSize;
}

uint64_t RapidFileWriter::getAllocatedSize(){
    return m_allocatedSize;
}