#include "OpenfileTable.h"
#include "InodeTable.h"
#include "User.h"
#include "Utils.h"
#include <cstring>

extern User *p_user;
extern InodeTable *p_inode_table;

File *OpenfileTable::AllocFile() {
  if (DEBUG) {
    Print("OpenfileTable Info", "begin alloc file");
  }
  int fd = p_user->u_openfiles_.AllocFreeSlot();
  if (fd < 0) {
    Print("OpenfileTable Info", "alloc field");
    return NULL;
  }
  // 寻找空闲项
  for (size_t i = 0; i < OpenfileTable::FILENUM; i++) {
    if (file_table_[i].f_count == 0) {
      p_user->u_openfiles_.SetFile(fd, &file_table_[i]);
      file_table_[i].f_count++;
      file_table_[i].f_offset = 0;
      return &file_table_[i];
    }
  }
  p_user->u_error_code_ = User::U_EMFILE;
  return NULL;
}

void OpenfileTable::CloseFile(File *p_file) {
  if (DEBUG) {
    Print("OpenfileTable Info", "begin close file");
  }
  p_file->f_count--;
  if (p_file->f_count <= 0) {
    p_inode_table->PutInode(p_file->f_inode);
  }
  return;
}

void OpenfileTable::FormatOpenFileTable() {
  if (DEBUG) {
    Print("OpenfileTable Info", "begin format file table");
  }
  File empty_file;
  for (size_t i = 0; i < OpenfileTable::FILENUM; i++) {
    memcpy(file_table_ + i, &empty_file, sizeof(File));
  }
  return;
}