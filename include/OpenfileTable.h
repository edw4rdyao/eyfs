#ifndef OPENFILETABLE_H
#define OPENFILETABLE_H
#include "File.h"

class OpenfileTable {
public:
  static const int FILENUM = 100;
  OpenfileTable();
  ~OpenfileTable();
  File *AllocFile();
  void CloseFile(File *p_file);
  void FormatOpenFileTable();
  File file_table_[FILENUM];
};

#endif