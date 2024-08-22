/*
 Copyright 2023 Kristian Duske

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

#include <functional>
#include <utility>

namespace kdl
{
/** Manage a generic resource.
 *
 * Takes the resource and a deleter that cleans up the resource. The deleter is called
 * for the resource upon construction unless the resource has been moved from or
 * release() was called.
 */
template <typename R>
class resource
{
  using Deleter = std::function<void(R&)>;

private:
  R m_resource;
  Deleter m_deleter;

public:
  /**
   * Creates a resource that wraps the given object and uses the given deleter.
   */
  resource(R resource, Deleter deleter)
    : m_resource{std::move(resource)}
    , m_deleter{std::move(deleter)}
  {
  }

  resource(const resource&) = delete;
  resource& operator=(const resource&) = delete;

  resource(resource&& other)
    : m_resource{std::move(other.m_resource)}
    , m_deleter{std::exchange(other.m_deleter, [](auto) {})}
  {
  }

  resource& operator=(resource&& other)
  {
    m_resource = std::move(other.m_resource);
    m_deleter = std::exchange(other.m_deleter, [](auto) {});
    return *this;
  }

  ~resource() { m_deleter(m_resource); }

  /**
   * Assign a new resource to this object. The current resource is destroyed before the
   * new resource is assigned.
   */
  resource& operator=(R resource)
  {
    m_deleter(m_resource);
    m_resource = std::move(resource);
    return *this;
  }

  // NOLINTNEXTLINE
  operator bool() const { return bool(m_resource); }

  R& operator*() { return m_resource; }
  const R& operator*() const { return m_resource; }

  R& operator->() { return m_resource; }
  const R& operator->() const { return m_resource; }

  R& get() { return m_resource; }
  const R& get() const { return m_resource; }

  /**
   * Return the resource. The resource will no longer be managed by this object. The
   * caller is responsible for any required cleanup.
   */
  R release()
  {
    m_deleter = [](auto) {};
    return std::move(m_resource);
  }
};

template <typename R, typename Deleter>
resource(R, Deleter) -> resource<R>;

} // namespace kdl
