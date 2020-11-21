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

#include "FloatType.h"
#include "Model/MapFormat.h"
#include "Model/ModelFactory.h"

#include <kdl/result_forward.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttributes;
        enum class BrushError;
        class Entity;

        class ModelFactoryImpl : public ModelFactory {
        private:
            MapFormat m_format;
        public:
            ModelFactoryImpl();
            ModelFactoryImpl(MapFormat format);
        private: // implement ModelFactory interface
            MapFormat doGetFormat() const override;
            WorldNode* doCreateWorld(Entity entity) const override;
            LayerNode* doCreateLayer(const std::string& name) const override;
            GroupNode* doCreateGroup(const std::string& name) const override;
            EntityNode* doCreateEntity(Entity entity) const override;

            kdl::result<BrushFace, BrushError> doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const override;
            kdl::result<BrushFace, BrushError> doCreateFaceFromStandard(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const override;
            kdl::result<BrushFace, BrushError> doCreateFaceFromValve(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const override;
        };
    }
}

#endif /* defined(TrenchBroom_ModelFactoryImpl) */
