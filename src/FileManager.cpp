#include "FileManager.h"
#include "BufferManager.h"
#include "FileSystem.h"
#include "InodeTable.h"
#include "OpenfileTable.h"
#include "User.h"
#include "Utils.h"
#include <cstring>

extern FileSystem *p_file_system;
extern OpenfileTable *p_openfile_table;
extern InodeTable *p_inode_table;
extern User *p_user;
extern BufferManager *p_buffer_manager;

FileManager::FileManager() {}

FileManager::~FileManager() {}

void FileManager::Open() {
  if (DEBUG)
    Print("FileManager Info", "execute fuction Open()");
  Inode *p_inode = NULL;
  p_inode = SearchDirectory(FileManager::OPEN);
  if (p_inode == NULL) {
    Print("FileManager Info", "search inode failed");
    return;
  }
  OpenCommon(p_inode, p_user->u_args_[1], 0);
  return;
}

void FileManager::Create() {
  if (DEBUG)
    Print("FileManager Info", "execute fuction Create()");
  Inode *p_inode = NULL;
  unsigned int mode =
      p_user->u_args_[1] & (Inode::IRWXU | Inode::IRWXG | Inode::IRWXO);
  // 搜索目录的模式为1，表示创建；若父目录不可写，出错返回
  p_inode = SearchDirectory(FileManager::CREATE);
  // 没有找到相应的Inode
  if (p_inode == NULL) {
    if (p_user->u_error_code_)
      return;
    p_inode = this->MakeInode(mode);
    if (p_inode == NULL)
      return;
    // 如果所希望的名字不存在，使用参数trans = 2来调用open
    OpenCommon(p_inode, File::F_WRITE, 2);
    return;
  }
  // 搜索到已经存在要创建的文件，则清空该文件
  OpenCommon(p_inode, File::F_WRITE, 1);
  p_inode->i_mode_ |= mode;
  return;
}

void FileManager::MakeDirectory() {
  if (DEBUG)
    Print("FileManager Info", "execute fuction MakeDirectory()");
  Inode *p_inode = NULL;
  p_inode = SearchDirectory(FileManager::CREATE);
  // 要创建的文件已经存在,这里并不能去覆盖此文件
  if (p_inode != NULL) {
    p_user->u_error_code_ = User::U_EEXIST;
    p_inode_table->PutInode(p_inode);
    return;
  }
  if (p_user->u_error_code_) {
    return;
  }
  p_inode = MakeInode(p_user->u_args_[1]);
  if (p_inode == NULL) {
    return;
  }
  p_inode_table->PutInode(p_inode);
  return;
}

void FileManager::OpenCommon(Inode *p_inode, int mode, int trans) {
  if (DEBUG)
    Print("FileManager Info", "execute fuction OpenCommon(...)");
  // 对所希望的文件已存在的情况下，即trans == 0或trans == 1进行权限检查
  if (trans != 2) {
    if (mode & File::F_READ) {
      CheckAccess(p_inode, Inode::IREAD);
    }
    if (mode & File::F_WRITE) {
      CheckAccess(p_inode, Inode::IWRITE);
    }
  }
  // 权限错误，释放Inode
  if (p_user->u_error_code_) {
    p_inode_table->PutInode(p_inode);
    return;
  }
  // 在creat文件的时候搜索到同文件名的文件，释放该文件所占据的所有盘块
  if (trans == 1) {
    p_inode->TruncateInode();
  }
  // 分配打开文件控制块File结构
  File *p_file = p_openfile_table->AllocFile();
  if (p_file == NULL) {
    p_inode_table->PutInode(p_inode);
    return;
  }
  // 设置打开文件方式，建立File结构和内存Inode的勾连关系
  p_file->f_flag_ = mode & (File::F_READ | File::F_WRITE);
  p_file->f_inode_ = p_inode;
  if (!p_user->u_error_code_) {
    return;
  } else {
    // 出错则释放资源
    int fd = p_user->u_ar0[User::EAX];
    if (fd != -1) {
      p_user->u_openfiles_.SetFile(fd, NULL);
      p_file->f_count_--;
    }
    p_inode_table->PutInode(p_inode);
    return;
  }
  return;
}

