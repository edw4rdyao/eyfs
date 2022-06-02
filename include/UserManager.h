#ifndef USERMANAGER_H
#define USERMANAGER_H
#include "User.h"
#include <iostream>
#include <string>
using namespace std;

struct UserItem {
  string username;
  string password;
  string uid;
};

class UserManager {
public:
  static const int USER_NUM = 20;
  UserManager();
  ~UserManager();
  void LoadUser(const char *user_list);
  void Login(string username, string password);
  void Logout();
  void AddUser(string username, string passward, string uid);
  void DeleteUser(string username);
  bool Check(string username);
  bool IsExistByUsername(string username);
  bool IsExistByUid(string uid);
  void UpdateUser();
  User users_[USER_NUM];          // 用户结构
  UserItem users_info_[USER_NUM]; // 用户信息，用户名密码
  int user_num_;                  // 系统的用户数
  int current_user_; // 当前用户在users_和users_info_中的索引
};

#endif