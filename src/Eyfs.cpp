#include "Eyfs.h"
#include <string.h>
DeviceManager *p_device_manager;
BufferManager *p_buffer_manager;
OpenfileTable *p_openfile_table;
SuperBlock *p_superblock;
FileSystem *p_file_system;
InodeTable *p_inode_table;
FileManager *p_file_manager;
User *p_user;
UserManager *p_user_manager;

Eyfs::Eyfs() {
  // 建立文件系统数据结构对象之间的勾连
  p_device_manager = new DeviceManager("MyDisk.img");
  p_buffer_manager = new BufferManager();
  p_openfile_table = new OpenfileTable();
  p_superblock = new SuperBlock();
  p_file_system = new FileSystem();
  p_inode_table = new InodeTable();
  p_file_manager = new FileManager();
  p_user = new User();
  p_user_manager = new UserManager();
  // 检查镜像文件是否存在，格式化文件系统或者加载文件系统
  if (p_device_manager->CheckImage()) {
    cout << "[Info] filesystem loading successfully" << endl;
    // 读入SuperBlock
    p_device_manager->ReadImage(p_superblock, sizeof(SuperBlock),
                                FileSystem::SUPERBLOCK_START_ADDR);
    if (DEBUG) {
      cout << "[Superblock Info] ";
      cout << "s_isize:" << p_superblock->s_isize_ << "  ";
      cout << "s_fsize:" << p_superblock->s_fsize_ << "  ";
      cout << "s_nfree:" << p_superblock->s_nfree_ << "  ";
      cout << "s_ninode:" << p_superblock->s_ninode_ << endl;
    }
    // 获取root根目录
    p_file_manager->root_inode_ = p_inode_table->GetInode(0);
    p_file_manager->root_inode_->i_count_ = 0xff;
    // 打开根目录
    p_file_manager->Open();
    p_user->u_pdir_current_ = p_file_manager->root_inode_;
    p_user->u_pdir_parent_ = NULL;
    // 获取当前用户列表文件
    p_user->Open("etc/user", "-r");
    int fd = p_user->u_ar0[User::EAX];
    cout << "fd: " << fd << endl;
    int size = p_user->u_openfiles_.GetFile(fd)->f_inode_->i_size_;
    // 读取文件
    char *user_list = new char[size];
    p_user->u_args_[0] = fd;
    p_user->u_args_[1] = (long)user_list;
    p_user->u_args_[2] = size;
    p_file_manager->Read();
    // 载入用户信息
    p_user_manager->LoadUser(user_list);
    delete[] user_list;
    // 关闭文件
    p_user->u_args_[0] = fd;
    p_file_manager->Close();
    // root登出
    p_user_manager->Logout();

  } else {
    cout << "[Info] filesystem image not exist, is creating and formating file "
            "system...\n";
    p_file_system->FormatFileSystem();
    // 获取root根目录
    p_file_manager->root_inode_ = p_inode_table->GetInode(0);
    p_file_manager->root_inode_->i_count_ = 0xff;
    // 打开根目录
    p_file_manager->Open();
    p_user->u_pdir_current_ = p_file_manager->root_inode_;
    p_user->u_pdir_parent_ = NULL;
    // root用户创建基础文件夹
    p_user->Mkdir("bin", "755");
    p_user->Mkdir("etc", "755");
    p_user->Mkdir("home", "755");
    p_user->Mkdir("dev", "755");
    // 创建用户信息文件
    p_user->Cd("etc");
    p_user->Create("user", "644");
    int fd = p_user->u_ar0[User::EAX];
    cout << "fd: " << fd << endl;
    // 写入用户信息
    const char *init_users = "root:root:0\nyzh:011988:1\n";
    p_user->u_args_[0] = fd;
    p_user->u_args_[1] = (long)init_users;
    p_user->u_args_[2] = strlen(init_users);
    p_file_manager->Write();
    // 初始化用户信息
    p_user_manager->LoadUser(init_users);
    // 关闭文件
    p_user->u_args_[0] = fd;
    p_file_manager->Close();
    // 返回根目录
    p_user->Cd("/");
    // root登出
    p_user_manager->Logout();
    cout << "[Info] filesystem format sucessfully" << endl;
  }
  running_ = true;
}

