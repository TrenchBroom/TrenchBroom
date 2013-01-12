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

#ifndef __TrenchBroom__MapObject__
#define __TrenchBroom__MapObject__

#include "Model/EditState.h"
#include "Model/MapObjectTypes.h"
#include "Utility/VecMath.h"

#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Filter;
        class PickResult;
        
        class MapObject {
        private:
            unsigned int m_uniqueId;
            EditState::Type m_editState;
            bool m_previouslyLocked;
        public:
            enum Type {
                EntityObject,
                BrushObject
            };

            MapObject() :
            m_editState(EditState::Default),
            m_previouslyLocked(false) {
                static unsigned int currentId = 1;
                m_uniqueId = currentId++;
            }
            
            virtual ~MapObject() {
                m_uniqueId = 0;
                m_editState = EditState::Default;
            }
            
            template <typename T>
            static BBox bounds(const std::vector<T*>& objects) {
                assert(!objects.empty());
                
                typename std::vector<T*>::const_iterator it, end;
                it = objects.begin();
                end = objects.end();
                
                BBox result = (**it).bounds();
                while (++it != end) {
                    T& object = **it;
                    result.mergeWith(object.bounds());
                }
                return result;
            }
            
            template <typename T1, typename T2>
            static BBox bounds(const std::vector<T1*>& objects1, const std::vector<T2*>& objects2) {
                assert(!objects1.empty() || !objects2.empty());
                
                if (objects1.empty())
                    return bounds(objects2);
                if (objects2.empty())
                    return bounds(objects1);
                return bounds(objects1).mergeWith(bounds(objects2));
            }
            
            inline unsigned int uniqueId() const {
                return m_uniqueId;
            }
            
            inline EditState::Type editState() const {
                return m_editState;
            }
            
            virtual EditState::Type setEditState(EditState::Type editState) {
                EditState::Type previous = m_editState;
                
                if (m_previouslyLocked && editState == EditState::Default)
                    m_editState = EditState::Locked;
                else
                    m_editState = editState;

                if (previous == EditState::Locked && editState == EditState::Hidden) {
                    m_previouslyLocked = true;
                } else
                    m_previouslyLocked = false;

                return previous;
            }
            
            inline bool selected() const {
                return m_editState == EditState::Selected;
            }
            
            inline bool hidden() const {
                return m_editState == EditState::Hidden;
            }
            
            inline bool locked() const {
                return m_editState == EditState::Locked;
            }
            
            virtual inline bool hideable() const {
                return m_editState != EditState::Hidden;
            }
            
            virtual inline bool lockable() const {
                return m_editState != EditState::Locked && (m_editState == EditState::Default || m_editState == EditState::Selected);
            }
            
            virtual const Vec3f& center() const = 0;
            virtual const BBox& bounds() const = 0;
            virtual Type objectType() const = 0;
            
            virtual void translate(const Vec3f& delta, bool lockTextures) = 0;
            virtual void rotate90(Axis::Type axis, const Vec3f& center, bool clockwise, bool lockTextures) = 0;
            virtual void rotate(const Quat& rotation, const Vec3f& center, bool lockTextures) = 0;
            virtual void flip(Axis::Type axis, const Vec3f& center, bool lockTextures) = 0;
            
            virtual void pick(const Ray& ray, PickResult& pickResults) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__MapObject__) */
