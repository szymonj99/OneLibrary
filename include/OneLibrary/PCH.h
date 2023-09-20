#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <regex>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <exception>
#include <thread>

#ifdef _WIN32
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __attribute__((visibility("default")))
#endif