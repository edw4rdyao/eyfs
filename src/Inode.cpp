#include "Inode.h"
#include "BufferManager.h"
#include "DiskInode.h"
#include "FileSystem.h"
#include "User.h"
#include "Utils.h"
#include <cstring>
#include <fstream>

extern BufferManager *p_buffer_manager;
extern User *p_user;
extern FileSystem *p_file_system;

Inode::Inode() {
  // 初始化属性，初始化为无效值
  i_flag_ = 0;
  i_mode_ = 0;
  i_count_ = 0;
  i_nlink_ = 0;
  i_dev_ = -1;
  i_id_ = -1;
  i_uid_ = -1;
  i_gid_ = -1;
  i_size_ = 0;
  for (size_t i = 0; i < 10; i++) {
    i_addr_[i] = 0;
  }
  i_last_read_ = -1;
  i_acess_time_ = 0;
  i_modify_time_ = 0;
}

Inode::~Inode() {}

void Inode::ReadInode() {
  if (DEBUG)
    Print("Inode Info", "execute fuction ReadInode()");
  int logic_block_id = 0; // 文件的逻辑块号
  int block_id = 0;       // 对应的物理块号
  int offset = 0;         // 当前字符块内起始的地址
  int bytes_num = 0;      // 已经传送的字节数
  Buffer *p_buffer = NULL;
  // 如果需要读取的字节数为0，则返回
  if (p_user->u_ioparam.io_count_ == 0) {
    Print("Inode Info", "have no rest data to read");
    return;
  }
  i_flag_ |= Inode::I_ACC;
  while (p_user->u_error_code_ == User::U_NOERROR &&
         p_user->u_ioparam.io_count_) {
    logic_block_id = p_user->u_ioparam.io_offset_ / Inode::BLOCK_SIZE;
    offset = p_user->u_ioparam.io_offset_ % Inode::BLOCK_SIZE;
    // 需要读取的字节数等于需要的剩余字节数和当前字符块的剩余有效字节数
    bytes_num = min(p_user->u_ioparam.io_count_, Inode::BLOCK_SIZE - offset);
    // 对文件结尾的判断
    int rest = i_size_ - p_user->u_ioparam.io_offset_;
    if (rest <= 0) {
      return;
    }
    // 实际读取的字节数还需要再和文件剩余字节数比较
    bytes_num = min(bytes_num, rest);
    block_id = MapBlock(logic_block_id);
    if (block_id == 0) {
      Print("Inode Info", "map block id filed");
      return;
    }
    // 读取缓存块
    p_buffer = p_buffer_manager->ReadBlock(block_id);
    // 记录最近读取字符块的逻辑块号
    i_last_read_ = logic_block_id;
    // 缓存中的读取开始位置
    unsigned char *buffer_start_addr = p_buffer->b_addr_ + offset;
    if (DEBUG) {
      cout << "[Inode Info] "
           << "buffer address:" << (void *)(p_buffer->b_addr_)
           << "  buffer offset:" << offset << "  read bytes nums:" << bytes_num
           << endl;
    }
    // 开始读取，成功之后更新读取位置
    memcpy(p_user->u_ioparam.io_start_addr_, buffer_start_addr, bytes_num);
    p_user->u_ioparam.io_start_addr_ += bytes_num;
    p_user->u_ioparam.io_offset_ += bytes_num;
    p_user->u_ioparam.io_count_ -= bytes_num;
    // 释放缓存资源
    p_buffer_manager->ReleaseBuffer(p_buffer);
  }
  return;
}

