#include "BufferManager.h"
#include "Utils.h"
#include <cstring>

extern DeviceManager *p_device_manager;

BufferManager::BufferManager() {
  bm_free_list_ = new Buffer;
  FormatBlock();
}

BufferManager::~BufferManager() {
  FlushBlock();
  delete bm_free_list_;
}

void BufferManager::Initialize() {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction Initialize()");
  // 初始化设备队列
  bm_map_.clear();
  // 初始化自由缓存队列
  for (size_t i = 0; i < BufferManager::BUFFERS_NUM; i++) {
    // 将每个缓存块的forward连起来
    if (i == 0) {
      bm_buffers_[i].b_forw_ = bm_free_list_;
      bm_free_list_->b_back_ = bm_buffers_ + i;
    } else {
      bm_buffers_[i].b_forw_ = bm_buffers_ + i - 1;
    }
    // 将每个缓存块的back连起来
    if (i + 1 == BufferManager::BUFFERS_NUM) {
      bm_buffers_[i].b_back_ = bm_free_list_;
      bm_free_list_->b_forw_ = bm_buffers_ + i;
    } else {
      bm_buffers_[i].b_back_ = bm_buffers_ + i + 1;
    }
    bm_buffers_[i].b_addr_ = bm_mem_buffers_[i];
  }
  return;
}

Buffer *BufferManager::GetBlock(int block_id) {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction GetBlock(...)");
  Buffer *p_tmp = NULL;
  // 在当前的设备队列中搜索，找到则返回
  if (bm_map_.find(block_id) != bm_map_.end()) {
    p_tmp = bm_map_[block_id];
    PopBuffer(p_tmp);
    return p_tmp;
  }
  // 找不到则分配自由缓存队列
  p_tmp = bm_free_list_->b_back_;
  if (p_tmp == bm_free_list_) {
    cout << "[Info] buffer free list is full" << endl;
    return NULL;
  }
  PopBuffer(p_tmp);
  bm_map_.erase(p_tmp->b_blkno_);
  // 如果具有延迟写标志，则写入设备
  if (p_tmp->b_flags_ & Buffer::B_DELWRT) {
    p_device_manager->WriteImage(p_tmp->b_addr_, BUFFER_SIZE,
                                 p_tmp->b_blkno_ * BUFFER_SIZE);
  }
  // 删除延迟写标志和done标志，更新设备缓存
  p_tmp->b_flags_ &= ~(Buffer::B_DELWRT | Buffer::B_DONE);
  p_tmp->b_blkno_ = block_id;
  bm_map_[block_id] = p_tmp;
  return p_tmp;
}

void BufferManager::ReleaseBuffer(Buffer *p_buffer) {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction ReleaseBuffer(...)");
  PushBuffer(p_buffer);
  return;
}

Buffer *BufferManager::ReadBlock(int block_id) {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction ReadBlock(...)");
  Buffer *p_tmp = NULL;
  p_tmp = GetBlock(block_id);
  // 如果该缓存块中具有延迟写标志，则直接返回
  if (p_tmp->b_flags_ & (Buffer::B_DELWRT | Buffer::B_DONE)) {
    return p_tmp;
  }
  // 否则读取设备
  p_device_manager->ReadImage(p_tmp->b_addr_, BUFFER_SIZE,
                              p_tmp->b_blkno_ * BUFFER_SIZE);
  p_tmp->b_flags_ |= Buffer::B_DONE;
  return p_tmp;
}

void BufferManager::WriteBlock(Buffer *p_buffer) {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction WriteBlock(...)");
  // 清除延迟写标志
  p_buffer->b_flags_ &= ~(Buffer::B_DELWRT);
  p_device_manager->WriteImage(p_buffer->b_addr_, BUFFER_SIZE,
                               p_buffer->b_blkno_ * BUFFER_SIZE);
  // 清楚busy标志
  p_buffer->b_flags_ |= Buffer::B_DONE;
  ReleaseBuffer(p_buffer);
  return;
}

void BufferManager::WriteBlockDelay(Buffer *p_buffer) {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction WriteBlockDelay(...)");
  // 加BDONE标志允许其他进程使用磁盘内容
  p_buffer->b_flags_ |= (Buffer::B_DELWRT | Buffer::B_DONE);
  ReleaseBuffer(p_buffer);
  return;
}

void BufferManager::ClearBuffer(Buffer *p_buffer) {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction ClearBuffer(...)");
  memset(p_buffer->b_addr_, 0, BUFFER_SIZE);
  return;
}

void BufferManager::FlushBlock() {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction FlushBlock()");
  // 将所有未写入的缓存写入到设备中
  Buffer *p_tmp = NULL;
  for (size_t i = 0; i < BUFFERS_NUM; i++) {
    p_tmp = bm_buffers_ + i;
    if (p_tmp->b_flags_ & Buffer::B_DELWRT) {
      p_tmp->b_flags_ &= ~(Buffer::B_DELWRT);
      p_device_manager->WriteImage(p_tmp->b_addr_, BUFFER_SIZE,
                                   p_tmp->b_blkno_ * BUFFER_SIZE);
      p_tmp->b_flags_ |= Buffer::B_DONE;
    }
  }
}

void BufferManager::FormatBlock() {
  if (DEBUG)
    Print("BufferManager Info", "execute fuction FormatBlock()");
  // 初始化每块缓存以及缓存队列
  Buffer empty_buffer;
  for (size_t i = 0; i < BUFFERS_NUM; i++) {
    memcpy(bm_buffers_ + i, &empty_buffer, sizeof(Buffer));
  }
  Initialize();
}

void BufferManager::PushBuffer(Buffer *p_buffer) {
  if (p_buffer->b_back_ != NULL) {
    return;
  }
  p_buffer->b_forw_ = bm_free_list_->b_forw_;
  p_buffer->b_back_ = bm_free_list_;
  bm_free_list_->b_forw_->b_back_ = p_buffer;
  bm_free_list_->b_forw_ = p_buffer;
  return;
}

void BufferManager::PopBuffer(Buffer *p_buffer) {
  // 将缓存块从队列中移除
  if (p_buffer->b_back_ == NULL) {
    return;
  }
  p_buffer->b_forw_->b_back_ = p_buffer->b_back_;
  p_buffer->b_back_->b_forw_ = p_buffer->b_forw_;
  p_buffer->b_back_ = NULL;
  p_buffer->b_forw_ = NULL;
  return;
}