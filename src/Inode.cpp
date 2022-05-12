#include "Inode.h"

Inode::Inode() {
  // 初始化属性
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
}

Inode::~Inode() {}

