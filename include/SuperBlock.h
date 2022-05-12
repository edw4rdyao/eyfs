#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

class SuperBlock {
public:
  int s_isize_;      // 外存Inode区占用的盘块数
  int s_fsize_;      // 盘块总数
  int s_nfree_;      // 直接管理的空闲盘块数量
  int s_free_[100];  // 直接管理的空闲盘块索引表
  int s_ninode_;     // 直接管理的空闲外存Inode数量
  int s_inode_[100]; // 直接管理的空闲外存Inode索引表
  int s_flock_;      // 封锁空闲盘块索引表标志
  int s_ilock_;      // 封锁空闲Inode表标志
  int s_time_;       // 最近一次更新时间
  int s_ronly_;      // 本文件系统只能读出
  int s_fmod_;       // 副本被修改标志
  int padding_[47];  // 填充使SuperBlock块大小等于1024字节
};

#endif