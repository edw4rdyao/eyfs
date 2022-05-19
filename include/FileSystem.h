#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "BufferManager.h"
#include "Inode.h"

// @class FileSystem
// @breif 文件系统类，管理文件系统中的文件，负责外存Inode分配
// 释放，负责SuperBlock的管理等等
class FileSystem {
public:
  // 磁盘设备的大小 8M
  static const int DISK_SIZE = 16384;
  // 每个物理块的字节数
  static const int BLOCK_SIZE = 512;
  // SuperBlock区的起始地址和大小
  static const int SUPERBLOCK_START_ADDR = 0;
  static const int SUPERBLOCK_ZONE_SIZE = 2;
  // Inode区的起始地址和大小
  static const int ROOT_INODE_ID = 0;
  static const int INODE_START_ADDR = 2;
  static const int INODE_ZONE_SIZE = 1024 - 2;
  static const int INODE_NUM_PER_BLOCK = 8;
  // 数据区的起始地址和大小
  static const int DATA_ZONE_START_ADDR = 1024;
  static const int DATA_ZONE_END_ADDR = 16384 - 1;
  static const int DATA_ZONE_SIZE = DISK_SIZE - DATA_ZONE_START_ADDR;
  FileSystem();
  ~FileSystem();
  void FormatFileSystem();
  void Update();
  Inode *AllocInode();
  void FreeInode(int inode_id);
  Buffer *AllocBlock();
  void FreeBlock(int block_id);
};

#endif