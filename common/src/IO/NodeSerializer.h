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

#include "Model/IdType.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class BrushFace;
        class EntityAttribute;
        class GroupNode;
        class LayerNode;
        class Node;
        class WorldNode;
    }

    namespace IO {
        class NodeSerializer {
        protected:
            using ObjectNo = unsigned int;
        private:
            class IdManager {
            private:
                using IdMap = std::unordered_map<const Model::Node*, std::string>;
                mutable IdMap m_ids;
            public:
                const std::string& getId(const Model::Node* t) const;
            private:
                Model::IdType makeId() const;
                std::string idToString(const Model::IdType nodeId) const;
            };

            IdManager m_layerIds;
            IdManager m_groupIds;

            ObjectNo m_entityNo;
            ObjectNo m_brushNo;

            bool m_exporting;
        public:
            NodeSerializer();
            virtual ~NodeSerializer();
        protected:
            ObjectNo entityNo() const;
            ObjectNo brushNo() const;
        public:
            bool exporting() const;
            void setExporting(bool exporting);
        public:
            void beginFile();
            void endFile();
        public:
            void defaultLayer(const Model::WorldNode& world);
            void customLayer(const Model::LayerNode* layer);
            void group(const Model::GroupNode* group, const std::vector<Model::EntityAttribute>& parentAttributes);

            void entity(const Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, const std::vector<Model::EntityAttribute>& parentAttributes, const Model::Node* brushParent);
            void entity(const Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, const std::vector<Model::EntityAttribute>& parentAttributes, const std::vector<Model::BrushNode*>& entityBrushes);
        private:
            void beginEntity(const Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, const std::vector<Model::EntityAttribute>& extraAttributes);
            void beginEntity(const Model::Node* node);
            void endEntity(const Model::Node* node);

            void entityAttributes(const std::vector<Model::EntityAttribute>& attributes);
            void entityAttribute(const Model::EntityAttribute& attribute);

            void brushes(const std::vector<Model::BrushNode*>& brushNodes);
            void brush(const Model::BrushNode* brushNode);

            void beginBrush(const Model::BrushNode* brushNode);
            void endBrush(const Model::BrushNode* brushNode);
        public:
            void brushFaces(const std::vector<Model::BrushFace>& faces);
        private:
            void brushFace(const Model::BrushFace& face);
        public:
            std::vector<Model::EntityAttribute> parentAttributes(const Model::Node* node);
        private:
            std::vector<Model::EntityAttribute> layerAttributes(const Model::LayerNode* layer);
            std::vector<Model::EntityAttribute> groupAttributes(const Model::GroupNode* group);
        protected:
            std::string escapeEntityAttribute(const std::string& str) const;
        private:
            virtual void doBeginFile() = 0;
            virtual void doEndFile() = 0;

            virtual void doBeginEntity(const Model::Node* node) = 0;
            virtual void doEndEntity(const Model::Node* node) = 0;
            virtual void doEntityAttribute(const Model::EntityAttribute& attribute) = 0;

            virtual void doBeginBrush(const Model::BrushNode* brushNode) = 0;
            virtual void doEndBrush(const Model::BrushNode* brushNode) = 0;
            virtual void doBrushFace(const Model::BrushFace& face) = 0;
        };
    }
}

