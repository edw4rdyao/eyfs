#ifndef DISKINODE_H
#define DISKINODE_H
#include<iostream>
// @class DiskInode
// @breif 外存索引节点，每个文件对应一个内存Inode和外存Inode
// ，用于保存文件信息，每个磁盘块可以存放8个DiskInode
class DiskInode {
public:
  unsigned int d_mode_; // 文件的工作方式见InodeFlag
  int d_nlink_;         // 文件的勾连数
  short d_uid_;         // 文件所有者的用户标识数
  short d_gid_;         // 文件所有者的组标识数
  int d_size_;          // 文件大小，字节为单位
  int d_addr_[10];      // 用于文件逻辑块转换的基本索引表
  int d_acess_time_;    // 最后访问时间
  int d_modify_time_;   // 最后修改时间
  // @param: 无
  // @brief: DiskInode的构造函数，初始化各个属性
  // @return: void
  DiskInode();
  ~DiskInode();
};

#endif