void Inode::WriteInode() {
  if (DEBUG)
    Print("Inode Info", "execute fuction WriteInode()");
  int logic_block_id = 0; // 文件的逻辑块号
  int block_id = 0;       // 对应的物理块号
  int offset = 0;         // 当前字符块内起始的地址
  int bytes_num = 0;      // 已经传送的字节数
  Buffer *p_buffer;
  // 设置Inode标志位
  i_flag_ |= (Inode::I_ACC | Inode::I_UPD);
  if (p_user->u_ioparam.io_count_ == 0) {
    return;
  }
  while (p_user->u_error_code_ == User::U_NOERROR &&
         p_user->u_ioparam.io_count_) {
    logic_block_id = p_user->u_ioparam.io_offset_ / Inode::BLOCK_SIZE;
    offset = p_user->u_ioparam.io_offset_ % Inode::BLOCK_SIZE;
    // 需要写入的字节数等于需要的剩余字节数和当前字符块的剩余有效字节数
    bytes_num = min(p_user->u_ioparam.io_count_, Inode::BLOCK_SIZE - offset);
    // 将逻辑块好转化为物理块号
    block_id = MapBlock(logic_block_id);
    if (block_id == 0) {
      return;
    }
    if (bytes_num == Inode::BLOCK_SIZE) {
      // 写入的数据刚好满512字节，为其分配缓存
      p_buffer = p_buffer_manager->GetBlock(block_id);
    } else {
      // 写入数据不满512字节，先读取再写入
      p_buffer = p_buffer_manager->ReadBlock(block_id);
    }
    // 缓存中写的起始位置
    unsigned char *buffer_start_addr = p_buffer->b_addr_ + offset;
    if (DEBUG) {
      cout << "[Inode Info] "
           << "buffer address:" << (void *)(p_buffer->b_addr_)
           << "  buffer offset:" << offset << "  write bytes nums:" << bytes_num
           << endl;
    }
    // 写入并更新
    memcpy(buffer_start_addr, p_user->u_ioparam.io_start_addr_, bytes_num);
    p_user->u_ioparam.io_start_addr_ += bytes_num;
    p_user->u_ioparam.io_offset_ += bytes_num;
    p_user->u_ioparam.io_count_ -= bytes_num;
    // 写过程出错
    if (p_user->u_error_code_) {
      p_buffer_manager->ReleaseBuffer(p_buffer);
    }
    p_buffer_manager->WriteBlockDelay(p_buffer);
    // 更新文件长度
    if (i_size_ < p_user->u_ioparam.io_offset_) {
      i_size_ = p_user->u_ioparam.io_offset_;
    }
    i_flag_ |= Inode::I_UPD;
  }
  return;
}

