#ifndef OPENFILES_H
#define OPENFILES_H
#include "File.h"

class OpenFiles {
public:
  static const int FILESNUM = 100; // 用户允许打开的最大文件数
  OpenFiles();
  ~OpenFiles();
  // @param:
  // @brief: 分配一个文件结构
  // @return:
  int AllocFreeSlot();
  // @param: int fd
  // @brief: 通过fd获取文件结构
  // @return:
  File *GetFile(int fd);
  // @param:int fd File *p_file
  // @brief:设置文件结构和fd相勾连
  // @return:
  void SetFile(int fd, File *p_file);
  // 指向系统文件打开表
  File *process_openfile_table_[FILESNUM];
};

#endif