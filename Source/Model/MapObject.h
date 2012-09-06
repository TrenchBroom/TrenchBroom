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
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Filter;
        class PickResult;
        
        class MapObject {
        private:
            int m_uniqueId;
            EditState::Type m_editState;
        public:
            enum Type {
                EntityObject,
                BrushObject
            };

            MapObject() {
                static int currentId = 1;
                m_uniqueId = currentId++;
                m_editState = EditState::Default;
            }
            
            virtual ~MapObject() {
                m_editState = EditState::Default;
            }
            
            inline int uniqueId() const {
                return m_uniqueId;
            }
            
            inline EditState::Type editState() const {
                return m_editState;
            }
            
            virtual EditState::Type setEditState(EditState::Type editState) {
                EditState::Type previous = m_editState;
                m_editState = editState;
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
                return m_editState != EditState::Hidden && (m_editState == EditState::Default || m_editState == EditState::Selected);
            }
            
            virtual inline bool lockable() const {
                return m_editState != EditState::Locked && (m_editState == EditState::Default || m_editState == EditState::Selected);
            }
            
            virtual const Vec3f& center() const = 0;
            virtual const BBox& bounds() const = 0;
            virtual Type objectType() const = 0;
            virtual void pick(const Ray& ray, PickResult& pickResults, Filter& filter) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__MapObject__) */
