#include "User.h"
#include "FileManager.h"
#include "Utils.h"
#include <cstring>
#include <fstream>

extern FileManager *p_file_manager;

User::User() {
  u_error_code_ = User::U_NOERROR;
  u_dir_param_ = "/";
  u_dir_fact_ = "/";
  u_uid_ = 0;
  u_gid_ = 0;
  memset(u_args_, 0, sizeof(u_args_));
}

User::~User() {}

int User::GetFileMode(string mode) {}

int User::GetInodeMode(string mode) {}

bool User::CheckDirectoryParam(string dir_name) {
  if (dir_name.empty()) {
    Print("Error", "param is empty");
    return false;
  }
  if (dir_name.substr(0, 2) != "..") {
    u_dir_param_ = dir_name;
  } else {
    string pre_dir = u_dir_fact_;
    unsigned int p = 0;
    // 多重路径
    for (; pre_dir.length() >= 3 && p < dir_name.length() &&
           dir_name.substr(p, 2) == "..";) {
      pre_dir.pop_back();
      pre_dir.erase(pre_dir.find_last_of('/') + 1);
      p += 2;
      p += (p < dir_name.length() && dir_name[p] == '/');
    }
    u_dir_param_ = pre_dir + dir_name.substr(p);
  }
  if (u_dir_param_.length() > 1 && u_dir_param_.back() == '/') {
    u_dir_param_.pop_back();
  }
  // 判断路径长度
  for (size_t i = 0, j = 0; i < u_dir_param_.length(); i = j + 1) {
    j = u_dir_param_.find('/', i);
    j = min(j, (size_t)u_dir_param_.length());
    if (j - i > DirectoryEntry::DIRSIZE) {
      Print("Error", "file path is too long");
      return false;
    }
  }
  return true;
}

void User::Cd(string dir_name) {
  if (!CheckDirectoryParam(dir_name)) {
    return;
  }
  p_file_manager->ChangeDirectory();
  if (u_error_code_)
    HandleError(u_error_code_);
  return;
}

void User::Mkdir(string dir_name) {
  if (!CheckDirectoryParam(dir_name)) {
    return;
  }
  u_args_[1] = Inode::IFDIR;
  p_file_manager->Create();
  if (u_error_code_)
    HandleError(u_error_code_);
  return;
}

void User::Create(string file_name, string mode) {
  if (!CheckDirectoryParam(file_name)) {
    return;
  }
  // int mode_mask = GetInodeMode(mode);
  int mode_mask = 0;
  mode_mask |= (Inode::IREAD | Inode::IWRITE);
  if (mode_mask == 0) {
    Print("Error", "mode not support");
    return;
  }
  u_args_[1] = mode_mask;
  p_file_manager->Create();
  if (u_error_code_) {
    HandleError(u_error_code_);
  }
  return;
}

void User::Delete(string file_name) {
  if (!CheckDirectoryParam(file_name)) {
    return;
  }
  p_file_manager->Unlink();
  if (u_error_code_)
    HandleError(u_error_code_);
  return;
}

void User::Open(string file_name, string mode) {
  if (!CheckDirectoryParam(file_name)) {
    return;
  }
  int mode_mask = 0;
  mode_mask |= (File::F_WRITE | File::F_READ);
  if (mode_mask == 0) {
    Print("Error", "mode not support");
    return;
  }
  u_args_[1] = mode_mask;
  p_file_manager->Open();
  if (u_error_code_)
    HandleError(u_error_code_);
  return;
}

void User::Close(string fd) {
  if (fd.empty() || !isdigit(fd.front())) {
    Print("Error", "close param error");
  }
  u_args_[0] = stoi(fd);
  p_file_manager->Close();
  if (u_error_code_)
    HandleError(u_error_code_);
  return;
}

void User::Seek(string fd, string offset, string origin) {
  if (fd.empty() || !isdigit(fd.front())) {
    Print("Error", "seek param error");
  }
  if (offset.empty()) {
    Print("Error", "seek param error");
  }
  if (origin.empty() || !isdigit(fd.front())) {
    Print("Error", "seek param error");
  }
  u_args_[0] = stoi(fd);
  u_args_[1] = stoi(offset);
  u_args_[2] = stoi(origin);
  p_file_manager->Seek();
  if (u_error_code_)
    HandleError(u_error_code_);
  return;
}

void User::Write(string fd, string input_file, string size) {
  if (fd.empty() || !isdigit(fd.front())) {
    Print("Error", "write param error");
  }
  int fd_write = stoi(fd);
  int write_size;
  if (size.empty() || !isdigit(size.front()) || (write_size = stoi(size)) < 0) {
    Print("Error", "write param error");
  }
  char *write_buffer = new char[write_size];
  fstream file_in(input_file, ios::in | ios::binary);
  if (!file_in) {
    Print("Error", "write file open error");
  }
  file_in.read(write_buffer, write_size);
  file_in.close();
  u_args_[0] = fd_write;
  u_args_[1] = (long)write_buffer;
  u_args_[2] = write_size;
  p_file_manager->Write();
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  // 输出写入结果
  cout << "[Write Info]"
       << "write successfully:" << u_ar0[User::EAX] << " bytes" << endl;
  delete[] write_buffer;
  return;
}

void User::Read(string fd, string output_file, string size) {
  if (fd.empty() || !isdigit(fd.front())) {
    Print("Error", "read param error");
  }
  int fd_read = stoi(fd);
  int read_size;
  if (size.empty() || !isdigit(size.front()) || (read_size = stoi(size)) < 0) {
    Print("Error", "read param error");
  }
  char *read_buffer = new char[read_size];
  u_args_[0] = fd_read;
  u_args_[1] = (long)read_buffer;
  u_args_[2] = read_size;
  p_file_manager->Read();
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  ofstream file_out(output_file, ios::out | ios::binary);
  if (!file_out) {
    Print("Error", "read file open error");
    return;
  }
  file_out.write(read_buffer, u_ar0[User::EAX]);
  file_out.close();
  Print("Read", "raed file successfully");
  delete[] read_buffer;
  return;
}

void User::Ls() {
  u_list_.clear();
  p_file_manager->List();
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  cout << u_list_ << endl;
  return;
}

void User::HandleError(enum ErrorCode err_code) {
  string err_message = "";
  switch (err_code) {
  case U_NOERROR:
    err_message = "no error";
    break;
  case U_ENOENT:
    err_message = "no such file or directory";
    break;
  case U_EBADF:
    err_message = "bad file number";
    break;
  case U_EACCES:
    err_message = "permission denied";
    break;
  case U_ENOTDIR:
    err_message = "not a directory";
    break;
  case U_ENFILE:
    err_message = "file table overflow";
    break;
  case U_EMFILE:
    err_message = "too many open files";
    break;
  case U_EFBIG:
    err_message = "file too large";
    break;
  case U_ENOSPC:
    err_message = "no space left on device";
    break;
  default:
    break;
  }
  Print("Error", err_message.c_str());
  u_error_code_ = U_NOERROR;
  return;
}