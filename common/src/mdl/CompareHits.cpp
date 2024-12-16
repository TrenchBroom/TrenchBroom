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

#include "CompareHits.h"

#include "Ensure.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitAdapter.h"

#include "vm/util.h"

namespace tb::mdl
{
CompareHits::~CompareHits() = default;

int CompareHits::compare(const Hit& lhs, const Hit& rhs) const
{
  return doCompare(lhs, rhs);
}

CombineCompareHits::CombineCompareHits(
  std::unique_ptr<CompareHits> first, std::unique_ptr<CompareHits> second)
  : m_first(std::move(first))
  , m_second(std::move(second))
{
  ensure(m_first != nullptr, "first is null");
  ensure(m_second != nullptr, "second is null");
}

int CombineCompareHits::doCompare(const Hit& lhs, const Hit& rhs) const
{
  const auto firstResult = m_first->compare(lhs, rhs);
  return firstResult != 0 ? firstResult : m_second->compare(lhs, rhs);
}

int CompareHitsByType::doCompare(const Hit& lhs, const Hit& rhs) const
{
  return lhs.type() == BrushNode::BrushHitType   ? -1
         : rhs.type() == BrushNode::BrushHitType ? 1
                                                 : 0;
}

int CompareHitsByDistance::doCompare(const Hit& lhs, const Hit& rhs) const
{
  return lhs.distance() < rhs.distance() ? -1 : lhs.distance() > rhs.distance() ? 1 : 0;
}

CompareHitsBySize::CompareHitsBySize(const vm::axis::type axis)
  : m_axis{axis}
{
}

int CompareHitsBySize::doCompare(const Hit& lhs, const Hit& rhs) const
{
  const auto lhsSize = getSize(lhs);
  const auto rhsSize = getSize(rhs);
  return lhsSize < rhsSize   ? -1
         : lhsSize > rhsSize ? 1
                             : m_compareByDistance.compare(lhs, rhs);
}

double CompareHitsBySize::getSize(const Hit& hit) const
{
  if (const auto faceHandle = mdl::hitToFaceHandle(hit))
  {
    return faceHandle->face().projectedArea(m_axis);
  }
  if (const auto* node = hitToNode(hit))
  {
    return node->projectedArea(m_axis);
  }
  return 0.0;
}

} // namespace tb::mdl
