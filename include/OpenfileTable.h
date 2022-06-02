#ifndef OPENFILETABLE_H
#define OPENFILETABLE_H
#include "File.h"

class OpenfileTable {
public:
  static const int FILENUM = 100;
  // @param 
  // @brief 分配一个文件结构
  // @return 
  File *AllocFile();
  // @param 
  // @brief 关闭一个文件
  // @return 
  void CloseFile(File *p_file);
  // @param 
  // @brief 格式化系统打开文件表
  // @return 
  void FormatOpenFileTable();
  File file_table_[FILENUM];
};

#endif