/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Picker.h"
#include <cassert>
#include <algorithm>
#include "Model/Map/MapObject.h"
#include "Model/Octree.h"
#include "Utilities/Filter.h"

namespace TrenchBroom {
    namespace Model {
        Hit::Hit(void* object, EHitType type, const Vec3f& hitPoint, float distance) : object(object), index(-1), type(type), hitPoint(hitPoint), distance(distance) {
        }

        Hit::Hit(void* object, int index, EHitType type, const Vec3f& hitPoint, float distance) : object(object), index(index), type(type), hitPoint(hitPoint), distance(distance) {
        }

        bool Hit::hasType(int typeMask) {
            return (type & typeMask) != 0;
        }

        Entity& Hit::entity() {
            assert(type == TB_HT_ENTITY);
            return *(Entity*)object;
        }

        Brush& Hit::brush() {
            assert(type == TB_HT_VERTEX_HANDLE || type == TB_HT_EDGE_HANDLE || type == TB_HT_FACE_HANDLE);
            return *(Brush*)object;
        }

        Face& Hit::face() {
            assert(type == TB_HT_FACE || type == TB_HT_CLOSE_FACE);
            return *(Face*)object;
        }

        bool HitList::compareHits(const Hit* left, const Hit* right) {
            return left->distance < right->distance;
        }

        void HitList::sortHits() {
            sort(m_hits.begin(), m_hits.end(), HitList::compareHits);
            m_sorted = true;
        }

        HitList::~HitList() {
            while(!m_hits.empty()) delete m_hits.back(), m_hits.pop_back();
        }

        void HitList::add(Hit& hit) {
            m_hits.push_back(&hit);
        }

        Hit* HitList::first(int typeMask, bool ignoreOccluders) {
            if (!m_hits.empty()) {
                if (!m_sorted) sortHits();
                if (!ignoreOccluders) {
                    if (m_hits[0]->hasType(typeMask)) return m_hits[0];

                    float closest = m_hits[0]->distance;
                    for (int i = 1; i < m_hits.size() && m_hits[i]->distance == closest; i++)
                        if (m_hits[i]->hasType(typeMask)) return m_hits[i];
                } else {
                    for (int i = 0; i < m_hits.size(); i++)
                        if (m_hits[i]->hasType(typeMask)) return m_hits[i];
                }
            }
            return NULL;
        }

        vector<Hit*> HitList::hits(int typeMask) {
            vector<Hit*> result;
            if (!m_sorted) sortHits();
            for (int i = 0; i < m_hits.size(); i++)
                if (m_hits[i]->hasType(typeMask))
                    result.push_back(m_hits[i]);
            return result;
        }

        const vector<Hit*>& HitList::hits() {
            if (!m_sorted) sortHits();
            return m_hits;
        }

        Picker::Picker(Octree& octree) : m_octree(octree) {
        }

        HitList* Picker::pick(const Ray& ray, Filter& filter) {
            HitList* hits = new HitList();

            vector<MapObject*> objects = m_octree.intersect(ray);
            for (int i = 0; i < objects.size(); i++)
                objects[i]->pick(ray, *hits);

            return hits;
        }

    }
}
