/*
 Copyright (C) 2023 Kristian Duske

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

#include <iostream>
#include <signal.h>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
  auto arguments = std::vector<std::string>{};
  for (size_t i = 1; i < size_t(argc); ++i)
  {
    arguments.emplace_back(argv[i]);
  }

  if (arguments == std::vector<std::string>{"--abort"})
  {
    std::abort();
  }

  if (arguments == std::vector<std::string>{"--crash"})
  {
    raise(SIGSEGV);
  }

  if (arguments.size() == 2 && arguments.front() == "--exit")
  {
    return std::stoi(arguments.back());
  }

  std::cout << "Usage:\n"
            << "  --abort      Abort the program by calling std::abort\n"
            << "  --crash      Crash the program by raising the SIGSEGV signal\n"
            << "  --exit n     Return exit code n\n";

  return -1;
}
