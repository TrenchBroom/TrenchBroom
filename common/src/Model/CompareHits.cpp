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

#include "CompareHits.h"

#include "Ensure.h"
#include "Model/Hit.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"

#include <vecmath/util.h>

namespace TrenchBroom {
    namespace Model {
        CompareHits::~CompareHits() {}

        int CompareHits::compare(const Hit& lhs, const Hit& rhs) const {
            return doCompare(lhs, rhs);
        }

        CombineCompareHits::CombineCompareHits(std::unique_ptr<CompareHits> first, std::unique_ptr<CompareHits> second) :
        m_first(std::move(first)),
        m_second(std::move(second)) {
            ensure(m_first != nullptr, "first is null");
            ensure(m_second != nullptr, "second is null");
        }

        int CombineCompareHits::doCompare(const Hit& lhs, const Hit& rhs) const {
            const int firstResult = m_first->compare(lhs, rhs);
            if (firstResult == 0)
                return m_second->compare(lhs, rhs);
            return firstResult;
        }

        int CompareHitsByType::doCompare(const Hit& lhs, const Hit& rhs) const {
            if (lhs.type() == BrushNode::BrushHit)
                return -1;
            if (rhs.type() == BrushNode::BrushHit)
                return 1;
            return 0;
        }

        int CompareHitsByDistance::doCompare(const Hit& lhs, const Hit& rhs) const {
            if (lhs.distance() < rhs.distance())
                return -1;
            if (lhs.distance() > rhs.distance())
                return 1;
            return 0;
        }

        CompareHitsBySize::CompareHitsBySize(const vm::axis::type axis) : m_axis(axis) {}

        int CompareHitsBySize::doCompare(const Hit& lhs, const Hit& rhs) const {
            const FloatType lhsSize = getSize(lhs);
            const FloatType rhsSize = getSize(rhs);
            if (lhsSize < rhsSize)
                return -1;
            if (lhsSize > rhsSize)
                return 1;
            return m_compareByDistance.compare(lhs, rhs);
        }

        FloatType CompareHitsBySize::getSize(const Hit& hit) const {
            const BrushFace* face = hitToFace(hit);
            if (face != nullptr)
                return face->area(m_axis);
            const Entity* entity = hitToEntity(hit);
            if (entity != nullptr)
                return entity->area(m_axis);
            return 0.0;
        }
    }
}
