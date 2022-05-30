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
    Print("Error", "only root can add user");
    return;
  }
  // 检查用户个数是否已满
  if (user_num_ == UserManager::USER_NUM) {
    Print("Error", "user num has reached the maximum");
    return;
  }
  // 检查格式
  if (username.size() > 8) {
    Print("Error", "username maximum length 6");
    return;
  } else if (passward.size() > 8 || passward.size() < 4) {
    Print("Error", "password length between 4-8");
    return;
  } else if (uid.size() > 4 || !IsDigit(uid)) {
    Print("Error", "uid is s number between 0 and 9999");
    return;
  }
  if (!Check(username) || !Check(passward)) {
    Print("Error", "username or password consists of numbers or letters");
    return;
  }
  // 检查是否存在该用户
  uid = to_string(stoi(uid));
  if (IsExistByUsername(username)) {
    Print("Error", "username exists");
    return;
  } else if (IsExistByUid(uid)) {
    Print("Error", "uid exists");
    return;
  }
  // 添加用户
  users_info_[user_num_].username = username;
  users_info_[user_num_].password = passward;
  users_info_[user_num_].uid = uid;
  user_num_++;
  cout << "add user " << username << " successfully" << endl;
  return;
}

void UserManager::DeleteUser(string username) {
  // 权限及合法性检查
  if (p_user->u_uid_ != 0) {
    Print("Error", "only root can add user");
    return;
  }
  if (username == "root") {
    Print("Error", "can not delete root");
    return;
  }
  // 删除用户
  for (size_t i = 0; i < user_num_; i++) {
    if (users_info_[i].username == username) {
      for (size_t j = i; j < user_num_; j++) {
        if (j == user_num_ - 1) {
          users_info_[j].username = "";
          users_info_[j].password = "";
          users_info_[j].uid = "";
        } else {
          users_info_[j].username = users_info_[j + 1].username;
          users_info_[j].password = users_info_[j + 1].password;
          users_info_[j].uid = users_info_[j + 1].uid;
        }
      }
      user_num_--;
      cout << "delete user " << username << " successfully" << endl;
      return;
    }
  }
  Print("Error", "user not exists");
  return;
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

bool UserManager::IsExistByUsername(string username) {
  for (size_t i = 0; i < user_num_; i++) {
    if (users_info_[i].username == username) {
      return true;
    }
  }
  return false;
}

bool UserManager::IsExistByUid(string uid) {
  for (size_t i = 0; i < user_num_; i++) {
    if (users_info_[i].uid == uid) {
      return true;
    }
  }
  return false;
}

bool UserManager::Check(string username) {
  for (size_t i = 0; i < username.size(); i++) {
    if (!isdigit(username[i]) && !isalpha(username[i])) {
      return false;
    }
  }
  return true;
}