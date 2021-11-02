#ifndef __MAIN_H
#define __MAIN_H
#include <string>
#include "portaudio.h"

#ifdef HAS_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#else //HAS_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif //HAS_FILESYSTEM

extern std::string prefpath;
extern std::string pathsep;
extern std::string respath;
extern PaDeviceIndex sel_in_device;
extern PaDeviceIndex sel_out_device;
#endif
