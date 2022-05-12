#include "File.h"

File::File() {
  f_count = 0;
  f_flag = 0;
  f_offset = 0;
  f_inode = NULL;
}

File::~File() {}