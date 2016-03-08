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

#ifndef TrenchBroom_MapStreamSerializer
#define TrenchBroom_MapStreamSerializer

#include "IO/NodeSerializer.h"
#include "Model/MapFormat.h"

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class MapStreamSerializer : public NodeSerializer {
        private:
            std::ostream& m_stream;
        public:
            static Ptr create(Model::MapFormat::Type format, std::ostream& stream);
        protected:
            MapStreamSerializer(std::ostream& stream);
        public:
            virtual ~MapStreamSerializer();
        private:
            void doBeginEntity(const Model::Node* node);
            void doEndEntity(Model::Node* node);
            void doEntityAttribute(const Model::EntityAttribute& attribute);
            void doBeginBrush(const Model::Brush* brush);
            void doEndBrush(Model::Brush* brush);
            void doBrushFace(Model::BrushFace* face);
        private:
            virtual void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_MapStreamSerializer) */
