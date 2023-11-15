// SPDX-FileCopyrightText: 2023 Christian Voigt
// SPDX-License-Identifier: MIT

#pragma once

// usage: DBG("HELLO WORLD");

#ifdef NDEBUG
#define DBG(x)
#else
#include <iostream>
#undef DBG
#define DBG(x) std::cout << x << "\n"
#endif
