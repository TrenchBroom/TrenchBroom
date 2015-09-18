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

#ifndef TrenchBroom_MapFileSerializer
#define TrenchBroom_MapFileSerializer

#include "IO/NodeSerializer.h"
#include "Model/MapFormat.h"
#include "Model/Brush.h"
#include "Model/Node.h"

#include <cstdio>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class MapFileSerializer : public NodeSerializer {
        private:
            typedef std::vector<size_t> LineStack;
            LineStack m_startLineStack;
            size_t m_line;
            FILE* m_stream;
        public:
            static Ptr create(Model::MapFormat::Type format, FILE* stream);
        protected:
            MapFileSerializer(FILE* file);
        private:
            void doBeginEntity(const Model::Node* node);
            void doEndEntity(Model::Node* node);
            void doEntityAttribute(const Model::EntityAttribute& attribute);
            void doBeginBrush(const Model::Brush* brush);
            void doEndBrush(Model::Brush* brush);
            void doBrushFace(Model::BrushFace* face);
        private:
            void setFilePosition(Model::Node* node);
            size_t startLine();
        private:
            virtual size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_MapFileSerializer) */
