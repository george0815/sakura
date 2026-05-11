#pragma once
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class LOGGER {

public:
  bool USES_STANDARD_OUTPUT = false;

  void LOG(string field, string value);

  LOGGER(bool uses_standard_output);

private:
  ofstream LOG_FILE;
  string OUTPUT_FILE_PATH = "./";
  void CREATE_LOG_FILE(string path);
};
