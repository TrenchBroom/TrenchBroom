/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_Octree
#define TrenchBroom_Octree

#include "CollectionUtils.h"
#include "Macros.h"
#include "VecMath.h"
#include "Exceptions.h"
#include "SharedPointer.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        template <typename F, typename T>
        class OctreeNode {
        private:
            typedef std::vector<T> List;
            
            BBox<F,3> m_bounds;
            F m_minSize;
            OctreeNode* m_parent;
            OctreeNode* m_children[8];
            List m_objects;
        public:
            OctreeNode(const BBox<F,3>& bounds, const F minSize, OctreeNode* parent) :
            m_bounds(bounds),
            m_minSize(minSize),
            m_parent(parent) {
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
            
            bool contains(const BBox<F,3>& bounds) const {
                return m_bounds.contains(bounds);
            }
            
            bool containsObject(const BBox<F,3>& bounds, T object) const {
                assert(contains(bounds));
                for (size_t i = 0; i < 8; ++i) {
                    if (m_children[i] != NULL && m_children[i]->contains(bounds)) {
                        return m_children[i]->containsObject(bounds, object);
                    }
                }
                
                typename List::const_iterator it = std::find(m_objects.begin(), m_objects.end(), object);
                return it != m_objects.end();
            }
            
            OctreeNode* addObject(const BBox<F,3>& bounds, T object) {
                assert(contains(bounds));
                
                const Vec3f size = m_bounds.size();
                if (size.x() > m_minSize || size.y() > m_minSize || size.z() > m_minSize) {
                    for (size_t i = 0; i < 8; ++i) {
                        if (m_children[i] != NULL && m_children[i]->contains(bounds)) {
                            return m_children[i]->addObject(bounds, object);
                        } else {
                            const BBox<F,3> childBounds = octant(i);
                            if (childBounds.contains(bounds)) {
                                OctreeNode<F,T>* child = new OctreeNode<F,T>(childBounds, m_minSize, this);
                                try {
                                    OctreeNode* result = child->addObject(bounds, object);
                                    m_children[i] = child;
                                    return result;
                                } catch (...) {
                                    delete child;
                                    throw;
                                }
                            }
                        }
                    }
                }
                
                assert(!VectorUtils::contains(m_objects, object));
                m_objects.push_back(object);
                return this;
            }
            
            bool removeObject(T object) {
                typename List::iterator it = std::find(m_objects.begin(), m_objects.end(), object);
                if (it == m_objects.end())
                    return false;
                m_objects.erase(it);
                return true;
            }

            OctreeNode* findContaining(const BBox<F,3>& bounds) {
                if (contains(bounds))
                    return this;
                if (m_parent == NULL)
                    return NULL;
                return m_parent->findContaining(bounds);
            }
            
            void findObjects(const Ray<F,3>& ray, List& result) const {
                const F distance = m_bounds.intersectWithRay(ray);
                if (Math::isnan(distance))
                    return;
                
                for (size_t i = 0; i < 8; ++i)
                    if (m_children[i] != NULL)
                        m_children[i]->findObjects(ray, result);
                result.insert(result.end(), m_objects.begin(), m_objects.end());
            }
        private:
            BBox<F,3> octant(const size_t index) const {
                const Vec3f& min = m_bounds.min;
                const Vec3f& max = m_bounds.max;
                const Vec3f mid = (min + max) / 2.0f;
                switch (index) {
                    case 0: // xyz +++
                        return BBox<F,3>(mid, max);
                    case 1: // xyz -++
                        return BBox<F,3>(Vec3f(min.x(), mid.y(), mid.z()),
                                      Vec3f(mid.x(), max.y(), max.z()));
                    case 2: // xyz +-+
                        return BBox<F,3>(Vec3f(mid.x(), min.y(), mid.z()),
                                      Vec3f(max.x(), mid.y(), max.z()));
                    case 3: // xyz --+
                        return BBox<F,3>(Vec3f(min.x(), min.y(), mid.z()),
                                      Vec3f(mid.x(), mid.y(), max.z()));
                    case 4: // xyz ++-
                        return BBox<F,3>(Vec3f(mid.x(), mid.y(), min.z()),
                                      Vec3f(max.x(), max.y(), mid.z()));
                    case 5: // xyz -+-
                        return BBox<F,3>(Vec3f(min.x(), mid.y(), min.z()),
                                      Vec3f(mid.x(), max.y(), mid.z()));
                    case 6: // xyz +--
                        return BBox<F,3>(Vec3f(mid.x(), min.y(), min.z()),
                                      Vec3f(max.x(), mid.y(), mid.z()));
                    case 7: // xyz ---
                        return BBox<F,3>(Vec3f(min.x(), min.y(), min.z()),
                                      Vec3f(mid.x(), mid.y(), mid.z()));
                    default:
                        assert(false);
                        return BBox<F,3>();
                }
            }
        };
        
        template <typename F, typename T>
        class Octree {
        public:
            typedef std::vector<T> List;
        private:
            typedef std::map<T, OctreeNode<F,T>*> ObjectMap;
            BBox<F,3> m_bounds;
            OctreeNode<F,T>* m_root;
            ObjectMap m_objectMap;
        public:
            Octree(const BBox<F,3>& bounds, const F minSize) :
            m_bounds(bounds),
            m_root(new OctreeNode<F,T>(bounds, minSize, NULL)) {}
            
            ~Octree() {
                delete m_root;
            }
            
            const BBox<F,3>& bounds() const {
                return m_bounds;
            }
            
            void addObject(const BBox<F,3>& bounds, T object) {
                if (!m_root->contains(bounds))
                    throw OctreeException("Object is too large for this octree");
                
                OctreeNode<F,T>* node = m_root->addObject(bounds, object);
                if (node == NULL)
                    throw OctreeException("Unknown error when inserting into octree");
                assertResult(MapUtils::insertOrFail(m_objectMap, object, node));
            }
            
            void removeObject(T object) {
                typename ObjectMap::iterator it = m_objectMap.find(object);
                if (it == m_objectMap.end())
                    throw OctreeException("Cannot find object in octree");
                
                OctreeNode<F,T>* node = it->second;
                if (!node->removeObject(object))
                    throw OctreeException("Cannot find object in octree");
                m_objectMap.erase(it);
            }
            
            void updateObject(const BBox<F,3>& bounds, T object) {
                typename ObjectMap::iterator it = m_objectMap.find(object);
                if (it == m_objectMap.end())
                    throw OctreeException("Cannot find object in octree");

                OctreeNode<F,T>* oldNode = it->second;
                if (!oldNode->removeObject(object))
                    throw OctreeException("Cannot find object in octree");
                
                OctreeNode<F,T>* newAncestor = oldNode->findContaining(bounds);
                if (newAncestor == NULL)
                    throw OctreeException("Cannot find new ancestor node in octree");
                OctreeNode<F,T>* newParent = newAncestor->addObject(bounds, object);
                if (newParent == NULL)
                    throw OctreeException("Unknown error when inserting into octree");
                MapUtils::insertOrReplace(m_objectMap, object, newParent);
            }
            
            bool containsObject(const BBox<F,3>& bounds, T object) const {
                if (!m_root->contains(bounds))
                    return false;
                return m_root->containsObject(bounds, object);
            }
            
            List findObjects(const Ray<F,3>& ray) const {
                List result;
                m_root->findObjects(ray, result);
                return result;
            }
        };
    }
}

#endif /* defined(TrenchBroom_Octree) */
