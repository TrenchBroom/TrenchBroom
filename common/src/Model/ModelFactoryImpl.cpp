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

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Brush.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/World.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        ModelFactoryImpl::ModelFactoryImpl() :
        m_format(MapFormat::Unknown),
        m_brushContentTypeBuilder(nullptr) {}
        
        ModelFactoryImpl::ModelFactoryImpl(const MapFormat format, const BrushContentTypeBuilder* brushContentTypeBuilder) :
        m_format(format),
        m_brushContentTypeBuilder(brushContentTypeBuilder) {
            assert(m_format != MapFormat::Unknown);
        }

        MapFormat ModelFactoryImpl::doGetFormat() const {
            return m_format;
        }

        World* ModelFactoryImpl::doCreateWorld(const vm::bbox3& worldBounds) const {
            assert(m_format != MapFormat::Unknown);
            return new World(m_format, m_brushContentTypeBuilder, worldBounds);
        }

        Layer* ModelFactoryImpl::doCreateLayer(const String& name, const vm::bbox3& worldBounds) const {
            assert(m_format != MapFormat::Unknown);
            return new Layer(name, worldBounds);
        }
        
        Group* ModelFactoryImpl::doCreateGroup(const String& name) const {
            assert(m_format != MapFormat::Unknown);
            return new Group(name);
        }

        Entity* ModelFactoryImpl::doCreateEntity() const {
            assert(m_format != MapFormat::Unknown);
            return new Entity();
        }
        
        Brush* ModelFactoryImpl::doCreateBrush(const vm::bbox3& worldBounds, const BrushFaceList& faces) const {
            assert(m_format != MapFormat::Unknown);
            Brush* brush = new Brush(worldBounds, faces);
            brush->setContentTypeBuilder(m_brushContentTypeBuilder);
            return brush;
        }

        BrushFace* ModelFactoryImpl::doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const {
            assert(m_format != MapFormat::Unknown);
            if (m_format == MapFormat::Valve) {
                return new BrushFace(point1, point2, point3, attribs,
                                     std::make_unique<ParallelTexCoordSystem>(point1, point2, point3, attribs));
            } else {
                return new BrushFace(point1, point2, point3, attribs,
                                     std::make_unique<ParaxialTexCoordSystem>(point1, point2, point3, attribs));
            }
        }

        BrushFace* ModelFactoryImpl::doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const {
            assert(m_format != MapFormat::Unknown);
            if (m_format == MapFormat::Valve) {
                return new BrushFace(point1, point2, point3, attribs,
                                     std::make_unique<ParallelTexCoordSystem>(texAxisX, texAxisY));
            } else {
                return new BrushFace(point1, point2, point3, attribs,
                                     std::make_unique<ParaxialTexCoordSystem>(point1, point2, point3, attribs));
            }
        }
    }
}
