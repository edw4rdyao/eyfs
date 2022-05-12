#ifndef FILE_H
#define FILE_H
#include "Inode.h"
class File {
public:
  enum FileFlags {
    FREAD = 0x1, // 读请求类型
    FWRITE = 0x2 // 写请求类型
  };
  File();
  ~File();
  unsigned int f_flag; // 对打开文件的读、写操作要求
  int f_count;         // 当前引用该文件控制块的进程数量
  Inode *f_inode;      // 指向打开文件的内存Inode
  int f_offset;        // 文件的读写指针
};

#endif