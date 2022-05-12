#include "InodeTable.h"

InodeTable::InodeTable(FileSystem *p_file_system) {
  p_file_system_ = p_file_system;
}

InodeTable::~InodeTable() {}

Inode *InodeTable::GetInode(int id) { return NULL; }

void InodeTable::PutInode(Inode *p_inode) {}

void InodeTable::UpdateInodeTable() {}

int InodeTable::IsLoaded(int id) { return 0; }

Inode *InodeTable::GetFreeInode() { return NULL; }

void InodeTable::FormatInodeTable() {}