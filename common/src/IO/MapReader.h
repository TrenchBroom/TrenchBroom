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

#include "FloatType.h"
#include "IO/StandardMapParser.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/IdType.h"

#include <kdl/result.h>

#include <vecmath/forward.h>
#include <vecmath/bbox.h>

#include <string_view>
#include <unordered_map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EntityNode;
        class EntityNodeBase;
        class BrushNode;
        class EntityProperty;
        class GroupNode;
        class LayerNode;
        enum class MapFormat;
        class Node;
    }

    namespace IO {
        class ParserStatus;

        /**
         * Abstract superclass containing common code for:
         *
         *  - WorldReader (loading a whole .map)
         *  - NodeReader (reading part of a map, for pasting into an existing map)
         *  - BrushFaceReader (reading faces when copy/pasting texture alignment)
         *
         * The flow of data is:
         *
         * 1. MapParser callbacks get called with the raw data, which we just store
         *    (m_entityInfos, m_brushInfos)
         * 2. convert the raw data to nodes (for brushes this happens in parallel) (createNodes)
         * 3. post process the nodes to resolve layers, etc.
         */
        class MapReader : public StandardMapParser {
        private:
            struct EntityInfo {
                size_t startLine;
                size_t lineCount;
                std::vector<Model::EntityProperty> properties;
                size_t brushesBegin;
                size_t brushesEnd;
            };

            struct BrushInfo {
                std::vector<Model::BrushFace> faces;
                size_t startLine;
                size_t lineCount;
            };
        protected:
            enum class ParentType {
                Layer,
                Group,
                None
            };

            struct ParentInfo {
                ParentType type;
                Model::IdType id;
            };
        private:
            vm::bbox3 m_worldBounds;
        private: // data populated in response to MapParser callbacks
            std::vector<EntityInfo> m_entityInfos;
            std::vector<BrushInfo> m_brushInfos;
        private: // layer / group lookup and management
            std::unordered_map<Model::IdType, Model::LayerNode*> m_layers;
            std::unordered_map<Model::IdType, Model::GroupNode*> m_groups;
            std::unordered_map<std::string, Model::GroupNode*> m_linkedGroups;
            std::vector<std::pair<Model::Node*, ParentInfo>> m_unresolvedNodes;
        protected:
            /**
             * Creates a new reader where the given string is expected to be formatted in the given source map format,
             * and the created objects are converted to the given target format.
             *
             * @param str the string to parse
             * @param sourceMapFormat the expected format of the given string
             * @param targetMapFormat the format to convert the created objects to
             */
            MapReader(std::string_view str, Model::MapFormat sourceMapFormat, Model::MapFormat targetMapFormat);

            /**
             * Attempts to parse as one or more entities.
             *
             * @throws ParserException if parsing fails
             */
            void readEntities(const vm::bbox3& worldBounds, ParserStatus& status);
            /**
             * Attempts to parse as one or more brushes without any enclosing entity.
             *
             * @throws ParserException if parsing fails
             */
            void readBrushes(const vm::bbox3& worldBounds, ParserStatus& status);
            /**
             * Attempts to parse as one or more brush faces.
             *
             * @throws ParserException if parsing fails
             */
            void readBrushFaces(const vm::bbox3& worldBounds, ParserStatus& status);
        protected: // implement MapParser interface
            void onBeginEntity(size_t line, std::vector<Model::EntityProperty> properties, ParserStatus& status) override;
            void onEndEntity(size_t startLine, size_t lineCount, ParserStatus& status) override;
            void onBeginBrush(size_t line, ParserStatus& status) override;
            void onEndBrush(size_t startLine, size_t lineCount, ParserStatus& status) override;
            void onStandardBrushFace(size_t line, Model::MapFormat targetMapFormat, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, ParserStatus& status) override;
            void onValveBrushFace(size_t line, Model::MapFormat targetMapFormat, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY, ParserStatus& status) override;
        private: // helper methods
            friend std::vector<std::unique_ptr<Model::BrushNode>> createBrushes(const vm::bbox3& worldBounds, std::vector<BrushInfo> brushInfos, ParserStatus& status);
            void createNodes(ParserStatus& status);
            void createLayerGroupOrEntity(Model::Node*& currentParent, EntityInfo& info, std::vector<std::unique_ptr<Model::BrushNode>>& loadedBrushes, ParserStatus& status);

            void storeNode(Model::Node* node, const std::vector<Model::EntityProperty>& properties, ParserStatus& status);
            void resolveNodes(ParserStatus& status);
            Model::Node* resolveParent(const ParentInfo& parentInfo) const;
        private: // subclassing interface - these will be called in the order that nodes should be inserted
            virtual Model::Node* onWorldspawn(std::vector<Model::EntityProperty> properties, ParserStatus& status) = 0;
            virtual void onWorldspawnFilePosition(size_t startLine, size_t lineCount, ParserStatus& status) = 0;
            virtual void onLayer(Model::LayerNode* layer, ParserStatus& status) = 0;
            virtual void onNode(Model::Node* parent, Model::Node* node, ParserStatus& status) = 0;
            virtual void onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node, ParserStatus& status) = 0;
            virtual void onBrush(Model::Node* parent, Model::BrushNode* brush, ParserStatus& status) = 0;
            virtual void onBrushFace(Model::BrushFace face, ParserStatus& status);
        };
    }
}

