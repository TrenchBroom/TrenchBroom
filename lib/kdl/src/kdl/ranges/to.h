/*
 Copyright (C) 2025 Kristian Duske

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

#include <algorithm>
#include <ranges>

namespace kdl::ranges
{
namespace detail
{

//*! Determine if C::reserve exists and can be called with a size
template <typename C>
constexpr bool reservable_container =
  std::ranges::sized_range<C> && requires(C& c, std::ranges::range_size_t<C> n) {
    c.reserve(n);
    { c.capacity() } -> std::same_as<decltype(n)>;
    { c.max_size() } -> std::same_as<decltype(n)>;
  };

//! Is it possible to append to C via push_back or inserting at the end?
template <typename C, typename Reference>
constexpr bool container_appendable = requires(C& c, Reference&& ref) {
  requires(
    requires { c.emplace_back(std::forward<Reference>(ref)); }
    || requires { c.push_back(std::forward<Reference>(ref)); }
    || requires { c.emplace(c.end(), std::forward<Reference>(ref)); }
    || requires { c.insert(c.end(), std::forward<Reference>(ref)); });
};

//! Create a function that appends an element to the given container c.
template <typename C>
constexpr auto container_appender(C& c)
{
  return [&]<typename Reference>(Reference&& ref) {
    if constexpr (requires { c.emplace_back(std::declval<Reference>()); })
    {
      c.emplace_back(std::forward<Reference>(ref));
    }
    else if constexpr (requires { c.push_back(std::declval<Reference>()); })
    {
      c.push_back(std::forward<Reference>(ref));
    }
    else if constexpr (requires { c.emplace(c.end(), std::declval<Reference>()); })
    {
      c.emplace(c.end(), std::forward<Reference>(ref));
    }
    else
    {
      c.insert(c.end(), std::forward<Reference>(ref));
    }
  };
}

//! Used for type constexpr if conditions, see below
template <typename R>
struct dummy_iterator
{
  using iterator_category = std::input_iterator_tag;
  using value_type = std::ranges::range_value_t<R>;
  using difference_type = std::ptrdiff_t;
  using pointer = std::add_pointer_t<std::ranges::range_reference_t<R>>;
  using reference = std::ranges::range_reference_t<R>;
  reference operator*() const;
  pointer operator->() const;
  dummy_iterator& operator++();
  dummy_iterator operator++(int);
  bool operator==(const dummy_iterator&) const;
};

// True if std::iterator_traits<std::ranges::iterator_t<R>>::iterator_category
// is valid and denotes a type derived from std::input_iterator_tag
template <typename R>
concept input_iterator_range =
  requires {
    typename std::iterator_traits<std::ranges::iterator_t<R>>::iterator_category;
  }
  && std::derived_from<
    typename std::iterator_traits<std::ranges::iterator_t<R>>::iterator_category,
    std::input_iterator_tag>;

} // namespace detail

template <typename C, std::ranges::input_range R, typename... Args>
  requires(!std::ranges::view<C>)
constexpr C to(R&& r, Args&&... args)
{
  // see https://en.cppreference.com/w/cpp/ranges/to.html
  if constexpr (
    !std::ranges::input_range<C>
    || std::
      convertible_to<std::ranges::range_reference_t<R>, std::ranges::range_value_t<C>>)
  {
    if constexpr (std::constructible_from<C, R, Args...>)
    {
      return C(std::forward<R>(r), std::forward<Args>(args)...);
    }
    // std::from_range_t is only available in C++23
    // else if constexpr (std::constructible_from<C, std::from_range_t, R, Args...>)
    // {
    //   return C(std::from_range, std::forward<R>(r), std::forward<Args>(args)...);
    // }
    if constexpr (
      std::ranges::common_range<R> && detail::input_iterator_range<R>
      && std::constructible_from<
        C,
        std::ranges::iterator_t<R>,
        std::ranges::sentinel_t<R>,
        Args...>)
    {
      return C(std::ranges::begin(r), std::ranges::end(r), std::forward<Args>(args)...);
    }
    else
    {
      static_assert(std::constructible_from<C, Args...>);
      static_assert(detail::container_appendable<C, std::ranges::range_reference_t<R>>);

      auto c = C(std::forward<Args>(args)...);
      if constexpr (std::ranges::sized_range<R> && detail::reservable_container<C>)
      {
        c.reserve(static_cast<std::ranges::range_size_t<C>>(std::ranges::size(r)));
      }
      std::ranges::for_each(r, detail::container_appender(c));
      return c;
    }
  }
  else
  {
    return to<C>(
      std::ranges::ref_view(r) | std::views::transform([](auto&& elem) {
        return to<std::ranges::range_value_t<C>>(std::forward<decltype(elem)>(elem));
      }),
      std::forward<Args>(args)...);
  }
}

template <template <typename...> typename C, std::ranges::input_range R, typename... Args>
constexpr auto to(R&& r, Args&&... args)
{
  // This overload deduces the collection's type arguments from the value type of R's
  // elements
  if constexpr (requires { C(std::declval<R>(), std::declval<Args>()...); })
  {
    // By "calling" C(...), we can use C's deduction guides to deduce the value type,
    // and the full type of C is passed on the the previous overload of to using decltype
    return to<decltype(C(std::declval<R>(), std::declval<Args>()...))>(
      std::forward<R>(r), std::forward<Args>(args)...);
  }
  // std::from_range is only available in C++23
  // else if constexpr (requires {
  //                      C(std::from_range, std::declval<R>(), std::declval<Args>()...);
  //                    })
  // {
  //   return to<decltype(C(std::from_range, std::declval<R>(),
  //   std::declval<Args>()...))>(
  //     std::forward<R>(r), std::forward<Args>(args)...);
  // }
  else
  {
    static_assert(requires {
      C(std::declval<detail::dummy_iterator<R>>(),
        std::declval<detail::dummy_iterator<R>>(),
        std::declval<Args>()...);
    });

    return to<decltype(C(
      std::declval<detail::dummy_iterator<R>>(),
      std::declval<detail::dummy_iterator<R>>(),
      std::declval<Args>()...))>(std::forward<R>(r), std::forward<Args>(args)...);
  }
}

namespace detail
{

//! Helper struct for the case when the collection type isn't complete and we need to
//! deduce its type parameters
template <template <typename...> typename T>
struct wrap_c
{
};

//! Restores the fully specified type of the collection incl. the type arguments
template <typename C, typename R, typename... Args>
struct unwrap_c
{
  //! The fully specified type of the collection to pass to ranges::to
  using type = C;
};

//! Deduces the type parameters of C when C isn't complete
template <template <typename...> typename C, typename R, typename... Args>
struct unwrap_c<wrap_c<C>, R, Args...>
{
  //! The fully specified type of the collection to pass to ranges::to
  using type = std::remove_cvref_t<std::remove_pointer_t<decltype(C(
    std::declval<detail::dummy_iterator<R>>(),
    std::declval<detail::dummy_iterator<R>>(),
    std::declval<Args>()...))>>;
};

//! A helper struct that, when called, constructs an instance of C (with its type
//! parameters deduced if necessary and fills it with the values taken from the given
//! range.
template <typename C, typename... Args>
struct to_fn
{
  //! Deduce the full type of the collection instantiate; this also restores any missing
  //! type parameters when these were omitted (e.g. ranges::to<std::vector>())
  template <typename R>
  using Actual_C = typename unwrap_c<C, R, Args...>::type;

  template <typename R>
  auto operator()(R&& r)
  {
    return to<Actual_C<R>, R, Args...>(std::forward<R>(r), std::forward<Args>(args)...);
  }

  template <typename R>
  auto operator()(R&& r) const
  {
    return to<Actual_C<R>, R, Args...>(std::forward<R>(r), std::forward<Args>(args)...);
  }

  std::tuple<Args...> args;
};

template <std::ranges::range R, typename C, typename... Args>
auto operator|(R&& r, to_fn<C, Args...>&& fn)
{
  return fn(std::forward<R>(r));
}

template <std::ranges::range R, typename C, typename... Args>
auto operator|(R&& r, to_fn<C, Args...>& fn)
{
  return fn(std::forward<R>(r));
}

template <std::ranges::range R, typename C, typename... Args>
auto operator|(R&& r, const to_fn<C, Args...>& fn)
{
  return fn(std::forward<R>(r));
}

} // namespace detail

template <typename C, typename... Args>
  requires(!std::ranges::view<C>)
constexpr auto to(Args&&... args)
{
  return detail::to_fn<C, Args...>{std::forward<Args>(args)...};
}

template <template <typename...> typename C, typename... Args>
constexpr auto to(Args&&... args)
{
  return detail::to_fn<detail::wrap_c<C>, Args...>{std::forward<Args>(args)...};
}

} // namespace kdl::ranges
