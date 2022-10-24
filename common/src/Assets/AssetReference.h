/*
Copyright (C) 2020 Kristian Duske

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

#include <kdl/reflection_impl.h>

#include <utility>

#pragma once

namespace TrenchBroom
{
namespace Assets
{
template <typename T>
class AssetReference
{
private:
  T* m_asset;

public:
  explicit AssetReference(T* asset = nullptr)
    : m_asset(asset)
  {
    if (m_asset != nullptr)
    {
      m_asset->incUsageCount();
    }
  }

  AssetReference(const AssetReference& other) noexcept
    : AssetReference(other.m_asset)
  {
  }

  AssetReference(AssetReference&& other) noexcept
    : m_asset(std::exchange(other.m_asset, nullptr))
  {
  }

  ~AssetReference()
  {
    if (m_asset != nullptr)
    {
      m_asset->decUsageCount();
    }
  }

  AssetReference& operator=(AssetReference other) noexcept
  {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  friend void swap(AssetReference& lhs, AssetReference& rhs)
  {
    using std::swap;
    swap(lhs.m_asset, rhs.m_asset);
  }

  T* get() { return m_asset; }

  const T* get() const { return m_asset; }

  kdl_reflect_inline(AssetReference<T>, m_asset);
};
} // namespace Assets
} // namespace TrenchBroom
