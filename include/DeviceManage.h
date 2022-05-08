#ifndef DEVIICEMANAGE_H
#define DEVIICEMANAGE_H

#include<cstdio>

class DeviceManage
{
private:
  FILE* image_fp_;  // 磁盘设备的文件指针
public:
  // @param: 
  // @brief: 
  // @ret: 
  bool OpenImage();

  void ImageWrite();
  void ImageRead();
  DeviceManage();
  ~DeviceManage();
};

#endif