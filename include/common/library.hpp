#pragma once
#if __unix__
#  include "common/unix/library.hpp"
#elif _WIN32
#  include "common/win32/library.hpp"
#endif
