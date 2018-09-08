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

#ifndef TrenchBroom_ModelFactoryImpl
#define TrenchBroom_ModelFactoryImpl

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Model/MapFormat.h"
#include "Model/ModelFactory.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushContentTypeBuilder;
        class BrushFaceAttributes;
        
        class ModelFactoryImpl : public ModelFactory {
        private:
            MapFormat::Type m_format;
            const BrushContentTypeBuilder* m_brushContentTypeBuilder;
        public:
            ModelFactoryImpl();
            ModelFactoryImpl(MapFormat::Type format, const BrushContentTypeBuilder* brushContentTypeBuilder);
        private: // implement ModelFactory interface
            MapFormat::Type doGetFormat() const override;
            World* doCreateWorld(const vm::bbox3& worldBounds) const override;
            Layer* doCreateLayer(const String& name, const vm::bbox3& worldBounds) const override;
            Group* doCreateGroup(const String& name) const override;
            Entity* doCreateEntity() const override;
            Brush* doCreateBrush(const vm::bbox3& worldBounds, const BrushFaceList& faces) const override;
            
            BrushFace* doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const override;
            BrushFace* doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const override;
        };
    }
}

#endif /* defined(TrenchBroom_ModelFactoryImpl) */
