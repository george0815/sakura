#include "../CORE/BUS/bus.h"
#include <string>

using namespace std;

namespace SAVE_MANAGER {

bool LOAD_SRAM(const string path, BUS *bus);

bool SAVE_SRAM(const string path, BUS *bus);

} // namespace SAVE_MANAGER
