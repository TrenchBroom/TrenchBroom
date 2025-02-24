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

#pragma once

#include "kdl/result.h"
#include "kdl/result_error.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

namespace kdl
{

template <typename Stream, typename F>
auto with_stream(
  const std::filesystem::path& path, const std::ios::openmode mode, const F& function)
  -> wrap_result_t<decltype(function(std::declval<Stream&>())), result_error>
{
  using namespace std::string_literals;

  using fn_result_type = decltype(function(std::declval<Stream&>()));
  using result_type = wrap_result_t<fn_result_type, result_error>;
  try
  {
    auto stream = Stream{path, mode};
    if (!stream)
    {
      return result_type{result_error{"Failed to open stream"}};
    }
    if constexpr (is_result_v<fn_result_type>)
    {
      if constexpr (std::is_same_v<typename fn_result_type::value_type, void>)
      {
        return function(stream) | and_then([]() { return result_type{}; });
      }
      else
      {
        return function(stream)
               | and_then([](auto x) { return result_type{std::move(x)}; });
      }
    }
    else if constexpr (std::is_same_v<typename result_type::value_type, void>)
    {
      function(stream);
      return result_type{};
    }
    else
    {
      return result_type{function(stream)};
    }
  }
  catch (const std::filesystem::filesystem_error& e)
  {
    return result_type{result_error{"Failed to open stream: "s + e.what()}};
  }
}

template <typename F>
auto with_istream(
  const std::filesystem::path& path, const std::ios::openmode mode, const F& function)
{
  return with_stream<std::ifstream>(path, mode, function);
}

template <typename F>
auto with_istream(const std::filesystem::path& path, const F& function)
{
  return with_stream<std::ifstream>(path, std::ios_base::in, function);
}

template <typename F>
auto with_ostream(
  const std::filesystem::path& path, const std::ios::openmode mode, const F& function)
{
  return with_stream<std::ofstream>(path, mode, function);
}

template <typename F>
auto with_ostream(const std::filesystem::path& path, const F& function)
{
  return with_stream<std::ofstream>(path, std::ios_base::out, function);
}

inline auto read_file(const std::filesystem::path& path)
{
  return with_istream(path, [](auto& is) {
    auto oss = std::ostringstream{};
    oss << is.rdbuf();
    return oss.str();
  });
}

class tmp_file
{
private:
  std::filesystem::path m_path;
  bool m_auto_remove = true;

public:
  tmp_file()
    : m_path(std::filesystem::temp_directory_path() / generateUniqueFileName())
  {
    // create an empty file
    auto ofs = std::ofstream{m_path};
  }

  ~tmp_file()
  {
    if (m_auto_remove)
    {
      // ignore any errors
      auto ec = std::error_code{};
      std::filesystem::remove(m_path, ec);
    }
  }

  const std::filesystem::path& path() const { return m_path; }

  // NOLINTNEXTLINE
  operator std::filesystem::path() const { return m_path; }

  void set_auto_remove(const bool auto_remove) { m_auto_remove = auto_remove; }

private:
  static std::string generateUniqueFileName()
  {
    const auto now = std::chrono::system_clock::now();
    const auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    const auto nowMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())
        .count()
      % 1000;

    auto rd = std::random_device{};
    auto gen = std::mt19937{rd()};
    auto dis = std::uniform_int_distribution<>{0, 9999};
    const auto randomNum = dis(gen);

    auto ss = std::ostringstream{};
    ss << "kdl_tmp_file_" << nowTimeT << "_" << nowMs << "_" << randomNum;
    return ss.str();
  }
};

} // namespace kdl