void FileManager::Close() {
  if (DEBUG)
    Print("FileManager Info", "execute fuction Close()");
  int fd = p_user->u_args_[0];
  File *p_file = p_user->u_openfiles_.GetFile(fd);
  if (p_file == NULL) {
    Print("FileManager Info", "get file failed");
    return;
  }
  // 释放打开文件描述符fd，递减File结构引用计数
  p_user->u_openfiles_.SetFile(fd, NULL);
  p_openfile_table->CloseFile(p_file);
  return;
}

void FileManager::Seek() {
  if (DEBUG)
    Print("FileManager Info", "execute fuction Seek()");
  File *p_file = NULL;
  int fd = p_user->u_args_[0];
  p_file = p_user->u_openfiles_.GetFile(fd);
  if (p_file == NULL) {
    Print("FileManager Info", "get file failed");
    return;
  }
  int offset = p_user->u_args_[1];
  // origin为0从文件头开始，origin为1从当前开始，origin为2从文件尾开始
  switch (p_user->u_args_[2]) {
  case 0:
    p_file->f_offset_ = offset;
    break;
  case 1:
    p_file->f_offset_ += offset;
    break;
  case 2:
    p_file->f_offset_ = p_file->f_inode_->i_size_ + offset;
    break;
  default:
    Print("FileManager Info", "seek file param error");
    break;
  }
  return;
}

void FileManager::Read() { ReadWriteCommon(File::F_READ); }

void FileManager::Write() { ReadWriteCommon(File::F_WRITE); }

void FileManager::ReadWriteCommon(enum File::FileFlags mode) {
  if (DEBUG)
    Print("FileManager Info", "execute fuction ReadWriteCommon(...)");
  // 根据Read()/Write()的系统调用参数fd获取打开文件控制块结构
  File *p_file;
  p_file = p_user->u_openfiles_.GetFile(p_user->u_args_[0]);
  if (p_file == NULL) {
    Print("FileManager Info", "get file failed");
    return;
  }
  // 读写的模式不正确
  if ((p_file->f_flag_ & mode) == 0) {
    p_user->u_error_code_ = User::U_EACCES;
    return;
  }
  // 设置读取参数
  p_user->u_ioparam.io_start_addr_ = (unsigned char *)p_user->u_args_[1];
  p_user->u_ioparam.io_count_ = p_user->u_args_[2];
  p_user->u_ioparam.io_offset_ = p_file->f_offset_;
  if (mode == File::F_READ) {
    p_file->f_inode_->ReadInode();
  } else {
    p_file->f_inode_->WriteInode();
  }
  // 根据读写字数，移动文件读写偏移指针
  p_file->f_offset_ += (p_user->u_args_[2] - p_user->u_ioparam.io_count_);
  // 返回实际读写的字节数，修改存放系统调用返回值的核心栈单元
  p_user->u_ar0[User::EAX] = p_user->u_args_[2] - p_user->u_ioparam.io_count_;
  return;
}

