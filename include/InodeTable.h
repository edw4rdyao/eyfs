#ifndef INODETABLE_H
#define INODETABLE_H
#include "FileSystem.h"
#include "Inode.h"

class InodeTable {
public:
  static const int INODENUM = 100;
  InodeTable();
  ~InodeTable();
  // @param 
  // @brief 根据id获取Inode
  // @return 
  Inode *GetInode(int id);
  // @param 
  // @brief 释放Inode
  // @return 
  void PutInode(Inode *p_inode);
  // @param 
  // @brief 更新InodeTable
  // @return 
  void UpdateInodeTable();
  int IsLoaded(int id);
  Inode *GetFreeInode();
  // @param 
  // @brief 格式化InodeTable
  // @return 
  void FormatInodeTable();
  Inode it_inode_[INODENUM];
};

#endif