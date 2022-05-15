#ifndef EYFS_H
#define EYFS_H
#include "BufferManager.h"
#include "DeviceManager.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "SuperBlock.h"
#include "User.h"
#include "Utils.h"

class Eyfs {
private:
  DeviceManager *p_device_manager_;
  BufferManager *p_buffer_manager_;
  OpenfileTable *p_openfile_table_;
  SuperBlock *p_superblock_;
  FileSystem *p_file_system_;
  InodeTable *p_inode_table_;
  FileManager *p_file_manager_;
  User *p_user_;
  bool running_;

public:
  // @param cmd_args：用户输入的参数
  // @brief 根据用户输入的命令和参数执行相关的函数
  // @return void
  void ExecuteCmd(vector<string> cmd_args);
  // @param 
  // @brief 
  // @return 
  void Run();
  Eyfs();
  ~Eyfs();
};

#endif