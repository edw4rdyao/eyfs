#include "DiskInode.h"

DiskInode::DiskInode() {
  // 初始化属性
  d_mode_ = 0;
  d_nlink_ = 0;
  d_uid_ = -1;
  d_gid_ = -1;
  d_size_ = 0;
  for (size_t i = 0; i < 10; i++) {
    d_addr_[i] = 0;
  }
  d_acess_time_ = 0;
  d_modify_time_ = 0;
}

DiskInode::~DiskInode() {}