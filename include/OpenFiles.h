#ifndef OPENFILES_H
#define OPENFILES_H
#include "File.h"

class OpenFiles {
public:
  // 进程允许打开的最大文件数
  static const int FILESNUM = 100;
  OpenFiles();
  ~OpenFiles();
  // @param:
  // @brief:
  // @ret:
  int AllocFreeSlot();
  // @param:
  // @brief:
  // @ret:
  File *GetFile(int fd);
  // @param: 
  // @brief: 
  // @ret: 
  void SetFile(int fd, File* p_file);
  // 指向系统文件打开表
  File* ProcessOpenFilesTable[FILESNUM];
};

#endif