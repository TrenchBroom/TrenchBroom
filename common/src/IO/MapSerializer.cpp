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

#include "MapSerializer.h"

namespace TrenchBroom {
    namespace IO {
        MapSerializer::~MapSerializer() {}
        
        void MapSerializer::beginEntity(const Model::Node* node) {
            doBeginEntity(node);
        }
        
        void MapSerializer::endEntity(Model::Node* node) {
            doEndEntity(node);
        }
        
        void MapSerializer::entityAttribute(const Model::EntityAttribute& attribute) {
            doEntityAttribute(attribute);
        }
        
        void MapSerializer::beginBrush(const Model::Brush* brush) {
            doBeginBrush(brush);
        }
        
        void MapSerializer::endBrush(Model::Brush* brush) {
            doEndBrush(brush);
        }
        
        void MapSerializer::brushFace(Model::BrushFace* face) {
            doBrushFace(face);
        }
    }
}
