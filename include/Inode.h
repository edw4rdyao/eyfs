#ifndef INODE_H
#define INODE_H
#include <iostream>
// @class: Inode
// @breif: 内存索引节点，系统每一个文件，目录以及挂载的子系统都对应唯一的
// Inode，Inode分为内存Inode和外存Inode(DiskInode),每一个内存Inode通过
// i_dev_对应一个外存的DiskInode
class Inode {
public:
  enum InodeFlag {
    ILOCK = 0x1,  // 索引节点上锁
    IUPD = 0x2,   // 内存Inode被修改过，更新相应外存Inode
    IACC = 0x4,   // 内存Inode被访问过，修改最近一次访问时间
    IMOUNT = 0x8, // 内存Inode用于挂载子文件系统
    IWANT = 0x10, // 有进程正在等待该内存Inode
    ITEXT = 0x20  // 内存Inode对应进程图像的正文段
  };
  static const unsigned int IALLOC = 0x8000; // 文件被使用
  static const unsigned int IFMT = 0x6000;   // 文件类型掩码
  static const unsigned int IFDIR = 0x4000;  // 目录文件
  static const unsigned int IFCHR = 0x2000;  // 字符设备特殊类型文件
  static const unsigned int IFBLK = 0x6000;  // 块设备特殊类型文件
  static const unsigned int ILARG = 0x1000;  // 大型或巨型文件
  // static const unsigned int ISUID =
  //     0x800; // 执行时文件时将用户的有效用户ID修改为文件所有者uid
  // static const unsigned int ISGID =
  //     0x400; // 执行时文件时将用户的有效组ID修改为文件所有者的Group ID
  static const unsigned int ISVTX = 0x200; // 使用后仍然位于交换区上的正文段
  static const unsigned int IREAD = 0x100; // 对文件的读权限
  static const unsigned int IWRITE = 0x80; // 对文件的写权限
  static const unsigned int IEXEC = 0x40;  // 对文件的执行权限
  static const unsigned int IRWXU =
      (IREAD | IWRITE | IEXEC); // 文件主对文件的读、写、执行权限
  static const unsigned int IRWXG =
      ((IRWXU) >> 3); // 文件主同组用户对文件的读、写、执行权限
  static const unsigned int IRWXO =
      ((IRWXU) >> 6); // 其他用户对文件的读、写、执行权限
  // 物理块的大小
  static const int BLOCK_SIZE = 512;
  // 每一个块双字编址
  static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);
  // 小文件对应的块的大小<= 6*512
  static const int SMALL_FILE_BLOCK = 6;
  // 大文件对应的块的大小<= 128*2*512
  static const int LARGE_FILE_BLOCK = 128 * 2 + 6;
  // 巨型文件对应的块的大小<= 128 * 128 * 2 + 128 * 2 + 6
  static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;

public:
  Inode();
  ~Inode();
  // @param: 无
  // @brief: 根据Inode中对应的磁盘设备索引表，读取数据
  // @ret: void
  void ReadInode();
  // @param: 无
  // @brief: 根据Inode中对应的磁盘设备索引表，读取数据
  // @ret: void
  void WriteInode();
  // @param: 无
  // @brief: 将文件的逻辑块号转化为对应的物理块号
  // @ret: 对应的物理块号
  int BlockMap(int logic_black_id);
  // @param: time 当前时间
  // @brief: 更新外存Inode的最后访问时间和修改时间
  // @ret: void
  void UpdateInode(int time);
  // @param:
  // @brief:
  // @ret:
  void CopyInode();
  // @param:
  // @brief:
  // @ret:
  void CleanInode();
  // @param:
  // @brief:
  // @ret:
  void TruncateInode();
  unsigned int i_flag_; // Inode文件状态的标志位
  unsigned int i_mode_; // 文件的工作方式
  int i_count_;         // 文件的引用计数
  int i_nlink_;         // 文件的勾连数
  short i_dev_;         // 外存Inode的设备号
  short i_id_;          // 外存Inode的块编号
  short i_uid_;         // 文件所有者的用户id
  short i_gid_;         // 文件所有者的用户组id
  int i_size_;          // 文件大小
  int i_addr_[10];      // 用于文件逻辑块转换的基本索引表
  int i_last_read_;     // 存放当前Inode最近一次读取时间
};
#endif