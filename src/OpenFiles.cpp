#include "OpenFiles.h"
#include "User.h"
#include <cstring>

extern User *p_user;

OpenFiles::OpenFiles() {
  for (size_t i = 0; i < OpenFiles::FILESNUM; i++) {
    process_openfile_table_[i] = NULL;
  }
}

OpenFiles::~OpenFiles() {}

int OpenFiles::AllocFreeSlot() {
  for (size_t i = 0; i < OpenFiles::FILESNUM; i++) {
    if (process_openfile_table_[i] == NULL) {
      // 设置核心栈现场保护区中的EAX寄存器的值，即系统调用返回值
      p_user->u_ar0[User::EAX] = i;
      return i;
    }
  }
  p_user->u_ar0[User::EAX] = -1;
  p_user->u_error_code_ = User::U_EMFILE;
  return -1;
}

File *OpenFiles::GetFile(int fd) {
  File *p_file;
  // 如果打开文件描述符的值超出了范围
  if (fd < 0 || fd >= OpenFiles::FILESNUM) {
    p_user->u_error_code_ = User::U_EBADF;
    return NULL;
  }
  p_file = process_openfile_table_[fd];
  if (p_file == NULL) {
    p_user->u_error_code_ = User::U_EBADF;
    return NULL;
  }
  return p_file;
}

void OpenFiles::SetFile(int fd, File *p_file) {
  // 如果打开文件描述符的值超出了范围
  if (fd < 0 || fd >= OpenFiles::FILESNUM) {
    p_user->u_error_code_ = User::U_EBADF;
    return;
  }
  process_openfile_table_[fd] = p_file;
  return;
}