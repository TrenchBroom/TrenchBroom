/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Octree__
#define __TrenchBroom__Octree__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Exceptions.h"

#include <vector>

namespace TrenchBroom {
    namespace Controller {
        template <typename T>
        class OctreeNode {
        private:
            typedef std::vector<T> List;
            
            BBox3f m_bounds;
            float m_minSize;
            OctreeNode* m_children[8];
            List m_objects;
        public:
            OctreeNode(const BBox3f& bounds, const float minSize) :
            m_bounds(bounds),
            m_minSize(minSize) {
                for (size_t i = 0; i < 8; ++i)
                    m_children[i] = NULL;
            }
            
            ~OctreeNode() {
                for (size_t i = 0; i < 8; ++i) {
                    if (m_children[i] != NULL) {
                        delete m_children[i];
                        m_children[i] = NULL;
                    }
                }
            }
            
            inline bool contains(const BBox3f& bounds) const {
                return m_bounds.contains(bounds);
            }
            
            inline bool containsObject(const BBox3f& bounds, T object) const {
                assert(contains(bounds));
                for (size_t i = 0; i < 8; ++i) {
                    if (m_children[i] != NULL && m_children[i]->contains(bounds)) {
                        return m_children[i]->containsObject(bounds, object);
                    }
                }
                
                typename List::const_iterator it = std::find(m_objects.begin(), m_objects.end(), object);
                return it != m_objects.end();
            }
            
            inline bool addObject(const BBox3f& bounds, T object) {
                assert(contains(bounds));
                
                const Vec3f size = m_bounds.size();
                if (size.x() > m_minSize || size.y() > m_minSize || size.z() > m_minSize) {
                    for (size_t i = 0; i < 8; ++i) {
                        if (m_children[i] != NULL && m_children[i]->contains(bounds)) {
                            return m_children[i]->addObject(bounds, object);
                        } else {
                            const BBox3f childBounds = octant(i);
                            if (childBounds.contains(bounds)) {
                                OctreeNode<T>* child = new OctreeNode<T>(childBounds, m_minSize);
                                try {
                                    const bool success = child->addObject(bounds, object);
                                    m_children[i] = child;
                                    return success;
                                } catch (...) {
                                    delete child;
                                    throw;
                                }
                            }
                        }
                    }
                }
                
                m_objects.push_back(object);
                return true;
            }
            
            inline bool removeObject(const BBox3f& bounds, T object) {
                assert(contains(bounds));
                for (size_t i = 0; i < 8; ++i) {
                    if (m_children[i] != NULL && m_children[i]->contains(bounds)) {
                        return m_children[i]->removeObject(bounds, object);
                    }
                }
                
                typename List::iterator it = std::find(m_objects.begin(), m_objects.end(), object);
                if (it == m_objects.end())
                    return false;
                m_objects.erase(it);
                return true;
            }

            inline void findObjects(const Ray3f& ray, List& result) const {
                const float distance = m_bounds.intersectWithRay(ray);
                if (Mathf::isnan(distance))
                    return;
                
                for (size_t i = 0; i < 8; ++i)
                    if (m_children[i] != NULL)
                        m_children[i]->findObjects(ray, result);
                result.insert(result.end(), m_objects.begin(), m_objects.end());
            }
        private:
            inline BBox3f octant(const size_t index) const {
                const Vec3f& min = m_bounds.min;
                const Vec3f& max = m_bounds.max;
                const Vec3f mid = (min + max) / 2.0f;
                switch (index) {
                    case 0: // xyz +++
                        return BBox3f(mid, max);
                    case 1: // xyz -++
                        return BBox3f(Vec3f(min.x(), mid.y(), mid.z()),
                                      Vec3f(mid.x(), max.y(), max.z()));
                    case 2: // xyz +-+
                        return BBox3f(Vec3f(mid.x(), min.y(), mid.z()),
                                      Vec3f(max.x(), mid.y(), max.z()));
                    case 3: // xyz --+
                        return BBox3f(Vec3f(min.x(), min.y(), mid.z()),
                                      Vec3f(mid.x(), mid.y(), max.z()));
                    case 4: // xyz ++-
                        return BBox3f(Vec3f(mid.x(), mid.y(), min.z()),
                                      Vec3f(max.x(), max.y(), mid.z()));
                    case 5: // xyz -+-
                        return BBox3f(Vec3f(min.x(), mid.y(), min.z()),
                                      Vec3f(mid.x(), max.y(), mid.z()));
                    case 6: // xyz +--
                        return BBox3f(Vec3f(mid.x(), min.y(), min.z()),
                                      Vec3f(max.x(), mid.y(), mid.z()));
                    case 7: // xyz ---
                        return BBox3f(Vec3f(min.x(), min.y(), min.z()),
                                      Vec3f(mid.x(), mid.y(), mid.z()));
                    default:
                        assert(false);
                        return BBox3f();
                }
            }
        };
        
        template <typename T>
        class Octree {
        public:
            typedef std::vector<T> List;
        private:
            BBox3f m_bounds;
            OctreeNode<T>* m_root;
        public:
            Octree(const BBox3f& bounds, const float minSize) :
            m_bounds(bounds),
            m_root(new OctreeNode<T>(bounds, minSize)) {}
            
            ~Octree() {
                delete m_root;
                m_root = NULL;
            }
            
            inline void addObject(const BBox3f& bounds, T object) {
                if (!m_root->contains(bounds))
                    throw OctreeException("Object is too large for this octree");
                if (!m_root->addObject(bounds, object))
                    throw OctreeException("Unknown error when inserting into octree");
            }
            
            inline void removeObject(const BBox3f& bounds, T object) {
                if (!m_root->contains(bounds))
                    throw OctreeException("Object is too large for this octree");
                if (!m_root->removeObject(bounds, object))
                    throw OctreeException("Cannot find object in octree");
            }
            
            inline bool containsObject(const BBox3f& bounds, T object) const {
                if (!m_root->contains(bounds))
                    return false;
                return m_root->containsObject(bounds, object);
            }
            
            inline List findObjects(const Ray3f& ray) const {
                List result;
                m_root->findObjects(ray, result);
                return result;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Octree__) */
