#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdio>

#include <consts.hpp>
#include <elements.hpp>

#ifndef GLOBAL_FUNCS
  #define GLOBAL_FUNCS

  extern void errorAndQuit(std::runtime_error *e);
  extern void assertRTE(bool statement, const char* message);  
  extern std::vector<std::string> splitString(std::string sourceString, char delimeter);
#endif