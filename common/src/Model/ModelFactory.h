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

#ifndef TrenchBroom_ModelFactory
#define TrenchBroom_ModelFactory

#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttributes;
        
        class ModelFactory {
        public:
            virtual ~ModelFactory();
            
            MapFormat::Type format() const;
            World* createWorld(const BBox3& worldBounds) const;
            Layer* createLayer(const String& name, const BBox3& worldBounds) const;
            Group* createGroup(const String& name) const;
            Entity* createEntity() const;
            Brush* createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const;
            
            BrushFace* createFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const BrushFaceAttributes& attribs) const;
            BrushFace* createFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY) const;
        private:
            virtual MapFormat::Type doGetFormat() const = 0;
            virtual World* doCreateWorld(const BBox3& worldBounds) const = 0;
            virtual Layer* doCreateLayer(const String& name, const BBox3& worldBounds) const = 0;
            virtual Group* doCreateGroup(const String& name) const = 0;
            virtual Entity* doCreateEntity() const = 0;
            virtual Brush* doCreateBrush(const BBox3& worldBounds, const BrushFaceList& faces) const = 0;
            virtual BrushFace* doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const BrushFaceAttributes& attribs) const = 0;
            virtual BrushFace* doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY) const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_ModelFactory) */
