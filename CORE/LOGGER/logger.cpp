#include "logger.h"
#include <fstream>
#include <string>

using namespace std;

LOGGER::LOGGER(bool uses_standard_output) {
  USES_STANDARD_OUTPUT = uses_standard_output;
  if (!USES_STANDARD_OUTPUT) {
    CREATE_LOG_FILE("./log.txt");
  }
}

void LOGGER::LOG(string field, string value) {
  if (!USES_STANDARD_OUTPUT) {
    LOG_FILE << field << ": " << value << endl;
  } else {
    cout << field << ": " << value << endl;
  }
}

void LOGGER::CREATE_LOG_FILE(string path) { LOG_FILE.open(path); }
