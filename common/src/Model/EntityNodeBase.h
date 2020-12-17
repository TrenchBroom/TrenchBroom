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

#pragma once

#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Node.h"

#include <vecmath/bbox.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class PropertyDefinition;
        class EntityDefinition;
    }

    namespace Model {
        const Assets::EntityDefinition* selectEntityDefinition(const std::vector<EntityNodeBase*>& nodes);
        const Assets::PropertyDefinition* propertyDefinition(const EntityNodeBase* node, const std::string& key);
        const Assets::PropertyDefinition* selectPropertyDefinition(const std::string& key, const std::vector<EntityNodeBase*>& nodes);
        std::string selectPRopertyValue(const std::string& key, const std::vector<EntityNodeBase*>& nodes);

        class EntityNodeBase : public Node {
        protected:
            EntityNodeBase(Entity entity);

            Entity m_entity;

            std::vector<EntityNodeBase*> m_linkSources;
            std::vector<EntityNodeBase*> m_linkTargets;
            std::vector<EntityNodeBase*> m_killSources;
            std::vector<EntityNodeBase*> m_killTargets;
        public:
            virtual ~EntityNodeBase() override;
        public: // entity access
            const Entity& entity() const;
            Entity setEntity(Entity entity);
        public: // definition 
            void setDefinition(Assets::EntityDefinition* definition);
        private: // property management internals
            class NotifyPropertyChange {
            private:
                NotifyNodeChange m_nodeChange;
                EntityNodeBase* m_node;
                vm::bbox3 m_oldPhysicalBounds;
            public:
                NotifyPropertyChange(EntityNodeBase* node);
                ~NotifyPropertyChange();
            };

            void propertiesWillChange();
            void propertiesDidChange(const vm::bbox3& oldPhysicalBounds);
        private: // bulk update after property changes
            void updateIndexAndLinks(const std::vector<EntityProperty>& newProperties);
            void updatePropertyIndex(const std::vector<EntityProperty>& oldProperties, const std::vector<EntityProperty>& newProperties);
            void updateLinks(const std::vector<EntityProperty>& oldProperties, const std::vector<EntityProperty>& newProperties);
        private: // search index management
            void addPropertiesToIndex();
            void removePropertiesFromIndex();

            void addPropertyToIndex(const std::string& key, const std::string& value);
            void removePropertyFromIndex(const std::string& key, const std::string& value);
            void updatePropertyIndex(const std::string& oldKey, const std::string& oldValue, const std::string& newKey, const std::string& newValue);
        public: // link management
            const std::vector<EntityNodeBase*>& linkSources() const;
            const std::vector<EntityNodeBase*>& linkTargets() const;
            const std::vector<EntityNodeBase*>& killSources() const;
            const std::vector<EntityNodeBase*>& killTargets() const;

            vm::vec3 linkSourceAnchor() const;
            vm::vec3 linkTargetAnchor() const;

            bool hasMissingSources() const;
            std::vector<std::string> findMissingLinkTargets() const;
            std::vector<std::string> findMissingKillTargets() const;
        private: // link management internals
            void findMissingTargets(const std::string& prefix, std::vector<std::string>& result) const;


            void addLinks(const std::string& name, const std::string& value);
            void removeLinks(const std::string& name, const std::string& value);
            void updateLinks(const std::string& oldName, const std::string& oldValue, const std::string& newName, const std::string& newValue);

            void addLinkTargets(const std::string& targetname);
            void addKillTargets(const std::string& targetname);

            void removeLinkTargets(const std::string& targetname);
            void removeKillTargets(const std::string& targetname);

            void addAllLinkSources(const std::string& targetname);
            void addAllLinkTargets();
            void addAllKillSources(const std::string& targetname);
            void addAllKillTargets();

            void addLinkTargets(const std::vector<EntityNodeBase*>& targets);
            void addKillTargets(const std::vector<EntityNodeBase*>& targets);
            void addLinkSources(const std::vector<EntityNodeBase*>& sources);
            void addKillSources(const std::vector<EntityNodeBase*>& sources);

            void removeAllLinkSources();
            void removeAllLinkTargets();
            void removeAllKillSources();
            void removeAllKillTargets();

            void removeAllLinks();
            void addAllLinks();

            void addLinkSource(EntityNodeBase* node);
            void addLinkTarget(EntityNodeBase* node);
            void addKillSource(EntityNodeBase* node);
            void addKillTarget(EntityNodeBase* node);

            void removeLinkSource(EntityNodeBase* node);
            void removeLinkTarget(EntityNodeBase* node);
            void removeKillSource(EntityNodeBase* node);
            void removeKillTarget(EntityNodeBase* node);
        protected:
            EntityNodeBase();
        private: // implemenation of node interface
            const std::string& doGetName() const override;
            virtual void doAncestorWillChange() override;
            virtual void doAncestorDidChange() override;
        private: // subclassing interface
            virtual void doPropertiesDidChange(const vm::bbox3& oldBounds) = 0;
            virtual vm::vec3 doGetLinkSourceAnchor() const = 0;
            virtual vm::vec3 doGetLinkTargetAnchor() const = 0;
        private: // hide copy constructor and assignment operator
            EntityNodeBase(const EntityNodeBase&);
            EntityNodeBase& operator=(const EntityNodeBase&);
        };

        bool operator==(const EntityNodeBase& lhs, const EntityNodeBase& rhs);
        bool operator!=(const EntityNodeBase& lhs, const EntityNodeBase& rhs);
    }
}

