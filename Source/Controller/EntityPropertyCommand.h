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

#ifndef __TrenchBroom__EntityPropertyCommand__
#define __TrenchBroom__EntityPropertyCommand__

#include "Controller/SnapshotCommand.h"
#include "Controller/ObjectsCommand.h"
#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"

#include <wx/wx.h>

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        class EntityPropertyCommand : public SnapshotCommand, ObjectsCommand {
        protected:
            const Model::EntityList m_entities;
            Model::PropertyKeyList m_keys;
            Model::PropertyKey m_newKey;
            Model::PropertyValue m_newValue;
            bool m_definitionChanged;
            bool m_force;
            
            inline const Model::PropertyKey& key() const {
                assert(m_keys.size() == 1);
                return m_keys.back();
            }
            
            inline void setKey(const Model::PropertyKey& key) {
                assert(m_keys.size() <= 1);
                if (m_keys.empty())
                    m_keys.push_back(key);
                else
                    m_keys[0] = key;
            }
            
            inline const Model::PropertyKeyList& keys() const {
                return m_keys;
            }
            
            inline void setKeys(const Model::PropertyKeyList&keys) {
                m_keys = keys;
            }
            
            inline const Model::PropertyKey& newKey() const {
                return m_newKey;
            }
            
            inline void setNewKey(const Model::PropertyKey& newKey) {
                m_newKey = newKey;
            }
            
            inline const Model::PropertyValue& newValue() const {
                return m_newValue;
            }
            
            inline void setNewValue(const Model::PropertyValue& newValue) {
                m_newValue = newValue;
            }
            
            inline void setForce(const bool force) {
                m_force = force;
            }
            
            bool performDo();
            bool affectsImmutableProperty() const;
            bool affectsImmutableKey() const;
            bool canSetKey() const;
            bool anyEntityHasProperty(const Model::PropertyKey& key) const;
            void setKey();
            void setValue();
            void remove();

            bool performUndo();
            void restoreEntityDefinitions();

            EntityPropertyCommand(Type type, Model::MapDocument& document, const Model::EntityList& entities, const wxString& name);
        public:
            static EntityPropertyCommand* setEntityPropertyKey(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey, const bool force = false);
            static EntityPropertyCommand* setEntityPropertyValue(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& key, const Model::PropertyValue& newValue, const bool force = false);
            static EntityPropertyCommand* setEntityPropertyValue(Model::MapDocument& document, Model::Entity& entity, const Model::PropertyKey& key, const Model::PropertyValue& newValue, const bool force = false);
            static EntityPropertyCommand* removeEntityProperty(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKey& key, const bool force = false);
            static EntityPropertyCommand* removeEntityProperties(Model::MapDocument& document, const Model::EntityList& entities, const Model::PropertyKeyList& keys, const bool force = false);
            
            inline bool hasDefinitionChanged() const {
                return m_definitionChanged;
            }
            
            inline bool isPropertyAffected(const Model::PropertyKey& key) const {
                return m_newKey == key || std::find(m_keys.begin(), m_keys.end(), key) != m_keys.end();
            }
            
            inline bool isEntityAffected(const Model::Entity& entity) const {
                return std::find(m_entities.begin(), m_entities.end(), &entity) != m_entities.end();
            }
            
            const Model::EntityList& entities() const {
                return m_entities;
            }
            
            const Model::BrushList& brushes() const {
                return Model::EmptyBrushList;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyCommand__) */
