#include "BufferManager.h"

BufferManager::BufferManager() {}

BufferManager::~BufferManager() {}

void BufferManager::Initialize() {
  for (size_t i = 0; i < BufferManager::BUFFERS_NUM; i++) {
    // 将每个缓存块的forward连起来
    if (i == 0) {
      bm_buffers_[i].b_forw_ = bm_free_list_;
      bm_free_list_->b_back_ = bm_buffers_;
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

