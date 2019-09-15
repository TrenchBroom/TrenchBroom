/*
 Copyright (C) 2018 Eric Wasylishen

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRENCHBROOM_BENCHMARKUTILS_H
#define TRENCHBROOM_BENCHMARKUTILS_H

#include <chrono>
#include <string>

#ifdef __GNUC__
#define TB_NOINLINE __attribute__((noinline))
#else
#define TB_NOINLINE
#endif

// the noinline is so you can see the timeLambda when profiling
template<class L>
TB_NOINLINE static void timeLambda(L&& lambda, const std::string& message) {
    const auto start = std::chrono::high_resolution_clock::now();
    lambda();
    const auto end = std::chrono::high_resolution_clock::now();

    printf("Time elapsed for '%s': %fms\n", message.c_str(),
           std::chrono::duration<double>(end - start).count() * 1000.0);
}


#endif //TRENCHBROOM_BENCHMARKUTILS_H
