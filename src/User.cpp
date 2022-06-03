#include "User.h"
#include "FileManager.h"
#include "UserManager.h"
#include "Utils.h"
#include <cstring>
#include <fstream>
#include <iomanip>

extern FileManager *p_file_manager;
extern UserManager *p_user_manager;
User::User() {
  u_error_code_ = User::U_NOERROR;
  u_dir_param_ = "/";
  u_dir_fact_ = "/";
  u_uid_ = 0;
  u_gid_ = 0;
  memset(u_args_, 0, sizeof(u_args_));
}

User::~User() {}

int User::GetFileMode(string mode) {
  int file_mode = 0;
  if (mode == "-r") {
    file_mode |= File::F_READ;
  } else if (mode == "-w") {
    file_mode |= File::F_WRITE;
  } else if (mode == "-rw" || mode == "-wr") {
    file_mode |= (File::F_READ | File::F_WRITE);
  } else {
    return -1;
  }
  return file_mode;
}

int User::GetInodeMode(string mode) {
  int inode_mode = 0;
  if (mode.length() != 3 || mode[0] < '0' || mode[0] > '7' || mode[1] < '0' ||
      mode[1] > '7' || mode[2] < '0' || mode[2] > '7') {
    return -1;
  } else {
    // mode:0000 000x xxxx xxxx,last 9 bits indicates the permissions.
    inode_mode = (mode[0] - 48) * 64 + (mode[1] - 48) * 8 + (mode[2] - 48) * 1;
  }
  return inode_mode;
}

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
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  return;
}

void User::Mkdir(string dir_name, string mode) {
  if (!CheckDirectoryParam(dir_name)) {
    return;
  }
  int new_mode = GetInodeMode(mode);
  new_mode |= Inode::IFDIR;
  u_args_[1] = new_mode;
  p_file_manager->MakeDirectory();
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  cout << "create new directory successfully" << endl;
  return;
}

void User::Create(string file_name, string mode) {
  if (!CheckDirectoryParam(file_name)) {
    return;
  }
  int new_mode = GetInodeMode(mode);
  if (new_mode == -1) {
    Print("Error", "mode not support");
    return;
  }
  u_args_[1] = new_mode;
  p_file_manager->Create();
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  cout << "create file successfully, file descriptor is " << u_ar0[User::EAX]
       << endl;
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
  int new_mode = GetFileMode(mode);
  if (new_mode == 0) {
    Print("Error", "mode not support");
    return;
  }
  u_args_[1] = new_mode;
  p_file_manager->Open();
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  cout << "open file successfully, file descriptor is " << u_ar0[User::EAX]
       << endl;
  return;
}

void User::Close(string fd) {
  if (!IsDigit(fd)) {
    Print("Error", "close param error");
    return;
  }
  u_args_[0] = stoi(fd);
  p_file_manager->Close();
  if (u_error_code_)
    HandleError(u_error_code_);
  cout << "close file successfully, which file descriptor is " << u_args_[0]
       << endl;
  return;
}

