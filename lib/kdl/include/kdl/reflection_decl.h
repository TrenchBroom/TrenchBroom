/*
 Copyright 2022 Kristian Duske

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

#include <array>
#include <iosfwd>
#include <string_view>
#include <tuple>

namespace kdl
{

namespace detail
{
constexpr size_t reflection_count_tokens(const std::string_view str)
{
  constexpr auto ws = std::string_view{" \n\t\r"};

  const auto s = str.find_first_not_of(ws);
  if (s == std::string_view::npos)
  {
    return 0;
  }

  auto c = 0u;
  auto has_non_ws = false;
  for (auto i = s; i < str.size(); ++i)
  {
    if (str[i] == ',')
    {
      if (has_non_ws)
      {
        ++c;
      }
      has_non_ws = false;
    }
    else if (ws.find_first_of(str[i]) == std::string_view::npos)
    {
      has_non_ws = true;
    }
  }

  return has_non_ws ? c + 1 : c;
}

template <size_t C>
constexpr auto reflection_split_tokens(std::string_view str)
{
  constexpr auto ws = std::string_view{" \n\t\r"};

  auto result = std::array<std::string_view, C>{};
  if constexpr (C == 0)
  {
    return result;
  }

  str = str.substr(str.find_first_not_of(ws));
  for (auto i = 0u; i < C - 1; ++i)
  {
    const auto c = str.find_first_of(",");
    const auto e = str.substr(0, c).find_last_not_of(ws);

    result[i] = str.substr(0, e + 1);
    str = str.substr(str.find_first_not_of(ws, c + 1));
  }

  auto e = str.find_last_not_of(ws);
  if (e == std::string_view::npos)
  {
    e = str.size() - 1;
  }
  result[C - 1] = str.substr(0, e + 1);
  return result;
}
} // namespace detail

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#endif

#define kdl_reflect_members_n(...)                                                       \
  template <size_t C = kdl::detail::reflection_count_tokens(#__VA_ARGS__)>               \
  [[maybe_unused]] constexpr static auto member_names()                                  \
  {                                                                                      \
    return kdl::detail::reflection_split_tokens<C>(#__VA_ARGS__);                        \
  }                                                                                      \
  [[maybe_unused]] auto members() const                                                  \
  {                                                                                      \
    return std::forward_as_tuple(__VA_ARGS__);                                           \
  }

#define kdl_reflect_members_0()                                                          \
  [[maybe_unused]] constexpr static auto member_names()                                  \
  {                                                                                      \
    return std::array<std::string_view, 0>{};                                            \
  }                                                                                      \
  [[maybe_unused]] auto members() const                                                  \
  {                                                                                      \
    return std::tuple<>{};                                                               \
  }

#define kdl_reflect_relational_operators(type_name)                                      \
  template <                                                                             \
    typename KDL_T,                                                                      \
    typename std::enable_if<std::is_same_v<type_name, KDL_T>, bool>::type = true>        \
  friend auto operator==(const KDL_T& lhs, const KDL_T& rhs)                             \
    ->decltype(lhs.members() == rhs.members())                                           \
  {                                                                                      \
    return lhs.members() == rhs.members();                                               \
  }                                                                                      \
  template <                                                                             \
    typename KDL_T,                                                                      \
    typename std::enable_if<std::is_same_v<type_name, KDL_T>, bool>::type = true>        \
  friend auto operator!=(const KDL_T& lhs, const KDL_T& rhs)                             \
    ->decltype(lhs.members() != rhs.members())                                           \
  {                                                                                      \
    return lhs.members() != rhs.members();                                               \
  }                                                                                      \
  template <                                                                             \
    typename KDL_T,                                                                      \
    typename std::enable_if<std::is_same_v<type_name, KDL_T>, bool>::type = true>        \
  friend auto operator<(const KDL_T& lhs, const KDL_T& rhs)                              \
    ->decltype(lhs.members() < rhs.members())                                            \
  {                                                                                      \
    return lhs.members() < rhs.members();                                                \
  }                                                                                      \
  template <                                                                             \
    typename KDL_T,                                                                      \
    typename std::enable_if<std::is_same_v<type_name, KDL_T>, bool>::type = true>        \
  friend auto operator<=(const KDL_T& lhs, const KDL_T& rhs)                             \
    ->decltype(lhs.members() <= rhs.members())                                           \
  {                                                                                      \
    return lhs.members() <= rhs.members();                                               \
  }                                                                                      \
  template <                                                                             \
    typename KDL_T,                                                                      \
    typename std::enable_if<std::is_same_v<type_name, KDL_T>, bool>::type = true>        \
  friend auto operator>(const KDL_T& lhs, const KDL_T& rhs)                              \
    ->decltype(lhs.members() > rhs.members())                                            \
  {                                                                                      \
    return lhs.members() > rhs.members();                                                \
  }                                                                                      \
  template <                                                                             \
    typename KDL_T,                                                                      \
    typename std::enable_if<std::is_same_v<type_name, KDL_T>, bool>::type = true>        \
  friend auto operator>=(const KDL_T& lhs, const KDL_T& rhs)                             \
    ->decltype(lhs.members() >= rhs.members())                                           \
  {                                                                                      \
    return lhs.members() >= rhs.members();                                               \
  }

#define kdl_stream_operator_decl(type_name)                                              \
  friend std::ostream& operator<<(std::ostream& lhs, const type_name& rhs);

#define kdl_reflect_decl(type_name, ...)                                                 \
  kdl_reflect_members_n(__VA_ARGS__) kdl_reflect_relational_operators(type_name)         \
    kdl_stream_operator_decl(type_name) struct dummy

#define kdl_reflect_decl_empty(type_name)                                                \
  kdl_reflect_members_0() kdl_reflect_relational_operators(type_name)                    \
    kdl_stream_operator_decl(type_name) struct dummy

#ifdef __clang__
#pragma clang diagnostic pop
#endif

} // namespace kdl
