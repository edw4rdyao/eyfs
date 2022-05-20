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
  Inode *p_inode = NULL;
  unsigned int mode = p_user->u_args_[1];
  // 搜索目录的模式为1，表示创建；若父目录不可写，出错返回
  p_inode = SearchDirectory(FileManager::CREATE);
  // 没有找到相应的Inode
  if (p_inode == NULL) {
    if (p_user->u_error_code_)
      return;
    p_inode = this->MakeInode(mode);
    if (p_inode == NULL)
      return;
    // ?如果所希望的名字不存在，使用参数trf = 2来调用open
    // ?不需要进行权限检查，因为刚刚建立的文件的权限和传入参数mode
    // ?所表示的权限内容是一样的
    OpenCommon(p_inode, File::F_WRITE, 2);
    return;
  }
  // 搜索到已经存在要创建的文件，则清空该文件
  OpenCommon(p_inode, File::F_WRITE, 1);
  p_inode->i_mode_ |= mode;
  return;
}

void FileManager::OpenCommon(Inode *p_inode, int mode, int trans) {
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
  }
}

void FileManager::Close() {}

void FileManager::Seek() {}

void FileManager::Read() {}

void FileManager::Write() {}

void FileManager::ReadWriteCommon(enum File::FileFlags mode) {}

Inode *FileManager::SearchDirectory(enum DirectorySearchMode mode) {
  int entry_offset = 0;
  unsigned int index = 0, nindex = 0;
  Inode *p_inode = p_user->u_pdir_current_;
  Buffer *p_buffer = NULL;
  // 如果该路径是'/'开头的，从根目录开始搜索
  if (p_user->u_dir_param_[0] == '/') {
    nindex = ++index + 1;
    p_inode = root_inode_;
  }
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
    // 将Pathname中当前准备进行匹配的路径分量拷贝到u.u_dbuf[]中，
    // 便于和目录项进行比较
    nindex = p_user->u_dir_param_.find_first_of('/', index);
    memset(p_user->u_dir_buffer_, 0, sizeof(p_user->u_dir_buffer_));
    memcpy(p_user->u_dir_buffer_, p_user->u_dir_param_.data() + index,
           (nindex == (unsigned int)string::npos ? p_user->u_dir_param_.length()
                                                 : nindex) -
               index);
    index = nindex;
    // 内部循环对于路径分量搜索，目录项，使用ioparam记录
    p_user->u_ioparam.io_offset_ = 0;
    p_user->u_ioparam.io_count_ = p_inode->i_size_ / sizeof(DirectoryEntry);
    while (true) {
      // 目录项搜索完毕
      if (p_user->u_ioparam.io_count_ == 0) {
        if (p_buffer != NULL) {
          p_buffer_manager->ReleaseBuffer(p_buffer);
        }
        if (mode == FileManager::CREATE &&
            nindex >= p_user->u_dir_param_.length()) {
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
                  sizeof(DirectoryEntry))) {
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
  return NULL;
}

Inode *FileManager::MakeInode(unsigned int mode) { return NULL; }

void FileManager::WriteDirectory(Inode p_inode) {}

void FileManager::ChangeDirectory() {}

void FileManager::Unlink() {}

void FileManager::List() {}