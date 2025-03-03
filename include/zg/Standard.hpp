#pragma once
#include <atomic>
#include <chrono>
#include <codecvt>
#include <cstdint>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>
#include <zg/enums/EFileLocation.hpp>
#include <zg/interfaces/IFile.hpp>
#include <zg/queue.hpp>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <fcntl.h>
#include <mach-o/dyld.h>
#include <sys/event.h>
#include <sys/types.h>
#elif defined(__linux__)
#include <sys/inotify.h>
#endif
#include <zg/glm.hpp>
#if defined(__linux) || defined(__APPLE__)
#include <dlfcn.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif
// learn from a**<:._.:>
#define STANDARD std
/*
1 long jau awagitar
#define STANDARD zgg/*|om+t|*/

#define GLMATH glm
