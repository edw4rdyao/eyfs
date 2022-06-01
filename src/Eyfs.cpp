#include "Eyfs.h"
#include <iomanip>
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
    LoadSystem();
    cout << "[Info] filesystem loading successfully" << endl;

  } else {
    cout << "[Info] filesystem image not exist, is creating and formating file "
            "system...\n";
    FormatSystem();
    cout << "[Info] filesystem format sucessfully" << endl;
  }
  running_ = true;
}

Eyfs::~Eyfs() {
  if (DEBUG)
    Print("Info", "delete object");
  delete p_user_manager;
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

void Eyfs::FormatSystem() {
  p_openfile_table->FormatOpenFileTable();
  p_inode_table->FormatInodeTable();
  p_buffer_manager->FormatBlock();
  p_file_system->FormatFileSystem();
  // 获取root根目录
  p_file_manager->root_inode_ = p_inode_table->GetInode(0);
  p_file_manager->root_inode_->i_count_ = 0xff;
  // 打开根目录
  p_user->u_pdir_current_ = p_file_manager->root_inode_;
  p_user->u_pdir_parent_ = NULL;
  // root用户创建基础文件夹
  p_user->Mkdir("bin", "777");
  p_user->Mkdir("etc", "755");
  p_user->Mkdir("home", "777");
  p_user->Mkdir("dev", "777");
  // 创建用户信息文件
  p_user->CheckDirectoryParam("/etc/user");
  p_user->u_args_[1] = p_user->GetInodeMode("644");
  p_file_manager->Create();
  int fd = p_user->u_ar0[User::EAX];
  if (DEBUG)
    cout << "oprnfile fd: " << fd << endl;
  // 写入用户信息
  const char *init_users = "root:root:0\nyzh:011988:1000\n";
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
}

void Eyfs::LoadSystem() {
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
  // p_file_manager->Open();
  p_user->u_pdir_current_ = p_file_manager->root_inode_;
  p_user->u_pdir_parent_ = NULL;
  // 获取当前用户列表文件
  // p_user->Open("etc/user", "-r");
  p_user->CheckDirectoryParam("etc/user");
  p_user->u_args_[1] = p_user->GetFileMode("-r");
  p_file_manager->Open();
  int fd = p_user->u_ar0[User::EAX];
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
}

void Eyfs::ExecuteCmd(vector<string> cmd_args) {
  if (cmd_args[0] == "exit") {
    running_ = false;
  } else if (cmd_args[0] == "clear") {
    system("cls");
  } else if (cmd_args[0] == "format") {
    if (p_user->u_uid_ != 0) {
      Print("Error", "only root can format system");
      return;
    }
    delete p_user;
    p_user = new User;
    FormatSystem();
    cout << "format successfully, please login" << endl;
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
    if ((cmd_args.size() != 4 && cmd_args.size() != 5) ||
        (cmd_args[1] != "-f" && cmd_args[1] != "-c") ||
        (cmd_args[1] == "-f" && cmd_args.size() != 5) ||
        (cmd_args[1] == "-c" && cmd_args.size() != 4)) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    if (cmd_args[1] == "-f") {
      p_user->Read(cmd_args[2], cmd_args[3], cmd_args[4]);
    } else if (cmd_args[1] == "-c") {
      p_user->Read(cmd_args[2], "", cmd_args[3]);
    }
  } else if (cmd_args[0] == "write") {
    if ((cmd_args.size() != 4 && cmd_args.size() != 5) ||
        (cmd_args[1] != "-f" && cmd_args[1] != "-c") ||
        (cmd_args[1] == "-f" && cmd_args.size() != 5) ||
        (cmd_args[1] == "-c" && cmd_args.size() != 4)) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    if (cmd_args[1] == "-f") {
      p_user->Write(cmd_args[2], cmd_args[3], cmd_args[4]);
    } else if (cmd_args[1] == "-c") {
      p_user->Write(cmd_args[2], "", cmd_args[3]);
    }
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
  } else if (cmd_args[0] == "adduser") {
    if (cmd_args.size() != 4) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user_manager->AddUser(cmd_args[1], cmd_args[2], cmd_args[3]);
  } else if (cmd_args[0] == "rmuser") {
    if (cmd_args.size() != 2) {
      Print("Error", "command param is error using 'help' to check");
      return;
    }
    p_user_manager->DeleteUser(cmd_args[1]);
  } else if (cmd_args[0] == "help") {
    PrintHelp();
  } else {
    Print("Error", "command not found");
  }
  return;
}

void Eyfs::PrintHelp() {
  cout << std::left << setw(50) << "command"
       << "description" << endl;
  cout << std::left << setw(50) << "exit"
       << "exit file system and update disk image." << endl;
  cout << std::left << setw(50) << "clear"
       << "clear the output before." << endl;
  cout << std::left << setw(50) << "ls"
       << "list the files and directories on current directory." << endl;
  cout << std::left << setw(50) << "cd [directory path]"
       << "change current directory to [directory path]." << endl;
  cout << std::left << setw(50) << "mkdir [directory path] [mode]"
       << "create a new directory with [mode] such as 755." << endl;
  cout << std::left << setw(50) << "rm [directory path | file path]"
       << "delete file or directory." << endl;
  cout << std::left << setw(50) << "create [file path]"
       << "create a new file, and open it with write-only mode." << endl;
  cout << std::left << setw(50) << "open [file path] [mode]"
       << "open a file with [mode] such as '-r', '-w' or '-rw'" << endl;
  cout << std::left << setw(50) << "seek [file descriptor] [offset] [origin]"
       << "move file pointer to position [origin] + [offset]." << endl;
  cout << std::left << setw(50) << "close [file descriptor]"
       << "close file of [file descriptor] in openfile tables." << endl;
  cout << std::left << setw(50) << "read -c [file descriptor] [size]"
       << "read file of [size] and print on terminal." << endl;
  cout << std::left << setw(50)
       << "read -f [file descriptor] [file name] [size]"
       << "read file of [size] and output in file [file name]." << endl;
  cout << std::left << setw(50) << "write -c [file descriptor] [content]"
       << "write file of [size] by input on terminal." << endl;
  cout << std::left << setw(50)
       << "write -f [file descriptor] [file name] [size]"
       << "write file of [size] by input file [file name]." << endl;
  cout << std::left << setw(50) << "ul"
       << "list the current user of file system." << endl;
  cout << std::left << setw(50) << "login [username] [password]"
       << "login user [username] with [password]." << endl;
  cout << std::left << setw(50) << "logout"
       << "logout the current user." << endl;
  cout << std::left << setw(50) << "adduser [username] [password] [user id]"
       << "[*root only] add a user with [username] [password] and [user id]."
       << endl;
  cout << std::left << setw(50) << "rmuser [username]"
       << "[*root only] delete a user with [username]." << endl;
  cout << std::left << setw(50) << "help"
       << "help information of commonds." << endl;
  cout << endl;
}

void Eyfs::Run() {
  while (running_) {
    if (p_user->u_uid_ == -1) {
      Print("Info", "please use 'login [username] [password]' to login a user");
      cout << "user login: ";
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
        } else if (cmd_args[0] == "help") {
          PrintHelp();
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