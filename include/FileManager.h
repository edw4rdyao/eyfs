#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "File.h"
#include "FileSystem.h"
#include "Inode.h"
#include "InodeTable.h"
#include "OpenfileTable.h"
#include "User.h"

class User;

class FileManager {
public:
  enum DirectorySearchMode {
    OPEN = 0,   // 打开方式搜索
    CREATE = 1, // 新建文件方式搜索
    DELETE = 2  // 删除文件方式
  };
  FileManager(InodeTable *p_inode_table, OpenfileTable *p_openfile_table,
              FileSystem *p_file_system);
  ~FileManager();
  // @param:
  // @brief:
  // @ret:
  void Open();
  // @param:
  // @brief:
  // @ret:
  void Create();
  // @param:
  // @brief:
  // @ret:
  void OpenCommon(Inode *p_inode, int mode, int trans);
  // @param:
  // @brief:
  // @ret:
  void Close();
  // @param:
  // @brief:
  // @ret:
  void Seek();
  // @param:
  // @brief:
  // @ret:
  void Read();
  // @param:
  // @brief:
  // @ret:
  void Write();
  // @param:
  // @brief:
  // @ret:
  void ReadWriteCommon(enum File::FileFlags mode);
  // @param:
  // @brief:
  // @ret:
  Inode *SearchInode(enum FileManager::DirectorySearchMode mode);
  Inode *MakeInode(unsigned int mode);
  void WriteDirectory(Inode p_inode);
  void ChangeDirectory();
  void Unlink();
  void List();
  Inode *root_inode_;               // 根目录对应的Inode
  FileSystem *p_file_system_;       // 文件系统的引用
  InodeTable *p_inode_table_;       // 内存Inode表的引用
  OpenfileTable *p_openfile_table_; // 系统打开文件表引用
  User *p_user_;                    // 当前用户的引用
};

#endif