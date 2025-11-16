/*
 Copyright (C) 2010 Kristian Duske

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

#include "ColorComponentType.h"
#include "Result.h"

#include "kdl/optional_utils.h"
#include "kdl/reflection_impl.h"
#include "kdl/string_utils.h"
#include "kdl/tuple_utils.h"

#include "vm/vec.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ranges>
#include <string_view>
#include <type_traits>

namespace tb
{

template <AnyColorComponentType... ComponentTypes>
  requires ComponentsAreUnique<ComponentTypes...>
class ColorT;

namespace detail
{

template <AnyColorComponentType ComponentType>
struct ComponentValue
{
  using Type = ComponentType;
  using ValueType = Type::ValueType;

  ValueType value;

  constexpr ComponentValue()
    : value{Type::defaultValue()}
  {
  }

  template <typename U>
  constexpr explicit ComponentValue(const U value_)
    : value{ValueType(value_)}
  {
  }

  static constexpr ComponentValue fromNormalizedValue(
    const Type::NormalizedValueType value)
  {
    return ComponentValue{Type::fromNormalizedValue(value)};
  }

  static std::optional<ComponentValue> parse(const std::string_view str)
  {
    return Type::parse(str)
           | kdl::optional_transform([](const auto& v) { return ComponentValue{v}; });
  }

  constexpr Type::NormalizedValueType normalize() const
  {
    return Type::normalizeValue(value);
  }

  kdl_reflect_inline(ComponentValue, value);
};

template <
  const ColorChannel C,
  AnyColorComponentType First,
  AnyColorComponentType... Rest>
constexpr size_t componentIndex()
{
  if constexpr (First::Channel == C)
  {
    return 0;
  }
  else
  {
    static_assert(sizeof...(Rest) > 0, "C not found");
    return componentIndex<C, Rest...>() + 1;
  }
}

template <typename... ComponentValues>
constexpr auto normalizedValues(const std::tuple<ComponentValues...>& values)
{
  return std::apply(
    [](const auto&... value) { return std::tuple{value.normalize()...}; }, values);
}

template <typename... ComponentTypes>
concept AllValueTypesEqual =
  (std::is_same_v<
     typename ComponentTypes::ValueType,
     typename std::tuple_element_t<0, std::tuple<ComponentTypes...>>::ValueType>
   && ...);

template <typename... ComponentValues>
  requires(AllValueTypesEqual<typename ComponentValues::Type...>)
constexpr auto componentVector(const std::tuple<ComponentValues...>& values)
{
  using T = std::tuple_element_t<0, std::tuple<ComponentValues...>>::ValueType;
  constexpr auto S = sizeof...(ComponentValues);

  return std::apply(
    [](const auto&... value) { return vm::vec<T, S>{value.value...}; }, values);
}

template <typename... ComponentTypes, typename... U>
constexpr auto fromValues(const std::tuple<U...>& values)
{
  using Tuple = std::tuple<ComponentValue<ComponentTypes>...>;

  if constexpr (sizeof...(U) != sizeof...(ComponentTypes))
  {
    return std::optional<Tuple>{};
  }
  else if constexpr (sizeof...(ComponentTypes) == 0)
  {
    return Tuple{};
  }
  else
  {
    return std::apply(
      [](const auto&... value) {
        return (ComponentTypes::inValueRange(value) && ...)
                 ? std::optional{Tuple{ComponentValue<ComponentTypes>{value}...}}
                 : std::nullopt;
      },
      values);
  }
}

template <typename... ComponentTypes>
constexpr auto fromNormalizedValues(
  const std::tuple<typename ComponentTypes::NormalizedValueType...>& values)
{
  return std::apply(
    [](const auto&... value) {
      return std::tuple{ComponentValue<ComponentTypes>::fromNormalizedValue(value)...};
    },
    values);
}

template <typename... ComponentTypes>
constexpr auto defaultValues()
{
  return std::tuple{ComponentValue<ComponentTypes>{}...};
}

template <typename... ComponentTypes, std::ranges::random_access_range R, size_t... I>
constexpr auto parseComponentValuesImpl(const R& strs, std::index_sequence<I...>)
{
  static_assert(((I < sizeof...(ComponentTypes)) && ...));

  using Tuple = std::tuple<ComponentTypes...>;
  const auto maybeValues =
    std::tuple{ComponentValue<std::tuple_element_t<I, Tuple>>::parse(strs[I])...};

  // fold std::tuple<std::optional<T>, std::optional<U>, ...>
  // into std::optional<std::tuple<T, U, ...>> if all optionals have a value
  return std::apply(
    [](const auto&... values) {
      return (values.has_value() && ...) ? std::optional{std::tuple{*values...}}
                                         : std::nullopt;
    },
    maybeValues);
}

template <typename... ComponentTypes, std::ranges::random_access_range R>
constexpr auto parseComponentValues(const R& strs)
{
  return strs.size() == sizeof...(ComponentTypes)
           ? parseComponentValuesImpl<ComponentTypes...>(
               strs, std::make_index_sequence<sizeof...(ComponentTypes)>{})
           : std::nullopt;
}

template <typename T>
struct IsColorT : std::false_type
{
};

template <AnyColorComponentType... ComponentTypes>
struct IsColorT<ColorT<ComponentTypes...>> : std::true_type
{
};

} // namespace detail

template <typename T>
concept AnyColorT = detail::IsColorT<T>::value;

template <AnyColorComponentType... ComponentTypes>
  requires ComponentsAreUnique<ComponentTypes...>
class ColorT
{
public:
  static constexpr auto NumComponents = sizeof...(ComponentTypes);
  using ComponentTuple = std::tuple<detail::ComponentValue<ComponentTypes>...>;

private:
  ComponentTuple m_components;

public:
  constexpr static auto defaultValues()
  {
    return detail::defaultValues<ComponentTypes...>();
  }

  constexpr ColorT()
    : m_components{defaultValues()}
  {
  }

  constexpr explicit ColorT(typename ComponentTypes::ValueType... v)
    : m_components{v...}
  {
  }

  constexpr explicit ColorT(ComponentTuple components_)
    : m_components{std::move(components_)}
  {
  }

  template <AnyColorT Color, typename... Values>
  constexpr explicit ColorT(const Color& color, const Values... tailValues)
    : m_components{std::apply(
        [](const auto... allValues) {
          return std::tuple{detail::ComponentValue<ComponentTypes>{allValues}...};
        },
        std::tuple_cat(color.values(), std::tuple{tailValues...}))}
  {
  }

  template <typename U>
  static Result<ColorT> fromVec(const vm::vec<U, NumComponents>& vec)
    requires(detail::AllValueTypesEqual<ComponentTypes...>)
  {
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
      return fromValues(std::tuple{vec[I]...});
    }(std::make_index_sequence<NumComponents>{});
  }

  static Result<ColorT> fromValues(const auto& values)
  {
    if (const auto componentValues = detail::fromValues<ComponentTypes...>(values))
    {
      return ColorT{*componentValues};
    }
    return Error{
      fmt::format("Failed to create color from values {}", fmt::join(values, ", "))};
  }

  constexpr static auto fromNormalizedValues(const auto& values)
  {
    return ColorT{detail::fromNormalizedValues<ComponentTypes...>(values)};
  }

  template <std::ranges::random_access_range R>
  static Result<ColorT> parseComponents(const R& components)
  {
    if (
      const auto result = detail::parseComponentValues<ComponentTypes...>(
        components | std::views::take(NumComponents)))
    {
      return ColorT{*result};
    }

    return Error{
      fmt::format("Failed to parse '{}' as color", fmt::join(components, " "))};
  }

  static Result<ColorT> parse(const std::string_view str)
  {
    return parseComponents(kdl::str_split(str, " "));
  }

  constexpr static size_t numComponents() { return NumComponents; }

  constexpr const auto& components() const { return m_components; }

  constexpr auto values() const
  {
    return std::apply(
      [](const auto&... component) { return std::tuple{component.value...}; },
      m_components);
  }

  template <ColorChannel C>
  constexpr auto get() const
  {
    static_assert(((ComponentTypes::Channel == C) || ...));
    return std::get<detail::componentIndex<C, ComponentTypes...>()>(m_components).value;
  }

  constexpr auto toVec() const
    requires(detail::AllValueTypesEqual<ComponentTypes...>)
  {
    return detail::componentVector(m_components);
  }

  template <AnyColorT ToColor>
  constexpr ToColor to() const
  {
    const auto normalizedValues = detail::normalizedValues(m_components);

    if constexpr (ToColor::NumComponents > NumComponents)
    {
      constexpr auto NumDefaultComponents = ToColor::NumComponents - NumComponents;
      static_assert(NumComponents + NumDefaultComponents == ToColor::NumComponents);

      const auto defaultValues =
        kdl::tup_suffix<NumDefaultComponents>(ToColor::defaultValues());
      const auto normalizedDefaultValues = detail::normalizedValues(defaultValues);

      return ToColor::fromNormalizedValues(
        std::tuple_cat(normalizedValues, normalizedDefaultValues));
    }
    else
    {
      return ToColor::fromNormalizedValues(
        kdl::tup_prefix<ToColor::NumComponents>(normalizedValues));
    }
  }

  std::string toString() const { return fmt::format("{}", fmt::join(values(), " ")); }

  kdl_reflect_inline(ColorT, m_components);
};

} // namespace tb
