#include "User.h"

User::User() {}

User::~User() {}

void User::Cd(string dir_name) {}

void User::Mkdir(string dir_name) {}

void User::Create(string file_name, string mode) {}

void User::Delete(string file_name) {}

void User::Open(string file_name, string mode) {}

void User::Close(string fd) {}

void User::Seek(string fd, string offset, string origin) {}

void User::Write(string fd, string input_file, string size) {}

void User::Read(string fd, string output_file, string size) {}

void User::Ls() {}

void User::HandleError(enum ErrorCode err_code) {}