Inode *FileManager::SearchDirectory(enum DirectorySearchMode mode) {
  if (DEBUG)
    Print("FileManager Info", "execute fuction SearchDirectory(...)");
  int entry_offset = 0;
  unsigned int index = 0, nindex = 0;
  Inode *p_inode = p_user->u_pdir_current_;
  Buffer *p_buffer = NULL;
  // 如果该路径是'/'开头的，从根目录开始搜索
  if (p_user->u_dir_param_[0] == '/') {
    nindex = ++index + 1;
    p_inode = root_inode_;
  }
  // TODO 如果试图更改和删除当前目录文件则出错
  // 外层循环每次处理pathname中一段路径分量
  while (true) {
    if (p_user->u_error_code_) {
      break;
    }
    // 整个路径搜索完毕，返回相应Inode指针。目录搜索成功返回
    if (nindex >= p_user->u_dir_param_.length()) {
      return p_inode;
    }
    // 如果要进行搜索的不是目录文件，释放相关Inode资源则退出
    if ((p_inode->i_mode_ & Inode::IFMT) != Inode::IFDIR) {
      p_user->u_error_code_ = User::U_ENOTDIR;
    }
    // 进行目录搜索权限检查,IEXEC在目录文件中表示搜索权限
    if (CheckAccess(p_inode, Inode::IEXEC)) {
      p_user->u_error_code_ = User::U_EACCES;
      break;
    }

    // 将pathname中当前准备进行匹配的路径分量拷贝便于和目录项进行比较
    nindex = p_user->u_dir_param_.find_first_of('/', index);
    memset(p_user->u_dir_buffer_, 0, sizeof(p_user->u_dir_buffer_));
    memcpy(p_user->u_dir_buffer_, p_user->u_dir_param_.data() + index,
           (nindex == (unsigned int)string::npos ? p_user->u_dir_param_.length()
                                                 : nindex) -
               index);
    index = nindex + 1;
    // 内部循环对于路径分量搜索，目录项，使用ioparam记录
    p_user->u_ioparam.io_offset_ = 0;
    p_user->u_ioparam.io_count_ = p_inode->i_size_ / sizeof(DirectoryEntry);
    while (true) {
      // 目录项搜索完毕
      if (p_user->u_ioparam.io_count_ == 0) {
        if (p_buffer != NULL) {
          p_buffer_manager->ReleaseBuffer(p_buffer);
        }
        // 如果是创建新文件
        if (mode == FileManager::CREATE &&
            nindex >= p_user->u_dir_param_.length()) {
          // 判断该目录是否可写
          if (CheckAccess(p_inode, Inode::IWRITE)) {
            p_user->u_error_code_ = User::U_EACCES;
            p_inode_table->PutInode(p_inode);
            return NULL;
          }
          // 将父目录Inode指针保存起来，以后写目录项会用到
          p_user->u_pdir_parent_ = p_inode;
          if (entry_offset) {
            p_user->u_ioparam.io_offset_ =
                entry_offset - sizeof(DirectoryEntry);
          } else {
            p_inode->i_flag_ |= Inode::I_UPD;
          }
          return NULL;
        }
        p_user->u_error_code_ = User::U_ENOENT;
        p_inode_table->PutInode(p_inode);
        return NULL;
      }
      // 已读完目录文件的当前盘块，需要读入下一目录项数据盘块
      if ((p_user->u_ioparam.io_offset_ % Inode::BLOCK_SIZE) == 0) {
        if (p_buffer) {
          p_buffer_manager->ReleaseBuffer(p_buffer);
        }
        // 计算要读取的盘块
        int block_id =
            p_inode->MapBlock(p_user->u_ioparam.io_offset_ / Inode::BLOCK_SIZE);
        p_buffer = p_buffer_manager->ReadBlock(block_id);
      }
      // 没有读完当前目录项盘块，则读取下一目录项
      memcpy(&p_user->u_dir_entry_,
             p_buffer->b_addr_ +
                 (p_user->u_ioparam.io_offset_ % Inode::BLOCK_SIZE),
             sizeof(DirectoryEntry));
      p_user->u_ioparam.io_offset_ += sizeof(DirectoryEntry);
      p_user->u_ioparam.io_count_--;
      // 如果是空闲目录项，记录该项位于目录文件中偏移量
      if (p_user->u_dir_entry_.inode_id_ == 0) {
        if (entry_offset == 0) {
          entry_offset = p_user->u_ioparam.io_offset_;
        }
        continue;
      }
      if (!memcmp(p_user->u_dir_buffer_, &p_user->u_dir_entry_.name_,
                  sizeof(DirectoryEntry) - 4)) {
        break;
      }
    }
    // 从内层目录项匹配循环跳至此处，说明pathname中
    // 当前路径分量匹配成功了，还需匹配pathname中下一路径
    // 分量，直至遇到'\0'结束
    if (p_buffer) {
      p_buffer_manager->ReleaseBuffer(p_buffer);
    }
    // 如果是删除操作，则返回父目录Inode
    if (mode == FileManager::DELETE &&
        nindex >= p_user->u_dir_param_.length()) {
      if (CheckAccess(p_inode, Inode::IWRITE)) {
        p_user->u_error_code_ = User::U_EACCES;
        break;
      }
      return p_inode;
    }
    // 匹配目录项成功，则释放当前目录Inode，根据匹配成功的目录项inode_id_
    // 获取相应下一级目录或文件的Inode
    p_inode_table->PutInode(p_inode);
    p_inode = p_inode_table->GetInode(p_user->u_dir_entry_.inode_id_);
    if (p_inode == NULL) {
      Print("FileManager Info", "get inode failed");
      return NULL;
    }
  }
  p_inode_table->PutInode(p_inode);
  return NULL;
}

