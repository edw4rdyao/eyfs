#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H

class DirectoryEntry {
public:
  static const int DIRSIZE = 28; // 目录项中路径部分的最大字符串长度
  int inode_id_;                  // 目录的Inode编号
  char name_[DIRSIZE];            // 目录的标识符
};

#endif