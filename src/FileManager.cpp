#include "FileManager.h"

FileManager::FileManager(InodeTable *p_inode_table,
                         OpenfileTable *p_openfile_table,
                         FileSystem *p_file_system) {
  p_inode_table_ = p_inode_table;
  p_openfile_table_ = p_openfile_table_;
  p_file_system_ = p_file_system_;
}

FileManager::~FileManager() {}

void FileManager::Open() {}

void FileManager::Create() {}

void FileManager::OpenCommon(Inode *p_inode, int mode, int trans) {}

void FileManager::Close() {}

void FileManager::Seek() {}

void FileManager::Read() {}

void FileManager::Write() {}

void FileManager::ReadWriteCommon(enum File::FileFlags mode) {}

Inode *FileManager::SearchInode(enum FileManager::DirectorySearchMode mode) {
  return NULL;
}

Inode *FileManager::MakeInode(unsigned int mode) { return NULL; }

void FileManager::WriteDirectory(Inode p_inode) {}

void FileManager::ChangeDirectory() {}

void FileManager::Unlink() {}

void FileManager::List() {}