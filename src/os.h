#ifndef OS_H
#define OS_H

#include "shared.h"
#include <string>

std::string GetResDir(void);
std::string GetDataDir(void);
void CreateDirectory(const char * path);

#endif
