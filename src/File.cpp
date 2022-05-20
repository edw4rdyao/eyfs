#include "File.h"

File::File() {
  f_count_ = 0;
  f_flag_ = 0;
  f_offset_ = 0;
  f_inode_ = NULL;
}

File::~File() {}