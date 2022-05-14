#ifndef BUFFER_H
#define BUFFER_H
using namespace std;

// @class: Buffer
// @brief: 缓存控制块Buffer，记录了缓存的使用情况
class Buffer {
public:
  enum BufferFlag {
    B_WRITE = 0x1,   // 写操作，将缓存块中的信息写到磁盘设备
    B_READ = 0x2,    // 读操作，将磁盘数据读取到缓存中
    B_DONE = 0x4,    // 读写操作结束
    B_ERROR = 0x8,   // 读写操作发生错误
    B_BUSY = 0x10,   // 缓存正在使用
    B_WANTED = 0x20, // 有进程正在等待使用该缓存
    B_ASYNC = 0x40,  // 异步IO标志
    B_DELWRT = 0x80, // 延迟写标志
  };

public:
  unsigned int b_flags_;  // !标志位，参考BufferFlag
  Buffer *b_forw_;        //
  Buffer *b_back_;        //
  unsigned int b_wcount_; // 缓存块传送的字节数
  unsigned char *b_addr_; // 指向缓冲区的首地址
  int b_blkno_;           // 磁盘的逻辑块号
  int b_error_;           // IO出错时的信息
  int b_resid_;           // IO出错剩余的字节数
  // @param:
  // @brief:
  // @return: void
  Buffer();
  // @param:
  // @brief:
  // @return: void
  ~Buffer();
};

#endif