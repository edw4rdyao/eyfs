#include "OpenFiles.h"
#include <cstring>

OpenFiles::OpenFiles() {
  memset(ProcessOpenFilesTable, 0, sizeof(ProcessOpenFilesTable));
}

OpenFiles::~OpenFiles() {}

int OpenFiles::AllocFreeSlot() { return 0; }

File *OpenFiles::GetFile(int fd) { return NULL; }

void OpenFiles::SetFile(int fd, File *p_file) {}