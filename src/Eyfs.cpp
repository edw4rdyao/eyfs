#include "Eyfs.h"

Eyfs::Eyfs() {
  // 建立文件系统数据结构对象之间的勾连
  p_device_manager_ = new DeviceManager("MyDisk.img");
  p_buffer_manager_ = new BufferManager(p_device_manager_);
  p_openfile_table_ = new OpenfileTable();
  p_superblock_ = new SuperBlock();
  p_file_system_ =
      new FileSystem(p_superblock_, p_buffer_manager_, p_device_manager_);
  p_inode_table_ = new InodeTable(p_file_system_);
  p_file_manager_ =
      new FileManager(p_inode_table_, p_openfile_table_, p_file_system_);
  p_user_ = new User(p_file_manager_);
  p_file_manager_->p_user_ = p_user_;
  // 检查镜像文件是否存在，格式化文件系统或者加载文件系统
  if (p_device_manager_->CheckImage()) {
    cout << "[Info] filesystem loading successfully" << endl;
    // 读入SuperBlock
    p_device_manager_->ReadImage(p_superblock_, sizeof(SuperBlock),
                                 FileSystem::SUPERBLOCK_START_ADDR);
    if (DEBUG) {
      cout << "[Superblock Infomation] ";
      cout << "s_isize:" << p_superblock_->s_isize_ << "  ";
      cout << "s_fsize:" << p_superblock_->s_fsize_ << "  ";
      cout << "s_nfree:" << p_superblock_->s_nfree_ << "  ";
      cout << "s_ninode:" << p_superblock_->s_ninode_ << endl;
    }
    // TODO 读入当前用户列表

  } else {
    cout << "[Info] filesystem image not exist, is creating and formating file "
            "system...\n";
    p_file_system_->FormatFileSystem();
    // TODO root用户创建基础文件夹

    cout << "[Info] filesystem format sucessfully" << endl;
  }
  running_ = true;
}

Eyfs::~Eyfs() {
  delete p_device_manager_;
  delete p_buffer_manager_;
  delete p_openfile_table_;
  delete p_superblock_;
  delete p_file_system_;
  delete p_inode_table_;
  delete p_file_manager_;
  delete p_user_;
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