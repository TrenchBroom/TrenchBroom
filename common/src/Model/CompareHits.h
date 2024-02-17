/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "FloatType.h"

#include "vecmath/util.h"

#include <memory>

namespace TrenchBroom
{
namespace Model
{
class Hit;

class CompareHits
{
public:
  virtual ~CompareHits();
  int compare(const Hit& lhs, const Hit& rhs) const;

private:
  virtual int doCompare(const Hit& lhs, const Hit& rhs) const = 0;
};

class CombineCompareHits : public CompareHits
{
private:
  std::unique_ptr<CompareHits> m_first;
  std::unique_ptr<CompareHits> m_second;

public:
  CombineCompareHits(
    std::unique_ptr<CompareHits> first, std::unique_ptr<CompareHits> second);

private:
  int doCompare(const Hit& lhs, const Hit& rhs) const override;
};

class CompareHitsByType : public CompareHits
{
private:
  int doCompare(const Hit& lhs, const Hit& rhs) const override;
};

class CompareHitsByDistance : public CompareHits
{
private:
  int doCompare(const Hit& lhs, const Hit& rhs) const override;
};

class CompareHitsBySize : public CompareHits
{
private:
  const vm::axis::type m_axis;
  CompareHitsByDistance m_compareByDistance;

public:
  CompareHitsBySize(vm::axis::type axis);

private:
  int doCompare(const Hit& lhs, const Hit& rhs) const override;
  FloatType getSize(const Hit& hit) const;
};
} // namespace Model
} // namespace TrenchBroom
