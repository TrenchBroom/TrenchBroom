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

#include "BrushBuilder.h"

#include "Ensure.h"
#include "Polyhedron.h"
#include "Model/Brush.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/result_for_each.h>
#include <kdl/string_utils.h>

#include <cassert>
#include <string>

namespace TrenchBroom {
    namespace Model {
        BrushBuilder::BrushBuilder(const MapFormat mapFormat, const vm::bbox3& worldBounds) :
        m_mapFormat(mapFormat),
        m_worldBounds(worldBounds),
        m_defaultAttribs(BrushFaceAttributes::NoTextureName) {}

        BrushBuilder::BrushBuilder(const MapFormat mapFormat, const vm::bbox3& worldBounds, const BrushFaceAttributes& defaultAttribs) :
        m_mapFormat(mapFormat),
        m_worldBounds(worldBounds),
        m_defaultAttribs(defaultAttribs) {}

        kdl::result<Brush, BrushError> BrushBuilder::createCube(const FloatType size, const std::string& textureName) const {
            return createCuboid(vm::bbox3(size / 2.0), textureName, textureName, textureName, textureName, textureName, textureName);
        }

        kdl::result<Brush, BrushError> BrushBuilder::createCube(FloatType size, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const {
            return createCuboid(vm::bbox3(size / 2.0), leftTexture, rightTexture, frontTexture, backTexture, topTexture, bottomTexture);
        }
        
        kdl::result<Brush, BrushError> BrushBuilder::createCuboid(const vm::vec3& size, const std::string& textureName) const {
            return createCuboid(vm::bbox3(-size / 2.0, size / 2.0), textureName, textureName, textureName, textureName, textureName, textureName);
        }

        kdl::result<Brush, BrushError> BrushBuilder::createCuboid(const vm::vec3& size, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const {
            return createCuboid(vm::bbox3(-size / 2.0, size / 2.0), leftTexture, rightTexture, frontTexture, backTexture, topTexture, bottomTexture);
        }

        kdl::result<Brush, BrushError> BrushBuilder::createCuboid(const vm::bbox3& bounds, const std::string& textureName) const {
            return createCuboid(bounds, textureName, textureName, textureName, textureName, textureName, textureName);
        }

        kdl::result<Brush, BrushError> BrushBuilder::createCuboid(const vm::bbox3& bounds, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const {
            const auto specs = std::vector<std::tuple<vm::vec3, vm::vec3, vm::vec3, BrushFaceAttributes>>({
                { bounds.min, bounds.min + vm::vec3::pos_y(), bounds.min + vm::vec3::pos_z(), BrushFaceAttributes(leftTexture, m_defaultAttribs) }, // left
                { bounds.max, bounds.max + vm::vec3::pos_z(), bounds.max + vm::vec3::pos_y(), BrushFaceAttributes(rightTexture, m_defaultAttribs) }, // right
                { bounds.min, bounds.min + vm::vec3::pos_z(), bounds.min + vm::vec3::pos_x(), BrushFaceAttributes(frontTexture, m_defaultAttribs) }, // front
                { bounds.max, bounds.max + vm::vec3::pos_x(), bounds.max + vm::vec3::pos_z(), BrushFaceAttributes(backTexture, m_defaultAttribs) }, // back
                { bounds.max, bounds.max + vm::vec3::pos_y(), bounds.max + vm::vec3::pos_x(), BrushFaceAttributes(topTexture, m_defaultAttribs) }, // top
                { bounds.min, bounds.min + vm::vec3::pos_x(), bounds.min + vm::vec3::pos_y(), BrushFaceAttributes(bottomTexture, m_defaultAttribs) }, // bottom
            });
            
            return kdl::for_each_result(specs, [&](const auto spec) {
                const auto& [p1, p2, p3, attrs] = spec;
                return BrushFace::create(p1, p2, p3, attrs, m_mapFormat);
            }).and_then([&](auto&& faces) {
                return Brush::create(m_worldBounds, std::move(faces));
            });
        }
        
        kdl::result<Brush, BrushError> BrushBuilder::createBrush(const std::vector<vm::vec3>& points, const std::string& textureName) const {
            return createBrush(Polyhedron3(points), textureName);
        }

        kdl::result<Brush, BrushError> BrushBuilder::createBrush(const Polyhedron3& polyhedron, const std::string& textureName) const {
            assert(polyhedron.closed());

            return kdl::for_each_result(polyhedron.faces(), [&](const auto* face) {
                const auto& boundary = face->boundary();

                auto bIt = std::begin(boundary);
                const auto* edge1 = *bIt++;
                const auto* edge2 = *bIt++;
                const auto* edge3 = *bIt++;

                const auto& p1 = edge1->origin()->position();
                const auto& p2 = edge2->origin()->position();
                const auto& p3 = edge3->origin()->position();

                return BrushFace::create(p1, p3, p2, Model::BrushFaceAttributes(textureName), m_mapFormat);
            }).and_then([&](auto&& faces) {
                return Brush::create(m_worldBounds, std::move(faces));
            });
        }
    }
}
