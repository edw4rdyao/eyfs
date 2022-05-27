#include "Eyfs.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

int main() {
  DisplayInfomation();
  Eyfs *eyfs = new Eyfs();
  eyfs->Run();
  delete eyfs;
  return 0;
}