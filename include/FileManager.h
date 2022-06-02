#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "File.h"
#include "FileSystem.h"
#include "Inode.h"
#include "InodeTable.h"
#include "OpenfileTable.h"
#include <vector>
class FileManager {
public:
  enum DirectorySearchMode {
    OPEN = 0,   // 打开方式搜索
    CREATE = 1, // 新建文件方式搜索
    DELETE = 2  // 删除文件方式
  };
  FileManager();
  ~FileManager();
  void Open();
  void Create();
  void MakeDirectory();
  void OpenCommon(Inode *p_inode, int mode, int trans);
  void Close();
  void Seek();
  void Read();
  void Write();
  void ReadWriteCommon(enum File::FileFlags mode);
  int CheckAccess(Inode *p_inode, unsigned int mode);
  Inode *SearchDirectory(enum FileManager::DirectorySearchMode mode);
  Inode *MakeInode(unsigned int mode);
  void WriteDirectory(Inode *p_inode);
  void ChangeDirectory();
  void Unlink();
  vector<string> List();
  Inode *root_inode_; // 根目录对应的Inode
};

#endif