void User::Seek(string fd, string offset, string origin) {
  if (!IsDigit(fd) || !IsDigit(offset) || !IsDigit(origin)) {
    Print("Error", "seek param error");
    return;
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
  if (!IsDigit(fd)) {
    Print("Error", "write param error");
    return;
  }
  int fd_write = stoi(fd);
  int write_size;
  char *write_buffer = NULL;
  if (input_file == "") {
    // 从控制台输入
    write_size = strlen(size.c_str());
    u_args_[1] = (long)size.c_str();
  } else {
    // 从文件输入，需要检查size
    if (!IsDigit(size)) {
      Print("Error", "write param error");
      return;
    }
    write_size = stoi(size);
    write_buffer = new char[write_size];
    fstream file_in(input_file, ios::in | ios::binary);
    if (!file_in) {
      Print("Error", "write file open error");
      return;
    }
    file_in.read(write_buffer, write_size);
    file_in.close();
    u_args_[1] = (long)write_buffer;
  }
  if (DEBUG) {
  }
  cout << "[Write Info]"
       << " fd:" << fd_write << " size:" << write_size << endl;
  u_args_[0] = fd_write;
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
  if (!IsDigit(fd)) {
    Print("Error", "read param error");
    return;
  }
  int fd_read = stoi(fd);
  if (!IsDigit(size)) {
    Print("Error", "read param error");
    return;
  }
  int read_size = stoi(size);

  char *read_buffer = new char[read_size];
  u_args_[0] = fd_read;
  u_args_[1] = (long)read_buffer;
  u_args_[2] = read_size;
  p_file_manager->Read();
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  cout << "[Read Info]"
       << "fd: " << fd_read << "  size: " << read_size << endl;

  if (output_file == "") {
    // 控制台输出
    cout << "read content: ";
    for (size_t i = 0; i < u_ar0[User::EAX]; i++) {
      cout << read_buffer[i];
    }
    cout << endl;
  } else {
    // 文件输出
    ofstream file_out(output_file, ios::out | ios::binary);
    if (!file_out) {
      Print("Error", "read file open error");
      return;
    }
    file_out.write(read_buffer, u_ar0[User::EAX]);
    file_out.close();
  }
  cout << "[Read Info]"
       << "read successfully:" << u_ar0[User::EAX] << " bytes" << endl;
  delete[] read_buffer;
  return;
}

void User::Ls() {
  u_list_.clear();
  vector<string> name_list = p_file_manager->List();
  if (u_error_code_) {
    HandleError(u_error_code_);
    return;
  }
  // 输出文件信息
  if (name_list.size()) {
    cout << std::left << setw(12) << "permission" << setw(8) << "nlink"
         << setw(8) << "owner" << setw(8) << "group" << setw(12) << "size"
         << setw(20) << "name" << endl;
    for (size_t i = 0; i < name_list.size(); i++) {
      cout << std::left << setw(12) << GetPermission(u_list_[i]->i_mode_)
           << setw(8) << u_list_[i]->i_nlink_ << setw(8) << u_list_[i]->i_uid_
           << setw(8) << u_list_[i]->i_gid_ << setw(12) << u_list_[i]->i_size_
           << setw(20) << name_list[i] << endl;
    }
  }
  return;
}

void User::UserList() {
  cout << std::left << setw(12) << "username" << setw(8) << "uid" << endl;
  for (int i = 0; i < p_user_manager->user_num_; i++) {
    cout << std::left << setw(12) << p_user_manager->users_info_[i].username
         << setw(8) << p_user_manager->users_info_[i].uid << endl;
  }
}

string User::GetPermission(unsigned int mode) {
  string permission;
  if ((mode & 0x4000) == 0)
    permission += "-";
  else
    permission += "d";
  // permissions of file owner
  if ((mode & 0x0100) == 0)
    permission += "-";
  else
    permission += "r";
  if ((mode & 0x0080) == 0)
    permission += "-";
  else
    permission += "w";
  if ((mode & 0x0040) == 0)
    permission += "-";
  else
    permission += "x";
  // permissions of users in the same group
  if ((mode & 0x0020) == 0)
    permission += "-";
  else
    permission += "r";
  if ((mode & 0x0010) == 0)
    permission += "-";
  else
    permission += "w";
  if ((mode & 0x0008) == 0)
    permission += "-";
  else
    permission += "x";
  // permissions of other users
  if ((mode & 0x0004) == 0)
    permission += "-";
  else
    permission += "r";
  if ((mode & 0x0002) == 0)
    permission += "-";
  else
    permission += "w";
  if ((mode & 0x0001) == 0)
    permission += "-";
  else
    permission += "x";
  return permission;
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
    err_message = "it is not a directory";
    break;
  case U_EISDIR:
    err_message = "it is a directory";
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
  case U_EEXIST:
    err_message = "file or directory exists";
  default:
    err_message = "something went wrong";
    break;
  }
  Print("Error", err_message.c_str());
  u_error_code_ = U_NOERROR;
  return;
}