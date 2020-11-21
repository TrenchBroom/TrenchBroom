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

#include "ModelFactoryImpl.h"

#include "Exceptions.h"
#include "Model/BrushNode.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_utils.h>

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        ModelFactoryImpl::ModelFactoryImpl() :
        m_format(MapFormat::Unknown) {}

        ModelFactoryImpl::ModelFactoryImpl(const MapFormat format) :
        m_format(format) {
            assert(m_format != MapFormat::Unknown);
        }

        MapFormat ModelFactoryImpl::doGetFormat() const {
            return m_format;
        }

        WorldNode* ModelFactoryImpl::doCreateWorld(Entity entity) const {
            assert(m_format != MapFormat::Unknown);
            return new WorldNode(std::move(entity), m_format);
        }

        LayerNode* ModelFactoryImpl::doCreateLayer(const std::string& name) const {
            assert(m_format != MapFormat::Unknown);
            return new LayerNode(name);
        }

        GroupNode* ModelFactoryImpl::doCreateGroup(const std::string& name) const {
            assert(m_format != MapFormat::Unknown);
            return new GroupNode(name);
        }

        EntityNode* ModelFactoryImpl::doCreateEntity(Entity entity) const {
            assert(m_format != MapFormat::Unknown);
            return new EntityNode(std::move(entity));
        }

        kdl::result<BrushFace, BrushError> ModelFactoryImpl::doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const {
            assert(m_format != MapFormat::Unknown);
            return Model::isParallelTexCoordSystem(m_format)
                   ? BrushFace::create(point1, point2, point3, attribs, std::make_unique<ParallelTexCoordSystem>(point1, point2, point3, attribs))
                   : BrushFace::create(point1, point2, point3, attribs, std::make_unique<ParaxialTexCoordSystem>(point1, point2, point3, attribs));
        }

        kdl::result<BrushFace, BrushError> ModelFactoryImpl::doCreateFaceFromStandard(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& inputAttribs) const {
            assert(m_format != MapFormat::Unknown);

            std::unique_ptr<TexCoordSystem> texCoordSystem;
            BrushFaceAttributes attribs("");

            if (Model::isParallelTexCoordSystem(m_format)) {
                // Convert paraxial to parallel
                std::tie(texCoordSystem, attribs) = ParallelTexCoordSystem::fromParaxial(point1, point2, point3, inputAttribs);
            } else {
                // Pass through paraxial
                texCoordSystem = std::make_unique<ParaxialTexCoordSystem>(point1, point2, point3, inputAttribs);
                attribs = inputAttribs;
            }

            return BrushFace::create(point1, point2, point3, attribs, std::move(texCoordSystem));
        }

        kdl::result<BrushFace, BrushError> ModelFactoryImpl::doCreateFaceFromValve(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& inputAttribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const {
            assert(m_format != MapFormat::Unknown);

            std::unique_ptr<TexCoordSystem> texCoordSystem;
            BrushFaceAttributes attribs("");

            if (Model::isParallelTexCoordSystem(m_format)) {
                // Pass through parallel
                texCoordSystem = std::make_unique<ParallelTexCoordSystem>(texAxisX, texAxisY);
                attribs = inputAttribs;
            } else {
                // Convert parallel to paraxial
                std::tie(texCoordSystem, attribs) = ParaxialTexCoordSystem::fromParallel(point1, point2, point3, inputAttribs, texAxisX, texAxisY);
            }

            return BrushFace::create(point1, point2, point3, attribs, std::move(texCoordSystem));
        }
    }
}
