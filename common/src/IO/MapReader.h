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

#ifndef TrenchBroom_MapReader
#define TrenchBroom_MapReader

#include "FloatType.h"
#include "IO/StandardMapParser.h"
#include "Model/BrushFace.h"
#include "Model/IdType.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>

#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;
        class BrushNode;
        class EntityAttribute;
        class GroupNode;
        class LayerNode;
        class ModelFactory;
        class Node;
    }

    namespace IO {
        class ParserStatus;

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

            using LayerMap = std::map<Model::IdType, Model::LayerNode*>;
            using GroupMap = std::map<Model::IdType, Model::GroupNode*>;

            using NodeParentPair = std::pair<Model::Node*, ParentInfo>;
            using NodeParentList = std::vector<NodeParentPair>;

            vm::bbox3 m_worldBounds;
            Model::ModelFactory* m_factory;

            Model::Node* m_brushParent;
            Model::Node* m_currentNode;
            std::vector<Model::BrushFace> m_faces;

            LayerMap m_layers;
            GroupMap m_groups;
            NodeParentList m_unresolvedNodes;
        protected:
            MapReader(const char* begin, const char* end);
            explicit MapReader(const std::string& str);

            void readEntities(Model::MapFormat format, const vm::bbox3& worldBounds, ParserStatus& status);
            void readBrushes(Model::MapFormat format, const vm::bbox3& worldBounds, ParserStatus& status);
            void readBrushFaces(Model::MapFormat format, const vm::bbox3& worldBounds, ParserStatus& status);
        private: // implement MapParser interface
            void onFormatSet(Model::MapFormat format) override;
            void onBeginEntity(size_t line, const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status) override;
            void onEndEntity(size_t startLine, size_t lineCount, ParserStatus& status) override;
            void onBeginBrush(size_t line, ParserStatus& status) override;
            void onEndBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& status) override;
            void onBrushFace(size_t line, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY, ParserStatus& status) override;
        private: // helper methods
            void createLayer(size_t line, const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status);
            void createGroup(size_t line, const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status);
            void createEntity(size_t line, const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status);
            void createBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& status);

            ParentInfo::Type storeNode(Model::Node* node, const std::vector<Model::EntityAttribute>& attributes, ParserStatus& status);
            void stripParentAttributes(Model::AttributableNode* attributable, ParentInfo::Type parentType);

            void resolveNodes(ParserStatus& status);
            Model::Node* resolveParent(const ParentInfo& parentInfo) const;

            EntityType entityType(const std::vector<Model::EntityAttribute>& attributes) const;

            void setFilePosition(Model::Node* node, size_t startLine, size_t lineCount);
        protected:
            void setExtraAttributes(Model::Node* node, const ExtraAttributes& extraAttributes);
        private: // subclassing interface
            virtual Model::ModelFactory& initialize(Model::MapFormat format) = 0;
            virtual Model::Node* onWorldspawn(const std::vector<Model::EntityAttribute>& attributes, const ExtraAttributes& extraAttributes, ParserStatus& status) = 0;
            virtual void onWorldspawnFilePosition(size_t startLine, size_t lineCount, ParserStatus& status) = 0;
            virtual void onLayer(Model::LayerNode* layer, ParserStatus& status) = 0;
            virtual void onNode(Model::Node* parent, Model::Node* node, ParserStatus& status) = 0;
            virtual void onUnresolvedNode(const ParentInfo& parentInfo, Model::Node* node, ParserStatus& status) = 0;
            virtual void onBrush(Model::Node* parent, Model::BrushNode* brush, ParserStatus& status) = 0;
            virtual void onBrushFace(Model::BrushFace face, ParserStatus& status);
        };
    }
}

#endif /* defined(TrenchBroom_MapReader) */
