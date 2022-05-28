#ifndef USER_H
#define USER_H
#include "DirectoryEntry.h"
#include "FileManager.h"
#include "Inode.h"
#include "IoParameter.h"
#include "OpenFiles.h"
#include <iostream>

using namespace std;

class User {
public:
  static const int EAX = 0;
  enum ErrorCode {
    U_NOERROR = 0,  // No error
    U_EPERM = 1,    // Operation not permitted
    U_ENOENT = 2,   // No such file or directory
    U_ESRCH = 3,    // No such process
    U_EINTR = 4,    // Interrupted system call
    U_EIO = 5,      // I/O error
    U_ENXIO = 6,    // No such device or address
    U_E2BIG = 7,    // Arg list too long
    U_ENOEXEC = 8,  // Exec format error
    U_EBADF = 9,    // Bad file number
    U_ECHILD = 10,  // No child processes
    U_EAGAIN = 11,  // Try again
    U_ENOMEM = 12,  // Out of memory
    U_EACCES = 13,  // Permission denied
    U_EFAULT = 14,  // Bad address
    U_ENOTBLK = 15, // Block device required
    U_EBUSY = 16,   // Device or resource busy
    U_EEXIST = 17,  // File exists
    U_EXDEV = 18,   // Cross-device link
    U_ENODEV = 19,  // No such device
    U_ENOTDIR = 20, // Not a directory
    U_EISDIR = 21,  // Is a directory
    U_EINVAL = 22,  // Invalid argument
    U_ENFILE = 23,  // File table overflow
    U_EMFILE = 24,  // Too many open files
    U_ENOTTY = 25,  // Not a typewriter(terminal)
    U_ETXTBSY = 26, // Text file busy
    U_EFBIG = 27,   // File too large
    U_ENOSPC = 28,  // No space left on device
    U_ESPIPE = 29,  // Illegal seek
    U_EROFS = 30,   // Read-only file system
    U_EMLINK = 31,  // Too many links
    U_EPIPE = 32,   // Broken pipe
  };

  User();
  ~User();
  void Cd(string dir_name);
  void Mkdir(string dir_name, string mode);
  void Create(string file_name, string mode);
  void Delete(string file_name);
  void Open(string file_name, string mode);
  void Close(string fd);
  void Seek(string fd, string offset, string origin);
  void Write(string fd, string input_file, string size);
  void Read(string fd, string output_file, string size);
  void Ls();
  void UserList();
  bool CheckDirectoryParam(string dir_name);
  void HandleError(enum ErrorCode err_code);
  int GetInodeMode(string mode);
  int GetFileMode(string mode);
  string GetPermission(unsigned int mode);
  unsigned int u_ar0[1024];
  int u_uid_;                  // 用户id
  int u_gid_;                  // 用户组id
  int u_args_[5];              // 系统调用参数
  string u_dir_param_;         // 系统调用参数
  Inode *u_pdir_current_;      // 当前目录的Inode指针
  Inode *u_pdir_parent_;       // 当前目录的父目录的Inode指针
  DirectoryEntry u_dir_entry_; // 当前的目录项
  char u_dir_buffer_[DirectoryEntry::DIRSIZE]; // 当前路径分量
  string u_dir_fact_;                          // 当前目录完整路径
  ErrorCode u_error_code_;                     // 错误码
  OpenFiles u_openfiles_;                      // 当前打开文件对象
  IOParameter u_ioparam;                       // 当前读写描述符
  vector<Inode*> u_list_;
};

#endif