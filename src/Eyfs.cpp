#include "Eyfs.h"

DeviceManager *p_device_manager;
BufferManager *p_buffer_manager;
OpenfileTable *p_openfile_table;
SuperBlock *p_superblock;
FileSystem *p_file_system;
InodeTable *p_inode_table;
FileManager *p_file_manager;
User *p_user;

Eyfs::Eyfs() {
  // 建立文件系统数据结构对象之间的勾连
  p_device_manager = new DeviceManager("MyDisk.img");
  p_buffer_manager = new BufferManager();
  p_openfile_table = new OpenfileTable();
  p_superblock = new SuperBlock();
  p_file_system = new FileSystem();
  p_inode_table = new InodeTable();
  p_file_manager = new FileManager();
  p_user_ = new User();
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
    // 初始化 root根目录
    p_file_manager->root_inode_ = p_inode_table->GetInode(0);
    p_file_manager->root_inode_->i_count_ = 0xff;
    // TODO 读入当前用户列表

  } else {
    cout << "[Info] filesystem image not exist, is creating and formating file "
            "system...\n";
    p_file_system->FormatFileSystem();
    // 初始化 root根目录
    p_file_manager->root_inode_ = p_inode_table->GetInode(0);
    p_file_manager->root_inode_->i_count_ = 0xff;
    // TODO root用户创建基础文件夹

    cout << "[Info] filesystem format sucessfully" << endl;
  }
  running_ = true;
}

Eyfs::~Eyfs() {
  delete p_device_manager;
  delete p_buffer_manager;
  delete p_openfile_table;
  delete p_superblock;
  delete p_file_system;
  delete p_inode_table;
  delete p_file_manager;
  delete p_user;
}

void Eyfs::ExecuteCmd(vector<string> cmd_args) {
  if (cmd_args[0] == "exit") {
    running_ = false;
  } else if (cmd_args[0] == "clear") {
    system("cls");
  }
  return;
}

void Eyfs::Run() {
  while (running_) {
    string cmd;
    getline(cin, cmd);
    vector<string> cmd_args = ParseCmd(cmd);
    if (cmd_args.size()) {
      if (DEBUG) {
        PrintLog();
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
      } catch (string err) {
        cout << "[Error] " << err;
      }
    }
  }
  cout << "[Info] bye, best wishes" << endl;
  return;
}