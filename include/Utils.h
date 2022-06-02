#ifndef UTILS_H
#define UTILS_H

#define DEBUG 0

#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
using namespace std;


void DisplayInfomation();
vector<string> ParseCmd(string cmd);
void PrintLog();
void Print(const char* title, const char* message);
bool IsDigit(string s);

#endif