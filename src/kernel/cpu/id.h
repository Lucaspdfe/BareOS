#pragma once
#include <stdbool.h>

bool __attribute__((cdecl)) ID_CheckSupported();
bool ID_APICSupport();
bool ID_CPUName(char* brand);
