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

#ifndef TrenchBroom_Filter_h
#define TrenchBroom_Filter_h

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Face.h"
#include "Utility/String.h"

namespace TrenchBroom {
    namespace Model {
        class Filter {
        public:
            inline const String& pattern() const {
                return m_pattern;
            }
            
            inline void setPattern(const String& pattern) {
                m_pattern = Utility::trim(pattern);
            }
            
            inline bool showEntities() const {
                return m_showEntities;
            }
            
            inline void setShowEntities(bool showEntities) {
                m_showEntities = showEntities;
            }
            
            inline bool showBrushes() const {
                return m_showBrushes;
            }
            
            inline void setShowBrushes(bool showBrushes) {
                m_showBrushes = showBrushes;
            }
            
            virtual inline bool entityVisible(const Model::Entity& entity) const {
                EntityDefinition* definition = entity.definition();
                if (definition != NULL && definition->type() == EntityDefinition::PointEntity && m_showEntities == false)
                    return false;
                
                if (entity.hidden() || entity.worldspawn())
                    return false;
                
                if (!m_pattern.empty()) {
                    const Model::Properties& properties = entity.properties();
                    Model::Properties::const_iterator it, end;
                    for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                        const PropertyKey& key = it->first;
                        const PropertyValue& value = it->second;
                        if (Utility::containsString(key, m_pattern, false) ||
                            Utility::containsString(value, m_pattern, false))
                            return true;
                    }
                    return false;
                }
                
                return true;
            }
            
            virtual inline bool entityPickable(const Model::Entity& entity) const {
                if (entity.worldspawn() || !entityVisible(entity) || entity.locked())
                    return false;
                
                EntityDefinition* definition = entity.definition();
                if (definition != NULL && definition->type() == EntityDefinition::BrushEntity && !entity.brushes().empty())
                    return false;
                
                return true;
            }

            virtual inline bool brushVisible(const Model::Brush& brush) const {
                if (!m_showBrushes || brush.hidden())
                    return false;
                
                if (!m_pattern.empty()) {
                    const Model::FaceList& faces = brush.faces();
                    for (unsigned int i = 0; i < faces.size(); i++)
                        if (Utility::containsString(faces[i]->textureName(), m_pattern, false))
                            return true;
                    return false;
                }
                
                return true;
            }
            
            virtual inline bool brushPickable(const Model::Brush& brush) const {
                if (!brushVisible(brush) || brush.locked() || brush.entity()->locked())
                    return false;
                
                return true;
            }
            
            virtual inline bool brushVerticesPickable(const Model::Brush& brush) const {
                if (!brushPickable(brush))
                    return false;
                
                return true;
            }
        };
    }
}

#endif
