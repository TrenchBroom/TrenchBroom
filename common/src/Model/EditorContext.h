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

#ifndef __TrenchBroom__EditorContext__
#define __TrenchBroom__EditorContext__

#include "Model/BrushContentType.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
    }

    namespace Model {
        class EditorContext {
        public:
            typedef enum {
                EntityLinkMode_All,
                EntityLinkMode_Transitive,
                EntityLinkMode_Direct,
                EntityLinkMode_None
            } EntityLinkMode;
        private:
            bool m_showPointEntities;
            bool m_showBrushes;
            Model::BrushContentType::FlagType m_hiddenBrushContentTypes;
            Bitset m_hiddenEntityDefinitions;
            EntityLinkMode m_entityLinkMode;
        public:
            EditorContext();
            
            bool showPointEntities() const;
            void setShowPointEntities(bool showPointEntities);
            
            bool showBrushes() const;
            void setShowBrushes(bool showBrushes);
            
            Model::BrushContentType::FlagType hiddenBrushContentTypes() const;
            void setHiddenBrushContentTypes(Model::BrushContentType::FlagType brushContentTypes);
            
            bool entityDefinitionHidden(const Assets::EntityDefinition* definition) const;
            void setEntityDefinitionHidden(const Assets::EntityDefinition* definition, bool hidden);
            
            EntityLinkMode entityLinkMode() const;
            void setEntityLinkMode(EntityLinkMode entityLinkMode);
        public:
            bool visible(const Model::Node* node) const;
            bool visible(const Model::Layer* layer) const;
            bool visible(const Model::Group* group) const;
            bool visible(const Model::Entity* entity) const;
            bool visible(const Model::Brush* brush) const;
            bool visible(const Model::BrushFace* face) const;
            
            bool locked(const Model::Node* node) const;
            bool locked(const Model::Layer* layer) const;
            bool locked(const Model::Group* group) const;
            bool locked(const Model::Entity* entity) const;
            bool locked(const Model::Brush* brush) const;
            bool locked(const Model::BrushFace* face) const;

            bool pickable(const Model::Node* node) const;
            bool pickable(const Model::Layer* layer) const;
            bool pickable(const Model::Group* group) const;
            bool pickable(const Model::Entity* entity) const;
            bool pickable(const Model::Brush* brush) const;
            bool pickable(const Model::BrushFace* face) const;
            
            bool selectable(const Model::Node* node) const;
            bool selectable(const Model::Layer* layer) const;
            bool selectable(const Model::Group* group) const;
            bool selectable(const Model::Entity* entity) const;
            bool selectable(const Model::Brush* brush) const;
            bool selectable(const Model::BrushFace* face) const;
        private:
            EditorContext(const EditorContext&);
            EditorContext& operator=(const EditorContext&);
        };
    }
}

#endif /* defined(__TrenchBroom__EditorContext__) */
