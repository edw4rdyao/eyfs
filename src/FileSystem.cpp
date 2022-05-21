#include "FileSystem.h"
#include "BufferManager.h"
#include "DeviceManager.h"
#include "DiskInode.h"
#include "InodeTable.h"
#include "SuperBlock.h"
#include "User.h"
#include "Utils.h"
#include <cstring>
#include <ctime>

extern DeviceManager *p_device_manager;
extern BufferManager *p_buffer_manager;
extern SuperBlock *p_superblock;
extern InodeTable *p_inode_table;
extern User *p_user;

FileSystem::FileSystem() {}

FileSystem::~FileSystem() { Update(); }

void FileSystem::FormatFileSystem() {
  if (DEBUG)
    Print("FileSystem Info", "execute fuction FormatFileSystem()");
  // [SuperBlock区 DiskInode区 数据块区]
  // 首先格式化SuperBlock并写入
  p_superblock->s_isize_ = FileSystem::INODE_ZONE_SIZE;
  p_superblock->s_fsize_ = FileSystem::DISK_SIZE;
  p_superblock->s_nfree_ = 0;
  p_superblock->s_free_[0] = -1;
  p_superblock->s_ninode_ = 0;
  p_superblock->s_flock_ = 0;
  p_superblock->s_ilock_ = 0;
  p_superblock->s_fmod_ = 0;
  p_superblock->s_ronly_ = 0;
  time((time_t *)&p_superblock->s_time_);
  p_device_manager->OpenImage();
  p_device_manager->WriteImage(p_superblock, sizeof(SuperBlock),
                               FileSystem::SUPERBLOCK_START_ADDR);
  // 初始化DiskInode区，root Inode+空闲Inode
  // 新建root根目录
  DiskInode root;
  root.d_mode_ |= (Inode::IALLOC | Inode::IFDIR);
  root.d_nlink_ = 1;
  p_device_manager->WriteImage(&root, sizeof(root));
  // 写入free DiskInode
  DiskInode free;
  for (size_t i = 1; i < FileSystem::INODE_NUM_PER_BLOCK * INODE_ZONE_SIZE;
       i++) {
    // 初始化SuperBlock直接管理的DiskInode
    if (p_superblock->s_ninode_ < 100) {
      p_superblock->s_inode_[p_superblock->s_ninode_++] = i;
    }
    p_device_manager->WriteImage(&free, sizeof(free));
  }
  // 初始化写入数据区
  unsigned char free_block[FileSystem::BLOCK_SIZE];
  unsigned char free_block_manager[FileSystem::BLOCK_SIZE];
  memset(free_block, 0, sizeof(free_block));
  memset(free_block_manager, 0, sizeof(free_block_manager));
  for (size_t i = 0; i < FileSystem::DATA_ZONE_SIZE; i++) {
    if (p_superblock->s_nfree_ < 100) {
      p_device_manager->WriteImage(free_block_manager,
                                   sizeof(free_block_manager));
    } else {
      // 将s_nfree和s_free[100]写入free_block
      // 参考类的数据对齐方式，所以可以直接写sizeof(s_free_)+sizeof(s_nfree_)
      memcpy(free_block, &p_superblock->s_nfree_,
             sizeof(p_superblock->s_free_) + sizeof(p_superblock->s_nfree_));
      p_device_manager->WriteImage(free_block, FileSystem::BLOCK_SIZE);
      p_superblock->s_nfree_ = 0;
    }
    p_superblock->s_free_[p_superblock->s_nfree_++] = i + DATA_ZONE_START_ADDR;
  }
  // 更新SuperBlock修改时间
  time((time_t *)&p_superblock->s_time_);
  p_device_manager->WriteImage(p_superblock, sizeof(SuperBlock),
                               FileSystem::SUPERBLOCK_START_ADDR);
}

void FileSystem::Update() {
  if (DEBUG)
    Print("FileSystem Info", "execute fuction Update()");
  Buffer *p_buffer;
  p_superblock->s_fmod_ = 0;
  p_superblock->s_time_ = (int)time(0);

  for (size_t i = 0; i < 2; i++) {
    int *p = (int *)p_superblock + i * 128;
    p_buffer =
        p_buffer_manager->GetBlock(FileSystem::SUPERBLOCK_START_ADDR + i);
    memcpy(p_buffer->b_addr_, p, FileSystem::BLOCK_SIZE);
    p_buffer_manager->WriteBlock(p_buffer);
  }
  p_inode_table->UpdateInodeTable();
  p_buffer_manager->FlushBlock();
  return;
}

