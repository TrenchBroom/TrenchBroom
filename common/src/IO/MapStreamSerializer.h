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

#ifndef TrenchBroom_MapStreamSerializer
#define TrenchBroom_MapStreamSerializer

#include "IO/NodeSerializer.h"
#include "Model/MapFormat.h"

#include <iosfwd>
#include <memory>
#include <string>

namespace TrenchBroom {
    namespace IO {
        class MapStreamSerializer : public NodeSerializer {
        private:
            std::ostream& m_stream;
        public:
            static std::unique_ptr<NodeSerializer> create(Model::MapFormat format, std::ostream& stream);
        protected:
            explicit MapStreamSerializer(std::ostream& stream);
        public:
            virtual ~MapStreamSerializer() override;
        protected:
            static std::string ftos(float v, int precision);
            static std::string ftos(double v, int precision);
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
            virtual void doWriteBrushFace(std::ostream& stream, const Model::BrushFace* face) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_MapStreamSerializer) */
