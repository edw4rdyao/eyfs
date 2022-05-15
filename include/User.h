#ifndef USER_H
#define USER_H
#include "DirectoryEntry.h"
#include "FileManager.h"
#include "Inode.h"
#include "IoParameter.h"
#include "OpenFiles.h"
#include <iostream>

using namespace std;

// 声明FileManager类，因为其与User类都含有引用，先后关系导致未定义
class FileManager;

class User {
public:
  enum ErrorCode {
    UNOERROR = 0,  // No error
    UEPERM = 1,    // Operation not permitted
    UENOENT = 2,   // No such file or directory
    UESRCH = 3,    // No such process
    UEINTR = 4,    // Interrupted system call
    UEIO = 5,      // I/O error
    UENXIO = 6,    // No such device or address
    UE2BIG = 7,    // Arg list too long
    UENOEXEC = 8,  // Exec format error
    UEBADF = 9,    // Bad file number
    UECHILD = 10,  // No child processes
    UEAGAIN = 11,  // Try again
    UENOMEM = 12,  // Out of memory
    UEACCES = 13,  // Permission denied
    UEFAULT = 14,  // Bad address
    UENOTBLK = 15, // Block device required
    UEBUSY = 16,   // Device or resource busy
    UEEXIST = 17,  // File exists
    UEXDEV = 18,   // Cross-device link
    UENODEV = 19,  // No such device
    UENOTDIR = 20, // Not a directory
    UEISDIR = 21,  // Is a directory
    UEINVAL = 22,  // Invalid argument
    UENFILE = 23,  // File table overflow
    UEMFILE = 24,  // Too many open files
    UENOTTY = 25,  // Not a typewriter(terminal)
    UETXTBSY = 26, // Text file busy
    UEFBIG = 27,   // File too large
    UENOSPC = 28,  // No space left on device
    UESPIPE = 29,  // Illegal seek
    UEROFS = 30,   // Read-only file system
    UEMLINK = 31,  // Too many links
    UEPIPE = 32,   // Broken pipe
  };

  User(FileManager *p_file_manager);
  ~User();
  void Cd(string dir_name);
  void Mkdir(string dir_name);
  void Create(string file_name, string mode);
  void Delete(string file_name);
  void Open(string file_name, string mode);
  void Close(string fd);
  void Seek(string fd, string offset, string origin);
  void Write(string fd, string input_file, string size);
  void Read(string fd, string output_file, string size);
  void Ls();
  void HandleError(enum ErrorCode err_code);

  int u_args_[5];               // 系统调用参数
  string u_dir_param_;          // 系统调用参数
  Inode *u_dir_current_;        // 当前目录的Inode指针
  Inode *u_dir_parent_;         // 当前目录的父目录的Inode指针
  DirectoryEntry u_dir_entry_;  // 当前的目录项
  string u_dir_fact_;           // 当前目录完整路径
  ErrorCode u_error_code_;      // 错误码
  OpenFiles u_openfiles_;       // 当前打开文件对象
  IOParameter u_ioparam;        // 当前读写描述符
  FileManager *p_file_manager_; // 文件管理的引用
};

#endif