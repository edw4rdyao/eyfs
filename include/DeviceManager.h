#ifndef DEVIICEMANAGER_H
#define DEVIICEMANAGER_H

#include <cstdio>
#include <iostream>
using namespace std;

class DeviceManager {
private:
  const char *image_name_;
  FILE *image_fp_; // 磁盘设备的文件指针
public:
  // @param: 无
  // @brief: 以写文件的方式打开镜像文件，用于写镜像
  // @ret: void
  void OpenImage();
  // @param: buffer 写入的数据 size 写入的数据大小 offset 地址偏移 origin 源地址
  // @brief: 写入镜像文件
  // @ret: void
  void WriteImage(const void *buffer, unsigned int size, int offset = -1,
                  unsigned int origin = SEEK_SET);
  // @param: buffer 读入的数据 size 读入的数据大小 offset 地址偏移 origin 源地址
  // @brief: 读取镜像文件
  // @ret: void
  void ReadImage(void *buffer, unsigned int size, int offset = -1,
                  unsigned int origin = SEEK_SET);
  // @param: 无
  // @brief: 检查镜像文件是否正常打开
  // @ret: bool值 ture为正常
  bool CheckImage();
  DeviceManager(const char *image_name);
  ~DeviceManager();
};

#endif