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

#pragma once

#include "Result.h"
#include "StringMakers.h"

#include "kdl/overload.h"
#include "kdl/result.h"
#include "kdl/std_io.h"
#include "kdl/vector_utils.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <sstream>
#include <vector>

#include "Catch2.h"

namespace tb
{

template <typename T>
class PointerMatcher : public Catch::MatcherBase<const T*>
{
  T m_expected;

public:
  explicit PointerMatcher(T expected)
    : m_expected{std::move(expected)}
  {
  }

  bool match(const T* const& in) const override { return in && *in == m_expected; }

  std::string describe() const override
  {
    auto str = std::stringstream{};
    str << kdl::make_streamable(m_expected);
    return str.str();
  }
};

template <typename T>
auto MatchesPointer(T expected)
{
  return PointerMatcher<T>{std::move(expected)};
}

template <typename M, typename T, typename... E>
class ResultMatcher : public Catch::MatcherBase<kdl::result<T, E...>>
{
  M m_makeMatcher;
  kdl::result<T, E...> m_expected;

public:
  explicit ResultMatcher(M makeMatcher, kdl::result<T, E...> expected)
    : m_makeMatcher{std::move(makeMatcher)}
    , m_expected{std::move(expected)}
  {
  }

  bool match(const kdl::result<T, E...>& in) const override
  {
    return m_expected.visit(kdl::overload(
      [&](const T& lhs) {
        return in.visit(kdl::overload(
          [&](const T& rhs) { return m_makeMatcher(lhs).match(rhs); },
          [](const auto&) { return false; }));
      },
      (
        [&](const E& lhs) {
          return in.visit(kdl::overload(
            [&](const E& rhs) { return lhs == rhs; }, [](const auto&) { return false; }));
        },
        ...)));
  }

  std::string describe() const override
  {
    return m_expected.visit(kdl::overload(
      [&](const T& lhs) { return m_makeMatcher(lhs).describe(); },
      (
        [&](const E& lhs) {
          auto str = std::stringstream{};
          str << "matches error " << lhs;
          return str.str();
        },
        ...)));
  }
};

template <typename M, typename T, typename... E>
auto MatchesResult(M makeMatcher, kdl::result<T, E...> expected)
{
  return ResultMatcher<M, T, E...>{std::move(makeMatcher), std::move(expected)};
}

inline auto MatchesPathsResult(std::vector<std::filesystem::path> paths)
{
  return MatchesResult(
    [](auto&& x) { return Catch::UnorderedEquals(std::forward<decltype(x)>(x)); },
    Result<std::vector<std::filesystem::path>>{std::move(paths)});
}

template <typename T>
class AnyOfMatcher : public Catch::MatcherBase<T>
{
  std::vector<T> m_expected;

public:
  explicit AnyOfMatcher(std::vector<T> expected)
    : m_expected{std::move(expected)}
  {
  }

  bool match(const T& in) const override
  {
    return std::any_of(
      m_expected.begin(), m_expected.end(), [&](const auto& e) { return in == e; });
  }

  std::string describe() const override
  {
    auto str = std::stringstream{};
    str << "matches any of " << kdl::make_streamable(m_expected);
    return str.str();
  }
};

template <typename T>
AnyOfMatcher<T> MatchesAnyOf(std::vector<T> expected)
{
  return AnyOfMatcher<T>{std::move(expected)};
}

template <typename T>
AnyOfMatcher<T> MatchesAnyOf(std::initializer_list<T> expected)
{
  return AnyOfMatcher<T>(std::vector<T>{expected});
}

template <typename T>
class NoneOfMatcher : public Catch::MatcherBase<T>
{
  std::vector<T> m_expected;

public:
  explicit NoneOfMatcher(std::vector<T> expected)
    : m_expected{std::move(expected)}
  {
  }

  bool match(const T& in) const override
  {
    return std::none_of(
      m_expected.begin(), m_expected.end(), [&](const auto& e) { return in == e; });
  }