Eyfs::~Eyfs() {
  if (DEBUG)
    Print("Info", "delete object");
  delete p_file_system;
  delete p_inode_table;
  delete p_file_manager;
  delete p_openfile_table;
  delete p_user;
  delete p_superblock;
  delete p_buffer_manager;
  delete p_device_manager;
  if (DEBUG)
    Print("Info", "delete object over");
}

void Eyfs::ExecuteCmd(vector<string> cmd_args) {
  if (cmd_args[0] == "exit") {
    running_ = false;
  } else if (cmd_args[0] == "clear") {
    system("cls");
  } else if (cmd_args[0] == "ls") {
    p_user->Ls();
  } else if (cmd_args[0] == "cd") {
    if (cmd_args.size() != 2) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Cd(cmd_args[1]);
  } else if (cmd_args[0] == "mkdir") {
    if (cmd_args.size() != 3) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Mkdir(cmd_args[1], cmd_args[2]);
  } else if (cmd_args[0] == "rm") {
    if (cmd_args.size() != 2) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Delete(cmd_args[1]);
  } else if (cmd_args[0] == "create") {
    string mode = "";
    if (cmd_args.size() == 3) {
      mode = cmd_args[2];
    } else if (cmd_args.size() > 3 || cmd_args.size() < 2) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Create(cmd_args[1], mode);
  } else if (cmd_args[0] == "open") {
    if (cmd_args.size() != 3) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Open(cmd_args[1], cmd_args[2]);
  } else if (cmd_args[0] == "seek") {
    if (cmd_args.size() != 4) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Seek(cmd_args[1], cmd_args[2], cmd_args[3]);
  } else if (cmd_args[0] == "close") {
    if (cmd_args.size() != 2) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Close(cmd_args[1]);
  } else if (cmd_args[0] == "read") {
    if (cmd_args.size() != 4) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Read(cmd_args[1], cmd_args[2], cmd_args[3]);
  } else if (cmd_args[0] == "write") {
    if (cmd_args.size() != 4) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->Write(cmd_args[1], cmd_args[2], cmd_args[3]);
  } else if (cmd_args[0] == "ul") {
    if (cmd_args.size() != 1) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user->UserList();
  } else if (cmd_args[0] == "logout") {
    if (cmd_args.size() != 1) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user_manager->Logout();
  } else {
    Print("Error", "command not found");
  }
  return;
}

void Eyfs::Run() {
  while (running_) {
    if (p_user->u_uid_ == -1) {
      Print("Info", "please use 'login' to login a user");
      string cmd;
      getline(cin, cmd);
      vector<string> cmd_args = ParseCmd(cmd);
      if (cmd_args.size()) {
        if (DEBUG) {
          cout << "[Command] ";
          for (auto it = cmd_args.begin(); it != cmd_args.end(); it++) {
            if (it == cmd_args.begin())
              cout << "cmd: " << *it << " ";
            else
              cout << "arg" << (it - cmd_args.begin()) << ":" << *it << " ";
          }
          cout << endl;
        }
        if (cmd_args[0] == "login") {
          if (cmd_args.size() != 3) {
            Print("Error", "command param is error using 'help' to check");
          } else {
            p_user_manager->Login(cmd_args[1], cmd_args[2]);
          }
        } else if (cmd_args[0] == "exit") {
          running_ = false;
        }
      }
    } else {
      cout
          << "["
          << p_user_manager->users_info_[p_user_manager->current_user_].username
          << "@localhost " << p_user->u_dir_fact_ << "]$ ";
      string cmd;
      getline(cin, cmd);
      vector<string> cmd_args = ParseCmd(cmd);
      if (cmd_args.size()) {
        if (DEBUG) {
          cout << "[Command] ";
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
        } catch (const char *err) {
          cout << "[Error] " << err << endl;
        }
      }
    }
  }
  cout << "[Info] bye, best wishes" << endl;
  return;
}