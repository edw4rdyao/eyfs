#include "OpenfileTable.h"
#include "InodeTable.h"
#include "User.h"
#include "Utils.h"
#include <cstring>

extern User *p_user;
extern InodeTable *p_inode_table;

File *OpenfileTable::AllocFile() {
  if (DEBUG)
    Print("OpenfileTable Info", "execute fuction AllocFile()");
  int fd = p_user->u_openfiles_.AllocFreeSlot();
  if (fd < 0) {
    Print("OpenfileTable Info", "alloc failed");
    return NULL;
  }
  cout << "[OpenfileTable Info]" << "open file: " << fd << endl; 
  // 寻找空闲项
  for (size_t i = 0; i < OpenfileTable::FILENUM; i++) {
    if (file_table_[i].f_count_ == 0) {
      p_user->u_openfiles_.SetFile(fd, &file_table_[i]);
      file_table_[i].f_count_++;
      file_table_[i].f_offset_ = 0;
      return &file_table_[i];
    }
  }
  p_user->u_error_code_ = User::U_EMFILE;
  return NULL;
}

void OpenfileTable::CloseFile(File *p_file) {
  if (DEBUG)
    Print("OpenfileTable Info", "execute fuction CloseFile(...)");
  p_file->f_count_--;
  if (p_file->f_count_ <= 0) {
    p_inode_table->PutInode(p_file->f_inode_);
  }
  return;
}

void OpenfileTable::FormatOpenFileTable() {
  if (DEBUG)
    Print("OpenfileTable Info", "execute fuction FormatOpenFileTable()");
  File empty_file;
  for (size_t i = 0; i < OpenfileTable::FILENUM; i++) {
    memcpy(file_table_ + i, &empty_file, sizeof(File));
  }
  return;
}