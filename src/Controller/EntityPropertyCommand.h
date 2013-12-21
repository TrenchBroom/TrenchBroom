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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__EntityPropertyCommand__
#define __TrenchBroom__EntityPropertyCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class EntityPropertyCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<EntityPropertyCommand> Ptr;
        private:
            typedef enum {
                PCRenameProperty,
                PCSetProperty,
                PCRemoveProperty
            } PropertyCommand;
            
            const PropertyCommand m_command;
            View::MapDocumentWPtr m_document;
            const Model::EntityList m_entities;
            bool m_force;
            Model::PropertyKeyList m_keys;
            Model::PropertyKey m_newKey;
            Model::PropertyValue m_newValue;
            bool m_definitionAffected;
            Model::Snapshot m_snapshot;

            void setKey(const Model::PropertyKey& key);
            void setKeys(const Model::PropertyKeyList& newKeys);
            void setNewKey(const Model::PropertyKey& key);
            void setNewValue(const Model::PropertyValue& newValue);
        public:
            EntityPropertyCommand(View::MapDocumentWPtr document, const PropertyCommand command, const Model::EntityList& entities, const bool force);

            static Command::Ptr renameEntityProperty(View::MapDocumentWPtr document, const Model::EntityList& entities, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey, const bool force);
            static Command::Ptr setEntityProperty(View::MapDocumentWPtr document, const Model::EntityList& entities, const Model::PropertyKey& key, const Model::PropertyKey& newValue, const bool force);
            static Command::Ptr removeEntityProperty(View::MapDocumentWPtr document, const Model::EntityList& entities, const Model::PropertyKey& key, const bool force);
            
            const Model::PropertyKey& key() const;
            const Model::PropertyKeyList& keys() const;
            const Model::PropertyKey& newKey() const;
            const Model::PropertyValue& newValue() const;
            
            bool definitionAffected() const;
            bool propertyAffected(const Model::PropertyKey& key);
            bool entityAffected(const Model::Entity* entity);
            const Model::EntityList& affectedEntities() const;
        private:
            static String makeName(const PropertyCommand command);
            bool doPerformDo();
            bool doPerformUndo();
            
            void rename();
            void setValue();
            void remove();
            
            bool affectsImmutablePropertyKey() const;
            bool affectsImmutablePropertyValue() const;
            bool canSetKey() const;
            bool anyEntityHasProperty(const Model::PropertyKey& key) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyCommand__) */
