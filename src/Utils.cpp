#include "Utils.h"

void DisplayInfomation() {
  cout << "              ____    \n";
  cout << "  ___  __  __/ __/____\n";
  cout << " / _ \\/ / / / /_/ ___/\n";
  cout << "/  __/ /_/ / __(__  ) \n";
  cout << "\\___/\\__, /_/ /____/  \n";
  cout << "    /____/            \n";
}

vector<string> ParseCmd(string cmd) {
  istringstream isstring_cmd(cmd);
  string tmp;
  vector<string> cmd_args;
  // 解析输入的命令
  while (isstring_cmd >> tmp) {
    cmd_args.push_back(tmp);
  }
  return cmd_args;
}

void Print(const char *title, const char *message) {
  cout << "[" << title << "] " << message << endl;
}

void PrintLog() {
  time_t now = time(0);
  cout << ctime(&now);
  return;
}

bool IsDigit(string s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] < '0' || s[i] > '9') {
      return false;
    }
  }
  return true;
}