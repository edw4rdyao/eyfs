#include "FileSystem.h"
#include "BufferManager.h"
#include "DeviceManager.h"
#include "DiskInode.h"
#include "SuperBlock.h"
#include "Utils.h"
#include <cstring>
#include <ctime>

extern DeviceManager *p_device_manager;
extern BufferManager *p_buffer_manager;
extern SuperBlock *p_superblock;

FileSystem::FileSystem() {}

FileSystem::~FileSystem() {
  // TODO: 更新文件系统
}

void FileSystem::FormatFileSystem() {
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
      // *将s_nfree和s_free[100]写入free_block
      // *参考类的数据对齐方式，所以可以直接写sizeof(s_free_)+sizeof(s_nfree_)
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

void FileSystem::Update() {}

Inode *FileSystem::AllocInode() { return NULL; }

void FileSystem::FreeInode(int block_id) {}

Buffer *FileSystem::AllocBlock() { return NULL; }

void FileSystem::FreeBlock(int block_id) {}