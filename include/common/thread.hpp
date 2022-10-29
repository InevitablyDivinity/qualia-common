#pragma once
#if __unix__
#include "common/unix/thread.hpp"
#elif _WIN32
#include "common/win32/thread.hpp"
#endif