int Inode::MapBlock(int logic_block_id) {
  if (DEBUG)
    Print("Inode Info", "execute fuction MapBlock(...)");
  Buffer *p_buffer_first = NULL, *p_buffer_second = NULL;
  int block_id = 0;
  int *index_table;
  int index;
  // 判断最大文件范围
  if (logic_block_id > Inode::HUGE_FILE_BLOCK) {
    p_user->u_error_code_ = User::U_EFBIG;
    return 0;
  }
  // 小型文件
  if (logic_block_id < 6) {
    block_id = i_addr_[logic_block_id];
    // 分配一个物理块，通常发生在对文件进行写入
    if (block_id == 0 &&
        (p_buffer_first = p_file_system->AllocBlock()) != NULL) {
      // 使用延迟写，因为后面马上又要用到，不需要着急写入
      p_buffer_manager->WriteBlockDelay(p_buffer_first);
      block_id = p_buffer_first->b_blkno_;
      i_addr_[logic_block_id] = block_id;
      i_flag_ |= Inode::I_UPD;
    }
    return block_id;
  } else {
    // 大型或者巨型文件
    if (logic_block_id < Inode::LARGE_FILE_BLOCK) {
      index = (logic_block_id - Inode::SMALL_FILE_BLOCK) /
                  Inode::ADDRESS_PER_INDEX_BLOCK +
              6;
    } else {
      index = (logic_block_id - Inode::LARGE_FILE_BLOCK) /
                  (Inode::ADDRESS_PER_INDEX_BLOCK *
                   Inode::ADDRESS_PER_INDEX_BLOCK) +
              8;
    }
    // 如果为0，则说明不存在该间接索引块，分配一块空闲块作为间接索引
    block_id = i_addr_[index];
    if (block_id == 0) {
      i_flag_ |= Inode::I_UPD;
      if ((p_buffer_first = p_file_system->AllocBlock()) == NULL) {
        // 分配失败
        Print("Inode Info", "alloc block field");
        return 0;
      }
      // 分配成功，记录间接索引块
      i_addr_[index] = p_buffer_first->b_blkno_;
    } else {
      // 存在索引块，读出索引块
      p_buffer_first = p_buffer_manager->ReadBlock(block_id);
    }
    // ?获取缓冲区首地址
    index_table = (int *)p_buffer_first->b_addr_;
    // 巨型文件的索引块占据8 9
    if (index >= 8) {
      // 对于巨型文件的情况，pFirstBuf中是二次间接索引表，
      // 还需根据逻辑块号，经由二次间接索引表找到一次间接索引表
      index = ((logic_block_id - Inode::LARGE_FILE_BLOCK) /
               Inode::ADDRESS_PER_INDEX_BLOCK) %
              Inode::ADDRESS_PER_INDEX_BLOCK;
      // 此时index_table指向缓存中的二次间接索引表
      block_id = index_table[index];
      // 不存在一次间接索引表，则分配
      if (block_id == 0) {
        if ((p_buffer_second = p_file_system->AllocBlock()) == NULL) {
          // 分配失败
          Print("Inode Info", "alloc block field");
          p_buffer_manager->ReleaseBuffer(p_buffer_first);
          return 0;
        }
        // 新分配的一次间接索引块号记录在二次间接索引表中
        index_table[index] = p_buffer_second->b_blkno_;
        p_buffer_manager->WriteBlock(p_buffer_first);
      } else {
        // 释放二次间接索引表，读入一次间接索引表
        p_buffer_manager->ReleaseBuffer(p_buffer_first);
        p_buffer_second = p_buffer_manager->ReadBlock(block_id);
      }
      // 指向一次间接索引表
      p_buffer_first = p_buffer_second;
      index_table = (int *)p_buffer_second->b_addr_;
    }
    // 计算真正对应的物理块号
    if (logic_block_id < Inode::LARGE_FILE_BLOCK) {
      index = (logic_block_id - Inode::SMALL_FILE_BLOCK) %
              Inode::ADDRESS_PER_INDEX_BLOCK;
    } else {
      index = (logic_block_id - Inode::LARGE_FILE_BLOCK) %
              Inode::ADDRESS_PER_INDEX_BLOCK;
    }
    block_id = index_table[index];
    if (block_id == 0 &&
        (p_buffer_second = p_file_system->AllocBlock()) != NULL) {
      // 将分配的文件数据盘块号登记在一次间接索引表中
      block_id = p_buffer_second->b_blkno_;
      index_table[index] = block_id;
      // 将更改的索引表延迟写入磁盘
      p_buffer_manager->WriteBlock(p_buffer_second);
      p_buffer_manager->WriteBlock(p_buffer_first);
    } else {
      // 释放一次间接索引表
      p_buffer_manager->ReleaseBuffer(p_buffer_first);
    }
    return block_id;
  }
  return 0;
}

void Inode::UpdateInode(int time) {
  if (DEBUG)
    Print("Inode Info", "execute fuction UpdateInode(...)");
  Buffer *p_buffer;
  DiskInode disk_inode;
  // 当UPD和IACC标志之一被设置，才需要更新相应DiskInode
  if (i_flag_ & (Inode::I_UPD | Inode::I_ACC)) {
    p_buffer = p_buffer_manager->ReadBlock(
        FileSystem::INODE_START_ADDR + i_id_ / FileSystem::INODE_NUM_PER_BLOCK);
    // 更新DiskInode
    disk_inode.d_mode_ = i_mode_;
    disk_inode.d_nlink_ = i_nlink_;
    disk_inode.d_uid_ = i_uid_;
    disk_inode.d_gid_ = i_gid_;
    disk_inode.d_size_ = i_size_;
    for (size_t i = 0; i < 10; i++) {
      disk_inode.d_addr_[i] = i_addr_[i];
    }
    if (i_flag_ & Inode::I_ACC) {
      disk_inode.d_acess_time_ = time;
    }
    if (i_flag_ & Inode::I_UPD) {
      disk_inode.d_modify_time_ = time;
    }
    // 将DiskInode写入磁盘
    DiskInode *p_disk_inode = &disk_inode;
    unsigned char *p =
        p_buffer->b_addr_ +
        (i_id_ % FileSystem::INODE_NUM_PER_BLOCK) * sizeof(DiskInode);
    ;
    memcpy(p, p_disk_inode, sizeof(DiskInode));
    p_buffer_manager->WriteBlock(p_buffer);
  }
}

