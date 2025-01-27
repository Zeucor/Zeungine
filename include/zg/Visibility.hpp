#pragma once
#ifdef _WIN32
#define ZG_VISIBILITY
#else
#define ZG_VISIBILITY __attribute__((visibility("default")))
#endif