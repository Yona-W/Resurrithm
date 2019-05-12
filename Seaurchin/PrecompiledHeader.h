#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#define _ENABLE_ATOMIC_ALIGNMENT_FIX

//Windows
#include <Windows.h>
#include <Shlwapi.h>
#include <Imm.h>
#include <intrin.h>


//C Runtime
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cmath>

//C++ Standard
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <chrono>
#include <ios>
#include <map>
#include <utility>
#include <limits>
#include <unordered_set>
#include <unordered_map>
#include <forward_list>
#include <list>
#include <tuple>
#include <random>
#include <exception>
#include <future>
#include <thread>
#include <numeric>
#include <any>
#include <filesystem>
#include <regex>
#include <set>
#include <bitset>

#define SU_STATIC_ASSERT(expression, ...) static_assert(expression, "expression : \"" #expression "\" is invalid.\r\n" # __VA_ARGS__)
#ifdef _DEBUG
#define SU_ASSERT(expression) assert(expression)
#else
#define SU_ASSERT(...)
#endif

//Libraries
#include <DxLib.h>

#include <angelscript.h>
#include <scriptarray\scriptarray.h>
#include <scriptmath\scriptmath.h>
#include <scriptmath\scriptmathcomplex.h>
#include <scriptstdstring\scriptstdstring.h>
#include <scriptdictionary\scriptdictionary.h>
#include "wscriptbuilder.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>
#include <spdlog/sinks/wincolor_sink.h>
#include <spdlog/sinks/sink.h>

#include <toml/toml.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Crc32.h"
#include "Misc.h"
#include "Config.h"
