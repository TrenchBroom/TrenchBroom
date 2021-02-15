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

#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
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
         * 2. convert the raw data to nodes (for brushes this happens in parallel)
         * 3. post process the nodes to resolve layers, etc.
         */
        class MapReader : public StandardMapParser {
        protected:
            class ParentInfo {
            public:
                typedef enum {
                    Type_Layer,
                    Type_Group,
                    Type_None
                } Type;

                Type m_type;
                Model::IdType m_id;
            public:
                static ParentInfo layer(Model::IdType layerId);
                static ParentInfo group(Model::IdType groupId);
            private:
                ParentInfo(Type type, Model::IdType id);
            public:
                bool layer() const;
                bool group() const;
                Model::IdType id() const;
            };
        private:
            typedef enum {
                EntityType_Layer,
                EntityType_Group,
                EntityType_Worldspawn,
                EntityType_Default
            } EntityType;

            using LayerMap = std::unordered_map<Model::IdType, Model::LayerNode*>;
            using GroupMap = std::unordered_map<Model::IdType, Model::GroupNode*>;

            using NodeParentPair = std::pair<Model::Node*, ParentInfo>;
            using NodeParentList = std::vector<NodeParentPair>;

            vm::bbox3 m_worldBounds;
        private: // data populated in response to MapParser callbacks
            struct BrushInfo {
                std::vector<Model::BrushFace> faces;
                size_t startLine;
                size_t lineCount;
                ExtraAttributes extraAttributes;
            };
            struct EntityInfo {
                size_t startLine;
                size_t lineCount;
                std::vector<Model::EntityProperty> properties;
                ExtraAttributes extraAttributes;
                size_t brushesBegin;
                size_t brushesEnd;
            };
            std::vector<EntityInfo> m_entityInfos;
            std::vector<BrushInfo> m_brushInfos;
        private: // data populated by loadBrushes
            struct LoadedBrush {
                // optional wrapper is just to let this struct be default-constructible
                std::optional<kdl::result<Model::Brush, Model::BrushError>> brush;
                ExtraAttributes extraAttributes;
                size_t startLine;
                size_t lineCount;
            };
        private:
            Model::Node* m_brushParent;
            Model::Node* m_currentNode;
            LayerMap m_layers;
            GroupMap m_groups;
            NodeParentList m_unresolvedNodes;
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
            void onBeginEntity(size_t line, const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& status) override;
            void onEndEntity(size_t startLine, size_t lineCount, ParserStatus& status) override;
            void onBeginBrush(size_t line, ParserStatus& status) override;
            void onEndBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& status) override;
            void onStandardBrushFace(size_t line, Model::MapFormat targetMapFormat, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, ParserStatus& status) override;
            void onValveBrushFace(size_t line, Model::MapFormat targetMapFormat, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY, ParserStatus& status) override;
        private: // helper methods
            void createNodes(ParserStatus& status);
            void createNode(EntityInfo& info, std::vector<LoadedBrush>& brushes, ParserStatus& status);
            void createLayer(size_t line, const std::vector<Model::EntityProperty>& propeties, const ExtraAttributes& extraAttributes, ParserStatus& status);
            void createGroup(size_t line, const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& status);
            void createEntity(size_t line, const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& status);
            void createBrush(kdl::result<Model::Brush, Model::BrushError> brush, Model::Node* parent, size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& status);

            ParentInfo::Type storeNode(Model::Node* node, const std::vector<Model::EntityProperty>& properties, ParserStatus& status);
            void stripParentProperties(Model::EntityNodeBase* node, ParentInfo::Type parentType);

            void resolveNodes(ParserStatus& status);
            std::vector<LoadedBrush> loadBrushes(ParserStatus& status);
            Model::Node* resolveParent(const ParentInfo& parentInfo) const;

            EntityType entityType(const std::vector<Model::EntityProperty>& properties) const;

            void setFilePosition(Model::Node* node, size_t startLine, size_t lineCount);
        protected:
            void setExtraAttributes(Model::Node* node, const ExtraAttributes& extraAttributes);
        private: // subclassing interface - these will be called in the order that nodes should be inserted
            virtual Model::Node* onWorldspawn(const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& status) = 0;
            virtual void onWorldspawnFilePosition(size_t startLine, size_t lineCount, ParserStatus& status) = 0;
            virtual void onLayer(Model::LayerNode* layer, ParserStatus& status) = 0;
            virtual void onNode(Model::Node* parent, Model::Node* node, ParserStatus& status) = 0;
            virtual void onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node, ParserStatus& status) = 0;
            virtual void onBrush(Model::Node* parent, Model::BrushNode* brush, ParserStatus& status) = 0;
            virtual void onBrushFace(Model::BrushFace face, ParserStatus& status);
        };
    }
}

