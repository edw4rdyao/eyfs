#include "Buffer.h"
#include <iostream>

Buffer::Buffer() {
  b_flags_ = 0;
  b_forw_ = NULL;
  b_back_ = NULL;
  b_wcount_ = 0;
  b_addr_ = NULL;
  b_blkno_ = -1;
  b_error_ = -1;
  b_resid_ = 0;
}

Buffer::~Buffer() {}