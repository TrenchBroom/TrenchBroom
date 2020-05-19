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

namespace TrenchBroom {
    namespace Model {
        ModelFactory::~ModelFactory() {}

        MapFormat ModelFactory::format() const {
            return doGetFormat();
        }

        World* ModelFactory::createWorld() const {
            return doCreateWorld();
        }

        LayerNode* ModelFactory::createLayer(const std::string& name) const {
            return doCreateLayer(name);
        }

        Group* ModelFactory::createGroup(const std::string& name) const {
            return doCreateGroup(name);
        }

        Entity* ModelFactory::createEntity() const {
            return doCreateEntity();
        }

        BrushNode* ModelFactory::createBrush(const vm::bbox3& worldBounds, const std::vector<BrushFace*>& faces) const {
            return doCreateBrush(worldBounds, faces);
        }

        BrushFace* ModelFactory::createFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const {
            return doCreateFace(point1, point2, point3, attribs);
        }

        BrushFace* ModelFactory::createFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const {
            return doCreateFace(point1, point2, point3, attribs, texAxisX, texAxisY);
        }
    }
}
