#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H
#include "Buffer.h"
#include "DeviceManager.h"
#include <iostream>
using namespace std;
// @class: BufferManager
// @brief: 缓存管理，管理缓存控制块Buffer
class BufferManager {
public:
  static const unsigned int BUFFERS_NUM = 100; // 缓存区的数量
  static const unsigned int BUFFER_SIZE = 512; // 每块缓存的大小
  // @param: 无
  // @brief: 初始化每个缓存块的b_forward和b_back
  // @return: void
  void Initialize();
  // @param: block_id: 需要申请的磁盘块号
  // @brief: 向设备申请一块块号为block_id的磁盘块
  // @return: Buffer* 申请成功后的缓存控制块Buffer的引用
  Buffer *GetBlock(int block_id);
  // @param: p_buffer: 需要释放的buffer的引用
  // @brief: 释放缓存控制块buffer
  // @return: void
  void ReleaseBuffer(Buffer *p_buffer);
  // @param: block_id: 需要读取的磁盘块号
  // @brief: 读取一个磁盘块到缓存中
  // @return: 返回读取后的缓存控制块的引用
  Buffer *ReadBlock(int block_id);
  // @param: p_buffer 需要写入磁盘块对应的缓存控制块
  // @brief: 将缓存控制块写入磁盘块
  // @return: void
  void WriteBlock(Buffer *p_buffer);
  // @param: p_buffer: 需要延迟写的磁盘块对应的缓存控制块
  // @brief:
  // @return: void
  void WriteBlockDelay(Buffer *p_buffer);
  // @param: p_buffer 需要清空的缓存控制块的引用
  // @brief: 清空缓存控制块内容
  // @return: void
  void ClearBuffer(Buffer *p_buffer);
  // @param:
  // @brief:
  // @return:
  void FlushBlock();
  // @param:
  // @brief:
  // @return:
  void FormatBlock();
  // @param:
  // @brief:
  // @return:
  void PushBuffer(Buffer *p_buffer);
  // @param:
  // @brief:
  // @return:
  void PopBuffer(Buffer *p_buffer);
  // @param:
  // @brief:
  // @return:
  BufferManager(DeviceManager *p_device_manager);
  // @param:
  // @brief:
  // @return:
  ~BufferManager();

private:
  DeviceManager *p_device_manager_; // 磁盘设备管理
  Buffer *bm_free_list_;            // 缓存控制块的自由缓存队列
  Buffer bm_buffers_[BUFFERS_NUM];  // 缓存控制块数组
  unsigned char bm_mem_buffers_[BUFFERS_NUM][BUFFER_SIZE]; // 缓存区数组
};

#endif