Inode *FileManager::MakeInode(unsigned int mode) {
  if (DEBUG)
    Print("FileManager Info", "execute fuction MakeInode(...)");
  Inode *p_inode;
  // 分配一个空闲DiskInode，里面内容已全部清空
  p_inode = p_file_system->AllocInode();
  if (p_inode == NULL) {
    Print("FileManager Info", "alloc inode failed");
    return NULL;
  }
  p_inode->i_flag_ |= (Inode::I_ACC | Inode::I_UPD);
  p_inode->i_mode_ = mode | Inode::IALLOC;
  p_inode->i_nlink_ = 1;
  p_inode->i_uid_ = p_user->u_uid_;
  p_inode->i_gid_ = p_user->u_gid_;
  // 将目录项写入user，随后写入目录文件
  WriteDirectory(p_inode);
  return p_inode;
}

void FileManager::WriteDirectory(Inode *p_inode) {
  if (DEBUG)
    Print("FileManager Info", "execute fuction WriteDirectory(...)");
  // 设置目录项信息
  p_user->u_dir_entry_.inode_id_ = p_inode->i_id_;
  memcpy(p_user->u_dir_entry_.name_, p_user->u_dir_buffer_,
         DirectoryEntry::DIRSIZE);
  p_user->u_ioparam.io_count_ = DirectoryEntry::DIRSIZE + 4;
  p_user->u_ioparam.io_start_addr_ = (unsigned char *)&p_user->u_dir_entry_;
  // 将目录项写入父目录文件
  p_user->u_pdir_parent_->WriteInode();
  p_inode_table->PutInode(p_user->u_pdir_parent_);
  return;
}

void FileManager::ChangeDirectory() {
  if (DEBUG)
    Print("FileManager Info", "execute fuction ChangeDirectory()");
  Inode *p_inode = NULL;
  p_inode = SearchDirectory(FileManager::OPEN);
  if (p_inode == NULL) {
    Print("FileManager Info", "search directory failed");
    return;
  }
  // 搜索到的文件不是目录文件
  if ((p_inode->i_mode_ & Inode::IFMT) != Inode::IFDIR) {
    p_user->u_error_code_ = User::U_ENOTDIR;
    p_inode_table->PutInode(p_inode);
    return;
  }
  p_user->u_pdir_current_ = p_inode;
  // 路径不是从根目录'/'开始，则在现有完整路径后面加上当前路径分量
  if (p_user->u_dir_param_[0] != '/') {
    p_user->u_dir_fact_ += p_user->u_dir_param_;
  } else {
    p_user->u_dir_fact_ = p_user->u_dir_param_;
  }
  if (p_user->u_dir_fact_.back() != '/') {
    p_user->u_dir_fact_.push_back('/');
  }
  return;
}

