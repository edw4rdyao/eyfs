#ifndef INODETABLE_H
#define INODETABLE_H
#include "FileSystem.h"
#include "Inode.h"

class InodeTable {
public:
  static const int INODENUM = 100;
  InodeTable(FileSystem *p_file_system);
  ~InodeTable();
  Inode *GetInode(int id);
  void PutInode(Inode *p_inode);
  void UpdateInodeTable();
  int IsLoaded(int id);
  Inode *GetFreeInode();
  void FormatInodeTable();
  Inode it_inode_[INODENUM];
  FileSystem *p_file_system_;
};

#endif