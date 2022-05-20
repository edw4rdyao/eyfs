#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "File.h"
#include "FileSystem.h"
#include "Inode.h"
#include "InodeTable.h"
#include "OpenfileTable.h"

class FileManager {
public:
  enum DirectorySearchMode {
    OPEN = 0,   // 打开方式搜索
    CREATE = 1, // 新建文件方式搜索
    DELETE = 2  // 删除文件方式
  };
  FileManager();
  ~FileManager();
  // @param:
  // @brief:
  // @return:
  void Open();
  // @param:
  // @brief:
  // @return:
  void Create();
  // @param:
  // @brief:
  // @return:
  void OpenCommon(Inode *p_inode, int mode, int trans);
  // @param:
  // @brief:
  // @return:
  void Close();
  // @param:
  // @brief:
  // @return:
  void Seek();
  // @param:
  // @brief:
  // @return:
  void Read();
  // @param:
  // @brief:
  // @return:
  void Write();
  // @param:
  // @brief:
  // @return:
  void ReadWriteCommon(enum File::FileFlags mode);
  // @param:
  // @brief:
  // @return:
  Inode *SearchDirectory(enum FileManager::DirectorySearchMode mode);
  Inode *MakeInode(unsigned int mode);
  void WriteDirectory(Inode* p_inode);
  void ChangeDirectory();
  void Unlink();
  void List();
  Inode *root_inode_; // 根目录对应的Inode
};

#endif