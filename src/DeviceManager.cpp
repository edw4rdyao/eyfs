#include "DeviceManager.h"

DeviceManager::DeviceManager(const char *image_name) {
  image_name_ = image_name;
  image_fp_ = fopen(image_name_, "rb+");
}

DeviceManager::~DeviceManager() {
  if (image_fp_)
    fclose(image_fp_);
}

void DeviceManager::OpenImage() {
  image_fp_ = fopen(image_name_, "wb+");
  if (!CheckImage()) {
    throw "open image filed";
  }
}

void DeviceManager::WriteImage(const void *buffer, unsigned int size,
                               int offset, unsigned int origin) {
  if (offset != -1) {
    fseek(image_fp_, offset, origin);
  }
  fwrite(buffer, size, 1, image_fp_);
}

void DeviceManager::ReadImage(void *buffer, unsigned int size, int offset,
                              unsigned int origin) {
  if (offset != -1) {
    fseek(image_fp_, offset, origin);
  }
  fread(buffer, size, 1, image_fp_);
}

bool DeviceManager::CheckImage() { return image_fp_ != NULL; }