#ifndef IOPARAMETER_H
#define IOPARAMETER_H

class IOParameter {
public:
  unsigned char *io_start_addr_; // 当前用户读写首地址
  int io_offset_;                // 当前用户读写偏移量
  int io_count_;                 // 当前用户读写剩余数量
};

#endif