/*
 Copyright (C) 2026 Kristian Duske

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

#include "kd/contracts.h"

#include <array>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace kdl
{

/**
 * A pool of objects of type T, allocated in fixed-size pages instead of one at a time.
 *
 * Once inserted, an object never relocates: growing the pool only appends a new page, it
 * never reallocates the pages that are already in use. This means that references and
 * pointers to elements stay valid for as long as the element itself has not been erased,
 * even while further elements are being inserted into or erased from the pool.
 *
 * Erased slots are recycled via a free list threaded through the pool's own storage, so
 * inserting after erasing does not necessarily grow the pool.
 *
 * stable_pool does not track insertion order and does not support iteration; it is
 * intended to be used purely as a stable-address memory pool underneath a separate
 * structure -- such as an intrusive linked list -- that is responsible for ordering and
 * for knowing which elements are alive.
 *
 * @tparam T the type of the pooled objects
 * @tparam PageSize the number of objects stored in each page
 */
template <typename T, std::size_t PageSize = 128>
class stable_pool
{
  static_assert(PageSize > 0, "PageSize must be greater than 0");

private:
  union storage
  {
    T value;
    storage* next_free = nullptr;

    storage() {}
    ~storage() {}
  };

  struct slot
  {
    storage data;
    bool occupied = false;

    slot() = default;
    slot(const slot&) = delete;
    slot& operator=(const slot&) = delete;

    template <typename... Args>
    T& create(Args&&... args)
    {
      contract_pre(!occupied);

      new (&data.value) T(std::forward<Args>(args)...);
      occupied = true;
      return data.value;
    }

    storage* free(storage* next_free)
    {
      contract_pre(occupied);

      data.value.~T();
      occupied = false;
      data.next_free = next_free;
      return &data;
    }

    ~slot()
    {
      if (occupied)
      {
        data.value.~T();
      }
    }
  };

  using page = std::array<slot, PageSize>;

  std::vector<std::unique_ptr<page>> m_pages;
  storage* m_free_list = nullptr;
  std::size_t m_size = 0;

public:
  /**
   * Creates a new, empty pool. No pages are allocated until the first element is
   * inserted.
   */
  stable_pool() = default;

  stable_pool(const stable_pool&) = delete;
  stable_pool& operator=(const stable_pool&) = delete;

  /**
   * Moves the given pool into this pool. The given pool is left empty, ready to be
   * reused for further insertions.
   */
  stable_pool(stable_pool&& other) noexcept
    : m_pages{std::move(other.m_pages)}
    , m_free_list{std::exchange(other.m_free_list, nullptr)}
    , m_size{std::exchange(other.m_size, 0u)}
  {
  }

  /**
   * Moves the given pool into this pool. The given pool is left empty, ready to be
   * reused for further insertions.
   */
  stable_pool& operator=(stable_pool&& other) noexcept
  {
    auto tmp = stable_pool{std::move(other)};
    swap(*this, tmp);
    return *this;
  }

  ~stable_pool() = default;

  /**
   * Exchanges the contents of the two given pools.
   */
  friend void swap(stable_pool& lhs, stable_pool& rhs) noexcept
  {
    using std::swap;
    swap(lhs.m_pages, rhs.m_pages);
    swap(lhs.m_free_list, rhs.m_free_list);
    swap(lhs.m_size, rhs.m_size);
  }

  /**
   * Constructs a new object of type T in an available slot of this pool, using the given
   * arguments, and returns a reference to it. The returned reference remains valid until
   * the object is erased from this pool.
   *
   * @param args the arguments to forward to T's constructor
   * @return a reference to the newly constructed object
   */
  template <typename... Args>
  T& emplace(Args&&... args)
  {
    auto* s = acquire_slot();
    try
    {
      auto& t = s->create(std::forward<Args>(args)...);
      ++m_size;
      return t;
    }
    catch (...)
    {
      s->data.next_free = m_free_list;
      m_free_list = &s->data;
      throw;
    }
  }

  /**
   * Destroys the given object and returns its slot to this pool to be reused by a later
   * call to emplace. The given object must have been returned by a call to emplace on
   * this pool and must not already have been erased.
   *
   * @param value the object to erase, must belong to this pool
   */
  void erase(T& value)
  {
    auto* s = reinterpret_cast<slot*>(std::addressof(value));
    m_free_list = s->free(m_free_list);
    --m_size;
  }

  /**
   * Returns the number of objects currently alive in this pool.
   */
  std::size_t size() const { return m_size; }

  /**
   * Returns true if this pool does not currently contain any objects.
   */
  bool empty() const { return m_size == 0u; }

private:
  slot* acquire_slot()
  {
    if (m_free_list == nullptr)
    {
      add_page();
    }

    auto* result = reinterpret_cast<slot*>(m_free_list);
    m_free_list = m_free_list->next_free;
    return result;
  }

  void add_page()
  {
    auto new_page = std::make_unique<page>();
    for (std::size_t i = 0; i < PageSize; ++i)
    {
      (*new_page)[i].data.next_free =
        i + 1u < PageSize ? &(*new_page)[i + 1u].data : nullptr;
    }

    m_pages.push_back(std::move(new_page));
    m_free_list = &m_pages.back()->front().data;
  }
};

} // namespace kdl
