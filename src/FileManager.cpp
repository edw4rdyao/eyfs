#include "FileManager.h"
#include "FileSystem.h"
#include "InodeTable.h"
#include "OpenfileTable.h"
#include "User.h"
#include "Utils.h"

extern FileSystem *p_file_system;
extern OpenfileTable *p_openfile_table;
extern InodeTable *p_inode_table;
extern User *p_user;

FileManager::FileManager() {}

FileManager::~FileManager() {}

void FileManager::Open() {
  Inode *p_inode = NULL;
  p_inode = SearchDirectory(FileManager::OPEN);
  if (p_inode == NULL) {
    Print("FileManager Info", "search inode failed");
    return;
  }
  OpenCommon(p_inode, p_user->u_args_[1], 0);
  return;
}

void FileManager::Create() {}

void FileManager::OpenCommon(Inode *p_inode, int mode, int trans) {}

void FileManager::Close() {}

void FileManager::Seek() {}

void FileManager::Read() {}

void FileManager::Write() {}

void FileManager::ReadWriteCommon(enum File::FileFlags mode) {}

Inode *FileManager::SearchDirectory(enum DirectorySearchMode mode) {
  return NULL;
}

Inode *FileManager::MakeInode(unsigned int mode) { return NULL; }

void FileManager::WriteDirectory(Inode p_inode) {}

void FileManager::ChangeDirectory() {}

void FileManager::Unlink() {}

void FileManager::List() {}