Inode *FileSystem::AllocInode() {
  if (DEBUG)
    Print("FileSystem Info", "execute fuction AllocInode()");
  Buffer *p_buffer;
  Inode *p_inode;
  int inode_id = -1;
  if (p_superblock->s_ninode_ <= 0) {
    for (size_t i = 0; i < p_superblock->s_isize_; i++) {
      // 获取DiskInode缓冲区
      p_buffer = p_buffer_manager->ReadBlock(FileSystem::INODE_START_ADDR + i);
      int *p = (int *)p_buffer->b_addr_;
      for (size_t j = 0; j < FileSystem::INODE_NUM_PER_BLOCK; j++) {
        inode_id++;
        // 直接获取inode的i_mode
        int disk_inode_mode = *(p + j * sizeof(DiskInode) / sizeof(int));
        // 如果mode不为0，则不能记入空闲索引表
        if (disk_inode_mode != 0) {
          continue;
        }
        // 如果外存inode的i_mode==0，此时并不能确定
        // 该inode是空闲的，因为有可能是内存inode没有写到
        // 磁盘上,所以要继续搜索内存inode中是否有相应的项
        if (p_inode_table->IsLoaded(inode_id) == -1) {
          // 如果没有内存Inode拷贝，则可以记入
          p_superblock->s_inode_[p_superblock->s_ninode_++] = inode_id;
          // 直接管理的空闲块已经装满
          if (p_superblock->s_ninode_ >= 100) {
            break;
          }
        }
      }
      // 读完了当前的磁盘块，释放缓存
      p_buffer_manager->ReleaseBuffer(p_buffer);
      // 直接管理的空闲块已经装满
      if (p_superblock->s_ninode_ >= 100) {
        break;
      }
    }
    // 磁盘块上没有搜索到DiskInode
    if (p_superblock->s_ninode_ <= 0) {
      p_user->u_error_code_ = User::U_ENOSPC;
      return NULL;
    }
  }
  inode_id = p_superblock->s_inode_[--p_superblock->s_ninode_];
  p_inode = p_inode_table->GetInode(inode_id);
  if (p_inode == NULL) {
    Print("FileSystem Info", "no free inode");
    return NULL;
  }
  p_inode->CleanInode();
  // 设置SuperBlock被修改标志
  p_superblock->s_fmod_ = 1;
  return p_inode;
}

void FileSystem::FreeInode(int inode_id) {
  if (DEBUG)
    Print("FileSystem Info", "execute fuction FreeInode(...)");
  // 如果超级块直接管理的空闲外存Inode超过100个，
  // 同样让释放的外存Inode散落在磁盘Inode区中
  if (p_superblock->s_ninode_ >= 100) {
    return;
  }
  p_superblock->s_inode_[p_superblock->s_ninode_++] = inode_id;
  // 设置SuperBlock被修改标志
  p_superblock->s_fmod_ = 1;
  return;
}

Buffer *FileSystem::AllocBlock() {
  if (DEBUG) {
    Print("FileSystem Info", "begin alloc block");
  }
  int block_id = 0;
  Buffer *p_buffer;
  // 从直接管理的数据块索引表“栈顶”获取空闲磁盘块编号
  block_id = p_superblock->s_free_[--p_superblock->s_nfree_];
  // 所有的磁盘块分配完毕
  if (block_id <= 0) {
    p_superblock->s_nfree_ = 0;
    p_user->u_error_code_ = User::U_ENOSPC;
    return NULL;
  }
  // 管理的空闲磁盘块已空 新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号,
  // 将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free_[100]中
  if (p_superblock->s_nfree_ <= 0) {
    p_buffer = p_buffer_manager->ReadBlock(block_id);
    // 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节
    int *p = (int *)p_buffer->b_addr_;
    p_superblock->s_nfree_ = *p++;
    memcpy(p_superblock->s_free_, p, sizeof(p_superblock->s_free_));
    // 释放缓存
    p_buffer_manager->ReadBlock(block_id);
  }
  p_buffer = p_buffer_manager->GetBlock(block_id);
  if (p_buffer != NULL) {
    // 清空缓存中的数据
    p_buffer_manager->ClearBuffer(p_buffer);
  }
  p_superblock->s_fmod_ = 1;
  return p_buffer;
}

void FileSystem::FreeBlock(int block_id) {
  if (DEBUG)
    Print("FileSystem Info", "execute fuction FreeBlock(...)");
  Buffer *p_buffer = NULL;
  // SuperBlock中直接管理空闲磁盘块号的栈已满
  if (p_superblock->s_nfree_ >= 100) {
    // 使用当前磁盘块，存放前100个磁盘块的索引
    p_buffer = p_buffer_manager->ReadBlock(block_id);
    // 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节
    int *p = (int *)p_buffer->b_addr_;
    *p++ = p_superblock->s_nfree_;
    memcpy(p, p_superblock->s_free_, sizeof(p_superblock->s_free_));
    p_superblock->s_nfree_ = 0;
    // 将存放空闲盘块索引表的“当前释放盘块”写入磁盘
    p_buffer_manager->WriteBlock(p_buffer);
  }
  p_superblock->s_free_[p_superblock->s_nfree_++] = block_id;
  p_superblock->s_fmod_ = 1;
  return;
}