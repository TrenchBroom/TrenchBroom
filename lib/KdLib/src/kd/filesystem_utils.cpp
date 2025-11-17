/*
 Copyright 2025 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "kd/filesystem_utils.h"

#include <chrono>
#include <random>
#include <sstream>

namespace kdl
{

result<std::string, result_error> read_file(const std::filesystem::path& path)
{
  return with_istream(path, [](auto& is) {
    auto oss = std::ostringstream{};
    oss << is.rdbuf();
    return oss.str();
  });
}

tmp_file::tmp_file()
  : m_path{std::filesystem::temp_directory_path() / generateUniqueFileName()}
{
  // create an empty file
  auto ofs = std::ofstream{m_path};
}

tmp_file::~tmp_file()
{
  if (m_auto_remove)
  {
    // ignore any errors
    auto ec = std::error_code{};
    std::filesystem::remove(m_path, ec);
  }
}

const std::filesystem::path& tmp_file::path() const
{
  return m_path;
}

tmp_file::operator std::filesystem::path() const
{
  return m_path;
}

void tmp_file::set_auto_remove(const bool auto_remove)
{
  m_auto_remove = auto_remove;
}

std::string tmp_file::generateUniqueFileName()
{
  const auto now = std::chrono::system_clock::now();
  const auto nowTimeT = std::chrono::system_clock::to_time_t(now);
  const auto nowMs =
    std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
    % 1000;

  auto rd = std::random_device{};
  auto gen = std::mt19937{rd()};
  auto dis = std::uniform_int_distribution<>{0, 9999};
  const auto randomNum = dis(gen);

  auto ss = std::ostringstream{};
  ss << "kdl_tmp_file_" << nowTimeT << "_" << nowMs << "_" << randomNum;
  return ss.str();
}

} // namespace kdl
