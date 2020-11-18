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
        private:
            TagType::Type m_hiddenTags;
            kdl::bitset m_hiddenEntityDefinitions;

            bool m_blockSelection;

            Model::GroupNode* m_currentGroup;
        public:
            Notifier<> editorContextDidChangeNotifier;
        public:
            EditorContext();

            void reset();

            TagType::Type hiddenTags() const;
            void setHiddenTags(TagType::Type hiddenTags);

            bool entityDefinitionHidden(const Model::AttributableNode* entityNode) const;
            bool entityDefinitionHidden(const Assets::EntityDefinition* definition) const;
            void setEntityDefinitionHidden(const Assets::EntityDefinition* definition, bool hidden);

            bool blockSelection() const;
            void setBlockSelection(bool blockSelection);
        public:
            Model::GroupNode* currentGroup() const;
            void pushGroup(Model::GroupNode* groupNode);
            void popGroup();
        public:
            bool visible(const Model::Node* node) const;
            bool visible(const Model::WorldNode* worldNode) const;
            bool visible(const Model::LayerNode* layerNode) const;
            bool visible(const Model::GroupNode* groupNode) const;
            bool visible(const Model::EntityNode* entityNode) const;
            bool visible(const Model::BrushNode* brushNode) const;
            bool visible(const Model::BrushNode* brushNode, const Model::BrushFace& face) const;
        private:
            bool anyChildVisible(const Model::Node* node) const;

        public:
            bool editable(const Model::Node* node) const;
            bool editable(const Model::BrushNode* brushNode, const Model::BrushFace& face) const;

        private:
            class NodePickable;
        public:
            bool pickable(const Model::Node* node) const;
            bool pickable(const Model::WorldNode* worldNode) const;
            bool pickable(const Model::LayerNode* layerNode) const;
            bool pickable(const Model::GroupNode* groupNode) const;
            bool pickable(const Model::EntityNode* entityNode) const;
            bool pickable(const Model::BrushNode* brushNode) const;
            bool pickable(const Model::BrushNode* brushNode, const Model::BrushFace& face) const;

            bool selectable(const Model::Node* node) const;
            bool selectable(const Model::WorldNode* worldNode) const;
            bool selectable(const Model::LayerNode* layerNode) const;
            bool selectable(const Model::GroupNode* groupNode) const;
            bool selectable(const Model::EntityNode* entityNode) const;
            bool selectable(const Model::BrushNode* brushNode) const;
            bool selectable(const Model::BrushNode* brushNode, const Model::BrushFace& face) const;

            bool canChangeSelection() const;
            bool inOpenGroup(const Model::Object* object) const;
        private:
            EditorContext(const EditorContext&);
            EditorContext& operator=(const EditorContext&);
        };
    }
}

#endif /* defined(TrenchBroom_EditorContext) */
