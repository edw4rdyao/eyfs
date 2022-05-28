#include "UserManager.h"
#include "User.h"
#include "Utils.h"

extern User *p_user;

UserManager::UserManager() {
  user_num_ = 0;
  current_user_ = -1;
  for (int i = 0; i < USER_NUM; i++) {
    users_info_[i].username = "";
    users_info_[i].password = "";
    users_info_[i].uid = "";
  }
}

UserManager::~UserManager() {}

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
        // TODO 打开root根目录
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
  // TODO 将用户打开所有文件关闭
}