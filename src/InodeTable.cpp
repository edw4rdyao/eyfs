#include "InodeTable.h"
#include "BufferManager.h"
#include "FileSystem.h"
#include "User.h"
#include "Utils.h"
#include <cstring>

extern User *p_user;
extern BufferManager *p_buffer_manager;
extern FileSystem *p_file_system;

InodeTable::InodeTable() {}

InodeTable::~InodeTable() {}

Inode *InodeTable::GetInode(int id) {
  if (DEBUG) {
    Print("InodeTable Info", "begin get inode");
  }
  Inode *p_inode = NULL;
  // 检查是否有内存Inode拷贝
  int index = IsLoaded(id);
  if (index >= 0) {
    if (DEBUG) {
      cout << "[InodeTable Info] inode is loaded.  "
           << "id:" << id << "  index:" << index << endl;
    }
    p_inode = it_inode_ + index;
    p_inode->i_count_++;
    return p_inode;
  } else {
    // 没有内存Inode，分配一个内存Inode
    p_inode = GetFreeInode();
    if (p_inode == NULL) {
      Print("InodeTable Info", "inode table is full");
      p_user->u_error_code_ = User::U_ENFILE;
      return NULL;
    } else {
      // 分配成功
      if (DEBUG) {
        cout << "[InodeTable Info] inode is not loaded but alloced.  "
             << "id:" << id << "  index:" << index << endl;
      }
      p_inode->i_id_ = id;
      p_inode->i_count_++;
      // 将外存Inode读入缓冲区并写入到内存Inode
      Buffer *p_buffer = p_buffer_manager->ReadBlock(
          FileSystem::INODE_START_ADDR + id / FileSystem::INODE_NUM_PER_BLOCK);
      p_inode->CopyInode(p_buffer, id);
      p_buffer_manager->ReleaseBuffer(p_buffer);
      return p_inode;
    }
  }
  Print("InodeTable Info", "no way");
  return NULL;
}

void InodeTable::PutInode(Inode *p_inode) {
  if (DEBUG) {
    Print("InodeTable Info", "begin put inode");
  }
  if (p_inode->i_count_ == 1) {
    // 当前进程为唯一引用inode的进程，释放inode
    if (p_inode->i_nlink_ <= 0) {
      // 当前inode没有目录，删除释放inode
      p_inode->TruncateInode();
      p_inode->i_mode_ = 0;
      p_file_system->FreeInode(p_inode->i_id_);
    }
    // 更新inode信息
    p_inode->UpdateInode((int)time(NULL));
    // 设置inode标志
    p_inode->i_flag_ = 0;
    p_inode->i_id_ = -1;
  }
  p_inode->i_count_--;
  return;
}

void InodeTable::UpdateInodeTable() {
  if (DEBUG) {
    Print("InodeTable Info", "begin update inode table");
  }
  for (size_t i = 0; i < InodeTable::INODENUM; i++) {
    // 如果Inode对象没有被上锁，即当前未被其它进程使用，可以同步到外存Inode
    // 并且count不等于0，count=0意味着该内存Inode未被任何打开文件引用，无需同步
    if (it_inode_[i].i_count_ != 0) {
      it_inode_[i].UpdateInode((int)time(NULL));
    }
  }
}

int InodeTable::IsLoaded(int id) {
  if (DEBUG) {
    Print("InodeTable Info", "begin judge is loaded inode");
  }
  // 寻找外存Inode是否存在内存Inode拷贝
  for (size_t i = 0; i < InodeTable::INODENUM; i++) {
    if (it_inode_[i].i_id_ == id && it_inode_[i].i_count_ != 0) {
      return i;
    }
  }
  return -1;
}

Inode *InodeTable::GetFreeInode() {
  if (DEBUG) {
    Print("InodeTable Info", "begin get free inode");
  }
  // 如果该内存Inode引用计数为零，则该Inode表示空闲
  for (size_t i = 0; i < InodeTable::INODENUM; i++) {
    if (it_inode_[i].i_count_ == 0) {
      return it_inode_ + i;
    }
  }
  return NULL;
}

void InodeTable::FormatInodeTable() {
  if (DEBUG) {
    Print("InodeTable Info", "begin format inode table");
  }
  Inode empty_inode;
  for (size_t i = 0; i < InodeTable::INODENUM; i++) {
    memcpy(it_inode_ + i, &empty_inode, sizeof(Inode));
  }
  return;
}