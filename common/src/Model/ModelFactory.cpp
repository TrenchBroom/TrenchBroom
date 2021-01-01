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

#include "ModelFactory.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"

#include <kdl/result.h>

namespace TrenchBroom {
    namespace Model {
        ModelFactory::~ModelFactory() {}

        MapFormat ModelFactory::format() const {
            return doGetFormat();
        }

        WorldNode* ModelFactory::createWorld(Entity entity) const {
            return doCreateWorld(std::move(entity));
        }

        LayerNode* ModelFactory::createLayer(const std::string& name) const {
            return doCreateLayer(name);
        }

        GroupNode* ModelFactory::createGroup(const std::string& name) const {
            return doCreateGroup(name);
        }

        EntityNode* ModelFactory::createEntity(Entity entity) const {
            return doCreateEntity(std::move(entity));
        }

        BrushNode* ModelFactory::createBrush(Brush brush) const {
            return doCreateBrush(std::move(brush));
        }

        kdl::result<BrushFace, BrushError> ModelFactory::createFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const {
            return doCreateFace(point1, point2, point3, attribs);
        }

        kdl::result<BrushFace, BrushError> ModelFactory::createFaceFromStandard(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const {
            return doCreateFaceFromStandard(point1, point2, point3, attribs);
        }

        kdl::result<BrushFace, BrushError> ModelFactory::createFaceFromValve(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const {
            return doCreateFaceFromValve(point1, point2, point3, attribs, texAxisX, texAxisY);
        }
        
        BrushNode* ModelFactory::doCreateBrush(Brush brush) const {
            return new BrushNode(std::move(brush));
        }
    }
}
