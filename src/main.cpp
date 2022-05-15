#include "Eyfs.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main() {
  DisplayInfomation();
  Eyfs *eyfs = new Eyfs();
  eyfs->Run();
  delete eyfs;
  return 0;
}