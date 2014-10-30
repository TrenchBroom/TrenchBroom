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

#include "ModelFactory.h"

namespace TrenchBroom {
    namespace Model {
        ModelFactory::~ModelFactory() {}
        
        MapFormat::Type ModelFactory::format() const {
            return doGetFormat();
        }

        World* ModelFactory::createWorld() const {
            return doCreateWorld();
        }
        
        Layer* ModelFactory::createLayer(const String& name) const {
            return doCreateLayer(name);
        }
        
        Group* ModelFactory::createGroup(const String& name) const {
            return doCreateGroup(name);
        }
        
        Entity* ModelFactory::createEntity() const {
            return doCreateEntity();
        }
        
        Brush* ModelFactory::createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const {
            return doCreateBrush(worldBounds, faces);
        }
        
        BrushFace* ModelFactory::createFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName) const {
            return doCreateFace(point1, point2, point3, textureName);
        }
        
        BrushFace* ModelFactory::createFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName, const Vec3& texAxisX, const Vec3& texAxisY) const {
            return doCreateFace(point1, point2, point3, textureName, texAxisX, texAxisY);
        }
    }
}
