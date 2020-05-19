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

#ifndef TrenchBroom_MapFileSerializer
#define TrenchBroom_MapFileSerializer

#include "IO/NodeSerializer.h"
#include "Model/MapFormat.h"

#include <cstdio> // for FILE*
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class BrushFace;
        class EntityAttribute;
        class Node;
    }

    namespace IO {
        class MapFileSerializer : public NodeSerializer {
        private:
            using LineStack = std::vector<size_t>;
            LineStack m_startLineStack;
            size_t m_line;
            FILE* m_stream;
        public:
            static std::unique_ptr<NodeSerializer> create(Model::MapFormat format, FILE* stream);
        protected:
            explicit MapFileSerializer(FILE* file);
        private:
            void doBeginFile() override;
            void doEndFile() override;

            void doBeginEntity(const Model::Node* node) override;
            void doEndEntity(Model::Node* node) override;
            void doEntityAttribute(const Model::EntityAttribute& attribute) override;
            void doBeginBrush(const Model::BrushNode* brush) override;
            void doEndBrush(Model::BrushNode* brush) override;
            void doBrushFace(const Model::BrushFace* face) override;
        private:
            void setFilePosition(Model::Node* node);
            size_t startLine();
        private:
            virtual size_t doWriteBrushFace(FILE* stream, const Model::BrushFace* face) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_MapFileSerializer) */
