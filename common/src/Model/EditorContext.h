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

#ifndef TrenchBroom_EditorContext
#define TrenchBroom_EditorContext

#include "Notifier.h"
#include "Model/TagType.h"

#include <kdl/bitset.h>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
    }

    namespace Model {
        class AttributableNode;
        class BrushNode;
        class BrushFace;
        class EntityNode;
        class GroupNode;
        class LayerNode;
        class Node;
        class Object;
        class WorldNode;

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
            TagType::Type m_hiddenTags;
            kdl::bitset m_hiddenEntityDefinitions;
            EntityLinkMode m_entityLinkMode;

            bool m_blockSelection;

            Model::GroupNode* m_currentGroup;
        public:
            Notifier<> editorContextDidChangeNotifier;
        public:
            EditorContext();

            void reset();

            bool showPointEntities() const;
            void setShowPointEntities(bool showPointEntities);

            bool showBrushes() const;
            void setShowBrushes(bool showBrushes);

            TagType::Type hiddenTags() const;
            void setHiddenTags(TagType::Type hiddenTags);

            bool entityDefinitionHidden(const Model::AttributableNode* entity) const;
            bool entityDefinitionHidden(const Assets::EntityDefinition* definition) const;
            void setEntityDefinitionHidden(const Assets::EntityDefinition* definition, bool hidden);

            EntityLinkMode entityLinkMode() const;
            void setEntityLinkMode(EntityLinkMode entityLinkMode);

            bool blockSelection() const;
            void setBlockSelection(bool blockSelection);
        public:
            Model::GroupNode* currentGroup() const;
            void pushGroup(Model::GroupNode* group);
            void popGroup();
        public:
            bool visible(const Model::Node* node) const;
            bool visible(const Model::WorldNode* world) const;
            bool visible(const Model::LayerNode* layer) const;
            bool visible(const Model::GroupNode* group) const;
            bool visible(const Model::EntityNode* entity) const;
            bool visible(const Model::BrushNode* brush) const;
            bool visible(const Model::BrushFace* face) const;
        private:
            bool anyChildVisible(const Model::Node* node) const;

        public:
            bool editable(const Model::Node* node) const;
            bool editable(const Model::BrushFace* face) const;

        private:
            class NodePickable;
        public:
            bool pickable(const Model::Node* node) const;
            bool pickable(const Model::WorldNode* world) const;
            bool pickable(const Model::LayerNode* layer) const;
            bool pickable(const Model::GroupNode* group) const;
            bool pickable(const Model::EntityNode* entity) const;
            bool pickable(const Model::BrushNode* brush) const;
            bool pickable(const Model::BrushFace* face) const;

            bool selectable(const Model::Node* node) const;
            bool selectable(const Model::WorldNode* world) const;
            bool selectable(const Model::LayerNode* layer) const;
            bool selectable(const Model::GroupNode* group) const;
            bool selectable(const Model::EntityNode* entity) const;
            bool selectable(const Model::BrushNode* brush) const;
            bool selectable(const Model::BrushFace* face) const;

            bool canChangeSelection() const;
            bool inOpenGroup(const Model::Object* object) const;
        private:
            EditorContext(const EditorContext&);
            EditorContext& operator=(const EditorContext&);
        };
    }
}

#endif /* defined(TrenchBroom_EditorContext) */
