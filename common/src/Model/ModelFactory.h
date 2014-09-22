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

#ifndef __TrenchBroom__ModelFactory__
#define __TrenchBroom__ModelFactory__

#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class ModelFactory {
        public:
            virtual ~ModelFactory();
            
            World* createWorld() const;
            Layer* createLayer(const String& name) const;
            Group* createGroup(const String& name) const;
            Entity* createEntity() const;
            Brush* createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const;
            BrushFace* createFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName) const;
            BrushFace* createFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName, const Vec3& texAxisX, const Vec3& texAxisY) const;
        private:
            virtual World* doCreateWorld() const = 0;
            virtual Layer* doCreateLayer(const String& name) const = 0;
            virtual Group* doCreateGroup(const String& name) const = 0;
            virtual Entity* doCreateEntity() const = 0;
            virtual Brush* doCreateBrush(const BBox3& worldBounds, const BrushFaceList& faces) const = 0;
            virtual BrushFace* doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName) const = 0;
            virtual BrushFace* doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName, const Vec3& texAxisX, const Vec3& texAxisY) const = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__ModelFactory__) */
