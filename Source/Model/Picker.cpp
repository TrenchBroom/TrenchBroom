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
#include "Model/Face.h"
#include "Model/MapObject.h"
#include "Model/Octree.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        Hit::Hit(HitType::Type type, const Vec3f& hitPoint, float distance) :
        m_type(type),
        m_hitPoint(hitPoint),
        m_distance(distance) {}
        
        Hit::~Hit() {}
        
        EntityHit::EntityHit(Entity& entity, const Vec3f& hitPoint, float distance) :
        Hit(HitType::EntityHit, hitPoint, distance),
        m_entity(entity) {}

        bool EntityHit::pickable(Filter& filter) const {
            return filter.entityPickable(m_entity);
        }

        FaceHit::FaceHit(HitType::Type type, Face& face, const Vec3f& hitPoint, float distance) :
        Hit(type, hitPoint, distance),
        m_face(face) {}

        FaceHit* FaceHit::faceHit(Face& face, const Vec3f& hitPoint, float distance) {
            return new FaceHit(HitType::FaceHit, face, hitPoint, distance);
        }
        
        FaceHit* FaceHit::nearFaceHit(Face& face, const Vec3f& hitPoint, float distance) {
            return new FaceHit(HitType::NearFaceHit, face, hitPoint, distance);
        }

        bool FaceHit::pickable(Filter& filter) const {
            return filter.brushPickable(*m_face.brush());
        }

        void PickResult::sortHits() {
            sort(m_hits.begin(), m_hits.end(), CompareHitsByDistance());
            m_sorted = true;
        }
        
        PickResult::~PickResult() {
            while(!m_hits.empty()) delete m_hits.back(), m_hits.pop_back();
        }

        void PickResult::add(Hit& hit) {
            m_hits.push_back(&hit);
        }

        Hit* PickResult::first(int typeMask, bool ignoreOccluders, Filter& filter) {
            if (!m_hits.empty()) {
                if (!m_sorted)
                    sortHits();
                if (!ignoreOccluders) {
                    unsigned int i = 0;
                    while (i < m_hits.size()) {
                        if (m_hits[i]->pickable(filter)) {
                            if (m_hits[i]->hasType(typeMask))
                                return m_hits[i];
                            break;
                        }
                        i++;
                    }

                    if (i < m_hits.size()) {
                        float closest = m_hits[i]->distance();
                        for (i = i + 1; i < m_hits.size() && m_hits[i]->distance() == closest; i++)
                            if (m_hits[i]->hasType(typeMask) && m_hits[i]->pickable(filter))
                                return m_hits[i];
                    }
                } else {
                    for (unsigned int i = 0; i < m_hits.size(); i++)
                        if (m_hits[i]->hasType(typeMask) && m_hits[i]->pickable(filter))
                            return m_hits[i];
                }
            }
            return NULL;
        }

        HitList PickResult::hits(int typeMask, Filter& filter) {
            HitList result;
            if (!m_sorted) sortHits();
            for (unsigned int i = 0; i < m_hits.size(); i++)
                if (m_hits[i]->hasType(typeMask) && m_hits[i]->pickable(filter))
                    result.push_back(m_hits[i]);
            return result;
        }

        HitList PickResult::hits(Filter& filter) {
            return hits(HitType::Any, filter);
        }

        Picker::Picker(Octree& octree) : m_octree(octree) {}

        PickResult* Picker::pick(const Ray& ray) {
            PickResult* pickResults = new PickResult();

            MapObjectList objects = m_octree.intersect(ray);
            for (unsigned int i = 0; i < objects.size(); i++)
                objects[i]->pick(ray, *pickResults);

            return pickResults;
        }

    }
}