void FileManager::Unlink() {
  if (DEBUG)
    Print("FileManager Info", "execute fuction Unlink()");
  Inode *p_inode = NULL;
  Inode *p_inode_delete = NULL;
  p_inode_delete = SearchDirectory(FileManager::DELETE);
  if (p_inode_delete == NULL) {
    return;
  }
  p_inode = p_inode_table->GetInode(p_user->u_dir_entry_.inode_id_);
  if (p_inode == NULL) {
    Print("FileManager Info", "get inode failed");
    return;
  }
  // 写入清零后的目录项
  p_user->u_ioparam.io_offset_ -= (DirectoryEntry::DIRSIZE + 4);
  p_user->u_ioparam.io_start_addr_ = (unsigned char *)&p_user->u_dir_entry_;
  p_user->u_ioparam.io_count_ = DirectoryEntry::DIRSIZE + 4;
  p_user->u_dir_entry_.inode_id_ = 0;
  p_inode_delete->WriteInode();
  // 修改Inode
  p_inode->i_nlink_--;
  p_inode->i_flag_ |= Inode::I_UPD;
  p_inode_table->PutInode(p_inode_delete);
  p_inode_table->PutInode(p_inode);
  return;
}

vector<string> FileManager::List() {
  if (DEBUG)
    Print("FileManager Info", "execute fuction List()");
  vector<string> name_list;
  Inode *p_inode = p_user->u_pdir_current_;
  Buffer *p_buffer = NULL;
  p_user->u_ioparam.io_offset_ = 0;
  p_user->u_ioparam.io_count_ = p_inode->i_size_ / sizeof(DirectoryEntry);
  while (p_user->u_ioparam.io_count_) {
    // 正好读完一个缓存块
    if ((p_user->u_ioparam.io_offset_ % Inode::BLOCK_SIZE) == 0) {
      if (p_buffer) {
        p_buffer_manager->ReleaseBuffer(p_buffer);
      }
      int block_id =
          p_inode->MapBlock(p_user->u_ioparam.io_offset_ / Inode::BLOCK_SIZE);
      p_buffer = p_buffer_manager->ReadBlock(block_id);
    }
    memcpy(&p_user->u_dir_entry_,
           p_buffer->b_addr_ +
               (p_user->u_ioparam.io_offset_ % Inode::BLOCK_SIZE),
           sizeof(DirectoryEntry));
    p_user->u_ioparam.io_offset_ += sizeof(DirectoryEntry);
    p_user->u_ioparam.io_count_--;
    // 遇到空目录项
    if (p_user->u_dir_entry_.inode_id_ == 0) {
      continue;
    }
    if (DEBUG) {
      cout << "[FileManager Info] "
           << "search inode id:" << p_user->u_dir_entry_.inode_id_ << endl;
    }
    // 获取目录文件中文件对应的Inode
    Inode *p_tmp_inode =
        p_inode_table->GetInode(p_user->u_dir_entry_.inode_id_);
    if (DEBUG) {
      cout << "[FileManager Info] "
           << "inode uid:" << p_tmp_inode->i_uid_
           << "  inode gid:" << p_tmp_inode->i_gid_
           << "  inode mode:" << p_tmp_inode->i_mode_
           << "  inode nlink:" << p_tmp_inode->i_nlink_ << endl;
    }
    // 处理输出
    name_list.push_back(string(p_user->u_dir_entry_.name_));
    p_user->u_list_.push_back(p_tmp_inode);
  }
  if (p_buffer) {
    p_buffer_manager->ReleaseBuffer(p_buffer);
  }
  return name_list;
}

int FileManager::CheckAccess(Inode *p_inode, unsigned int mode) {
  // 对于超级用户，读写任何文件都是允许的
  // 而要执行某文件时，必须在i_mode有可执行标志
  if (p_user->u_uid_ == 0) {
    if (mode == Inode::IEXEC &&
        (p_inode->i_mode_ &
         (Inode::IEXEC | (Inode::IEXEC >> 3) | (Inode::IEXEC >> 6))) == 0) {
      p_user->u_error_code_ = User::U_EACCES;
      return 1;
    }
    return 0;
  }
  if (p_user->u_uid_ != p_inode->i_uid_) {
    mode = mode >> 3;
    if (p_user->u_gid_ != p_inode->i_gid_) {
      mode = mode >> 3;
    }
  }
  if ((p_inode->i_mode_ & mode) != 0) {
    return 0;
  }
  p_user->u_error_code_ = User::U_EACCES;
  return 1;
}