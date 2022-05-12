#include "BufferManager.h"
#include "DeviceManager.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "SuperBlock.h"
#include "Utils.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

const char *IMAGE_NAME = "MyDisk.img";

// 单例模式，一些全局的变量
DeviceManager kDeviceManager(IMAGE_NAME);
SuperBlock kSuperBlock;
BufferManager kBufferManager;
FileSystem kFileSystem;

// @param cmd_args：用户输入的参数
// @brief 根据用户输入的命令和参数执行相关的函数
// @ret void
void ExecuteCmd(vector<string> cmd_args);

int main() {
  DisplayInfomation();
  while (1) {
    string cmd;
    getline(cin, cmd);
    vector<string> cmd_args = ParseCmd(cmd);
    if (cmd_args.size()) {
      if (DEBUG) {
        PrintLog();
        cout << "[Cmd] ";
        for (auto it = cmd_args.begin(); it != cmd_args.end(); it++) {
          if (it == cmd_args.begin())
            cout << "cmd: " << *it << " ";
          else
            cout << "arg" << (it - cmd_args.begin()) << ":" << *it << " ";
        }
        cout << endl;
      }
      try {
        ExecuteCmd(cmd_args);
      } catch (string err) {
        cout << "[Error]" << err;
      }
    }
  }
  return 0;
}

void ExecuteCmd(vector<string> cmd_args) { return; }