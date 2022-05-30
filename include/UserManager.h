#ifndef USERMANAGER_H
#define USERMANAGER_H
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

  UserItem users_info_[USER_NUM];
  int user_num_;
  int current_user_;
  void LoadUser(const char *user_list);
  void Login(string username, string password);
  void Logout();
  void AddUser(string username, string passward, string uid);
  void DeleteUser(string username);
  void UpdateUser();
};

#endif