void Inode::CleanInode() {
  // Inode::Clean()特定用于AllocInode()中清空新分配DiskInode的原有数据，
  // 即旧文件信息。Clean()函数中不应当清除i_dev, i_number, i_flag, i_count,
  // 这是属于内存Inode而非DiskInode包含的旧文件信息，而Inode类构造函数需要
  // 将其初始化为无效值。
  if (DEBUG)
    Print("Inode Info", "execute fuction CleanInode()");
  i_mode_ = 0;
  i_nlink_ = 0;
  i_uid_ = -1;
  i_gid_ = -1;
  i_size_ = 0;
  i_last_read_ = -1;
  for (size_t i = 0; i < 10; i++) {
    i_addr_[i] = 0;
  }
  return;
}

void Inode::CopyInode(Buffer *p_buffer, int id) {
  if (DEBUG)
    Print("Inode Info", "execute fuction CopyInode(...)");
  // 获取DiskInode内容引用
  DiskInode &disk_inode = *(
      DiskInode *)(p_buffer->b_addr_ +
                   (id % FileSystem::INODE_NUM_PER_BLOCK) * sizeof(DiskInode));
  i_mode_ = disk_inode.d_mode_;
  i_nlink_ = disk_inode.d_nlink_;
  i_uid_ = disk_inode.d_uid_;
  i_gid_ = disk_inode.d_gid_;
  i_size_ = disk_inode.d_size_;
  for (size_t i = 0; i < 10; i++) {
    i_addr_[i] = disk_inode.d_addr_[i];
  }
  // ?上面应该赋值过一遍
  memcpy(i_addr_, disk_inode.d_addr_, sizeof(i_addr_));
}

void Inode::TruncateInode() {
  if (DEBUG)
    Print("Inode Info", "execute fuction TruncateInode()");
  // 采用FILO方式释放，以尽量使得SuperBlock中记录的空闲盘块号连续
  for (int i = 9; i >= 0; i--) {
    // 如果i_addr[]中第i项存在索引
    if (i_addr_[i] != 0) {
      if (i >= 6 && i <= 9) {
        Buffer *p_buffer_first = p_buffer_manager->ReadBlock(i_addr_[i]);
        int *p_first = (int *)p_buffer_first->b_addr_;
        // 每张间接索引表记录 512/sizeof(int) =
        // 128个磁盘块号，遍历这全部128个磁盘块
        for (int j = 128 - 1; j >= 0; j--) {
          if (p_first[j] != 0) {
            // 如果是两次间接索引表，i_addr_[8]或i_addr[9]_项，
            // 那么该字符块记录的是128个一次间接索引表存放的磁盘块号
            if (i >= 8 && i <= 9) {
              Buffer *p_buffer_second = p_buffer_manager->ReadBlock(p_first[j]);
              int *p_second = (int *)p_buffer_second->b_addr_;
              for (int k = 128 - 1; k >= 0; k--) {
                if (p_second[k] != 0) {
                  p_file_system->FreeBlock(p_second[k]);
                }
              }
              p_buffer_manager->ReleaseBuffer(p_buffer_second);
            }
            p_file_system->FreeBlock(p_first[j]);
          }
        }
        p_buffer_manager->ReleaseBuffer(p_buffer_first);
      }
      p_file_system->FreeBlock(i_addr_[i]);
      i_addr_[i] = 0;
    }
  }
  // 盘块释放完毕，文件大小清零
  i_size_ = 0;
  i_flag_ |= Inode::I_UPD;
  i_mode_ &= ~(Inode::ILARG);
  i_nlink_ = 1;
  return;
}
