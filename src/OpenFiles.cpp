#include "OpenFiles.h"
#include "cstring"

OpenFiles::OpenFiles() {
  memset(ProcessOpenFilesTable, NULL, sizeof(ProcessOpenFilesTable));
}

OpenFiles::~OpenFiles() {}

int OpenFiles::AllocFreeSlot() {}

File *OpenFiles::GetFile(int fd) {
  return NULL;
}

void OpenFiles::SetFile(int fd, File *p_file) {}