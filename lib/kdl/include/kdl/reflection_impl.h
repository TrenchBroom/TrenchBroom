/*
 Copyright 2022 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <kdl/reflection_decl.h>
#include <kdl/struct_io.h>

#include <ostream>

namespace kdl {
namespace detail {

template <size_t C, size_t I, typename... V>
void print_reflective_member(
  [[maybe_unused]] struct_stream& sstr,
  [[maybe_unused]] const std::array<std::string_view, C>& member_names,
  [[maybe_unused]] const std::tuple<V...>& member_values) {
  if constexpr (I < C) {
    sstr << member_names[I] << std::get<I>(member_values);
    print_reflective_member<C, I + 1, V...>(sstr, member_names, member_values);
  }
}

template <size_t C, typename... V>
void print_reflective(
  std::ostream& str, const std::string_view type_name,
  const std::array<std::string_view, C>& member_names, const std::tuple<V...>& member_values) {
  using kdl::detail::print_reflective_member;

  auto sstr = struct_stream{str};
  sstr << type_name;
  print_reflective_member<std::tuple_size_v<std::tuple<V...>>, 0, V...>(
    sstr, member_names, member_values);
}
} // namespace detail

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#endif

#define kdl_stream_operator_impl(type_name)                                                        \
  std::ostream& operator<<(std::ostream& lhs, const type_name& rhs) {                              \
    kdl::detail::print_reflective<std::tuple_size_v<decltype(rhs.members())>>(                     \
      lhs, #type_name, rhs.member_names(), rhs.members());                                         \
    return lhs;                                                                                    \
  }

#define kdl_reflect_impl(type_name) kdl_stream_operator_impl(type_name) struct dummy

#define kdl_reflect_inline(type_name, ...)                                                         \
  kdl_reflect_members_n(__VA_ARGS__) kdl_reflect_relational_operators(                             \
    type_name) friend kdl_stream_operator_impl(type_name) struct dummy

#define kdl_reflect_inline_empty(type_name)                                                        \
  kdl_reflect_members_0() kdl_reflect_relational_operators(                                        \
    type_name) friend kdl_stream_operator_impl(type_name) struct dummy

#ifdef __clang__
#pragma clang diagnostic pop
#endif

} // namespace kdl
