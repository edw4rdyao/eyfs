#ifndef EYFS_H
#define EYFS_H
#include "BufferManager.h"
#include "DeviceManager.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "SuperBlock.h"
#include "User.h"
#include "UserManager.h"
#include "Utils.h"

class Eyfs {
private:
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
  void FormatSystem();
  void LoadSystem();
  void PrintHelp();
  Eyfs();
  ~Eyfs();
};

#endif