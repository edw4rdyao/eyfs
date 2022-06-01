#ifndef UTILS_H
#define UTILS_H

#define DEBUG 1

#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
using namespace std;

// @param:
// @brief:
// @return:
void DisplayInfomation();

// @param:
// @brief:
// @return:
vector<string> ParseCmd(string cmd);

// @param:
// @brief:
// @return:
void PrintLog();

// @param
// @brief
// @return
void Print(const char* title, const char* message);

bool IsDigit(string s);

#endif