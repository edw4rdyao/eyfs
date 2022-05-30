#include "UserManager.h"
#include "FileManager.h"
#include "User.h"
#include "Utils.h"
#include <string.h>

extern User *p_user;
extern FileManager *p_file_manager;

UserManager::UserManager() {
  user_num_ = 0;
  current_user_ = -1;
  for (int i = 0; i < USER_NUM; i++) {
    users_info_[i].username = "";
    users_info_[i].password = "";
    users_info_[i].uid = "";
  }
}

UserManager::~UserManager() { UpdateUser(); }

void UserManager::AddUser(string username, string passward, string uid) {
  if (p_user->u_uid_ != 0) {
    // not root
  }
  // check
}

void UserManager::LoadUser(const char *user_list) {
  // 开始解析用户信息
  string s(user_list);
  int state = 0;
  for (int i = 0; i < s.length(); i++) {
    if (s[i] == '\n') {
      user_num_++;
      state = 0;
      continue;
    }
    if (s[i] == ':') {
      state = (state + 1) % 3;
      continue;
    }
    if (state == 0) {
      // 用户名
      users_info_[user_num_].username += s[i];
    } else if (state == 1) {
      // 密码
      users_info_[user_num_].password += s[i];
    } else {
      // uid
      users_info_[user_num_].uid += s[i];
    }
  }
}

void UserManager::Login(string username, string password) {
  for (int i = 0; i < user_num_; i++) {
    if (users_info_[i].username == username) {
      if (users_info_[i].password != password) {
        Print("Error", "password error");
        return;
      } else {
        current_user_ = i;
        p_user->u_uid_ = stoi(users_info_[i].uid);
        return;
      }
    }
  }
  Print("Error", "username not matched");
  return;
}

void UserManager::Logout() {
  // uid清空
  current_user_ = -1;
  p_user->u_uid_ = -1;
  // 将用户打开所有文件关闭
  for (size_t i = 0; i < OpenFiles::FILESNUM; i++) {
    if (p_user->u_openfiles_.process_openfile_table_[i]) {
      p_user->u_args_[0] = i;
      p_file_manager->Close();
    }
  }
}

void UserManager::UpdateUser() {
  string user_list = "";
  for (int i = 0; i < user_num_; i++) {
    user_list += users_info_[i].username;
    user_list += ':';
    user_list += users_info_[i].password;
    user_list += ':';
    user_list += users_info_[i].uid;
    user_list += '\n';
  }
  if (DEBUG) {
    cout << "[UserManager Info]" << user_list << endl;
  }
  // write userlist to /etc/user
  if (p_user->u_uid_ != 0) {
    Login("root", "root");
  }
  p_user->CheckDirectoryParam("/etc/user");
  p_user->u_args_[1] = p_user->GetInodeMode("644");
  p_file_manager->Create();
  int fd = p_user->u_ar0[User::EAX];
  p_user->u_args_[0] = fd;
  p_user->u_args_[1] = (long)user_list.c_str();
  p_user->u_args_[2] = strlen(user_list.c_str());
  p_file_manager->Write();
  p_user->u_args_[0] = fd;
  p_file_manager->Close();
  return;
}