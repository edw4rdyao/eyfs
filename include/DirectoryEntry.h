#ifndef DIRECTORYENTRY_H
#define DIRECTORYENTRY_H

class DirectoryEntry {
public:
  // 目录项中路径部分的最大字符串长度
  static const int DIRSIZE = 28;
  // 目录的Inode编号
  int inode_id;
  char name[DIRSIZE];
};

#endif