  std::string describe() const override
  {
    auto str = std::stringstream{};
    str << "matches none of " << kdl::make_streamable(m_expected);
    return str.str();
  }
};

template <typename T>
NoneOfMatcher<T> MatchesNoneOf(std::vector<T> expected)
{
  return NoneOfMatcher<T>{std::move(expected)};
}

template <typename T>
NoneOfMatcher<T> MatchesNoneOf(std::initializer_list<T> expected)
{
  return NoneOfMatcher<T>(std::vector<T>{expected});
}

template <typename T>
class AllDifferentMatcher : public Catch::MatcherBase<T>
{
public:
  bool match(const T& in) const override
  {
    return in.size() == kdl::vec_sort_and_remove_duplicates(in).size();
  }

  std::string describe() const override
  {
    auto str = std::stringstream{};
    str << "contains no duplicates";
    return str.str();
  }
};

template <typename T>
AllDifferentMatcher<T> AllDifferent()
{
  return AllDifferentMatcher<T>{};
}

class GlobMatcher : public Catch::MatcherBase<std::string>
{
private:
  std::string m_glob;

public:
  explicit GlobMatcher(std::string glob);
  bool match(const std::string& value) const override;
  std::string describe() const override;
};

GlobMatcher MatchesGlob(std::string glob);

/**
 * Catch2 matcher that compares two `std::vector`s of `vm::vec<T,S>`s,
 * ignoring order of the `std::vector`s, and checking equality of `vm::vec<T,S>`s with
 * an epsilon.
 */
template <typename T, std::size_t S>
class UnorderedApproxVecMatcher : public Catch::MatcherBase<std::vector<vm::vec<T, S>>>
{
private:
  std::vector<vm::vec<T, S>> m_expected;
  T m_epsilon;

public:
  explicit UnorderedApproxVecMatcher(
    const std::vector<vm::vec<T, S>>& expected, const T epsilon)
    : m_expected(expected)
    , m_epsilon(epsilon)
  {
  }

  bool match(const std::vector<vm::vec<T, S>>& actual) const override
  {
    if (actual.size() != m_expected.size())
    {
      return false;
    }

    for (auto& actualElement : actual)
    {
      bool foundMatch = false;

      for (size_t i = 0; i < m_expected.size(); ++i)
      {
        if (vm::is_equal(m_expected[i], actualElement, m_epsilon))
        {
          foundMatch = true;
          break;
        }
      }

      if (!foundMatch)
      {
        return false;
      }
    }
    return true;
  }

  std::string describe() const override
  {
    std::stringstream ss;
    ss << "approximatetly unordered matches vecs (";
    for (size_t i = 0; i < m_expected.size(); ++i)
    {
      ss << m_expected[i];
      if (i + 1 < m_expected.size())
      {
        ss << ", ";
      }
    }
    ss << ") with epsilon " << m_epsilon;
    return ss.str();
  }
};

template <typename T, std::size_t S>
UnorderedApproxVecMatcher<T, S> UnorderedApproxVecMatches(
  const std::vector<vm::vec<T, S>>& actual, const T epsilon)
{
  return UnorderedApproxVecMatcher(actual, epsilon);
}

} // namespace tb

namespace tb::mdl
{
class Node;

class NodeMatcher : public Catch::MatcherBase<Node>
{
  const Node& m_expected;

public:
  explicit NodeMatcher(const Node& expected);

  bool match(const Node& in) const override;

  std::string describe() const override;
};

NodeMatcher MatchesNode(const Node& expected);

class NodeVectorMatcher : public Catch::MatcherBase<const std::vector<Node*>&>
{
  const std::vector<Node*> m_expected;

public:
  explicit NodeVectorMatcher(std::vector<Node*> expected);

  bool match(const std::vector<Node*>& in) const override;

  std::string describe() const override;
};

NodeMatcher MatchesNodeVector(std::vector<Node*> expected);

} // namespace tb::mdl