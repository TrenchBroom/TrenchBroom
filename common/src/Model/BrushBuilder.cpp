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
#include "Model/BrushFace.h"
#include "Model/ModelFactory.h"

#include <cassert>
#include <string>

namespace TrenchBroom {
    namespace Model {
        BrushBuilder::BrushBuilder(const ModelFactory* factory, const vm::bbox3& worldBounds) :
        m_factory(factory),
        m_worldBounds(worldBounds),
        m_defaultAttribs(BrushFaceAttributes::NoTextureName) {
            ensure(m_factory != nullptr, "factory is null");
        }

        BrushBuilder::BrushBuilder(const ModelFactory* factory, const vm::bbox3& worldBounds, const BrushFaceAttributes& defaultAttribs) :
        m_factory(factory),
        m_worldBounds(worldBounds),
        m_defaultAttribs(defaultAttribs) {
            ensure(m_factory != nullptr, "factory is null");
        }

        Brush BrushBuilder::createCube(const FloatType size, const std::string& textureName) const {
            return createCuboid(vm::bbox3(size / 2.0), textureName, textureName, textureName, textureName, textureName, textureName);
        }

        Brush BrushBuilder::createCube(FloatType size, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const {
            return createCuboid(vm::bbox3(size / 2.0), leftTexture, rightTexture, frontTexture, backTexture, topTexture, bottomTexture);
        }
        
        Brush BrushBuilder::createCuboid(const vm::vec3& size, const std::string& textureName) const {
            return createCuboid(vm::bbox3(-size / 2.0, size / 2.0), textureName, textureName, textureName, textureName, textureName, textureName);
        }

        Brush BrushBuilder::createCuboid(const vm::vec3& size, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const {
            return createCuboid(vm::bbox3(-size / 2.0, size / 2.0), leftTexture, rightTexture, frontTexture, backTexture, topTexture, bottomTexture);
        }

        Brush BrushBuilder::createCuboid(const vm::bbox3& bounds, const std::string& textureName) const {
            return createCuboid(bounds, textureName, textureName, textureName, textureName, textureName, textureName);
        }

        Brush BrushBuilder::createCuboid(const vm::bbox3& bounds, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const {
            std::vector<BrushFace> faces;
            faces.reserve(6u);

            // left face
            faces.push_back(m_factory->createFace(
                bounds.min + vm::vec3::zero(),
                bounds.min + vm::vec3::pos_y(),
                bounds.min + vm::vec3::pos_z(),
                BrushFaceAttributes(leftTexture, m_defaultAttribs)));
            // right face
            faces.push_back(m_factory->createFace(
                bounds.max + vm::vec3::zero(),
                bounds.max + vm::vec3::pos_z(),
                bounds.max + vm::vec3::pos_y(),
                BrushFaceAttributes(rightTexture, m_defaultAttribs)));
            // front face
            faces.push_back(m_factory->createFace(
                bounds.min + vm::vec3::zero(),
                bounds.min + vm::vec3::pos_z(),
                bounds.min + vm::vec3::pos_x(),
                BrushFaceAttributes(frontTexture, m_defaultAttribs)));
            // back face
            faces.push_back(m_factory->createFace(
                bounds.max + vm::vec3::zero(),
                bounds.max + vm::vec3::pos_x(),
                bounds.max + vm::vec3::pos_z(),
                BrushFaceAttributes(backTexture, m_defaultAttribs)));
            // top face
            faces.push_back(m_factory->createFace(
                bounds.max + vm::vec3::zero(),
                bounds.max + vm::vec3::pos_y(),
                bounds.max + vm::vec3::pos_x(),
                BrushFaceAttributes(topTexture, m_defaultAttribs)));
            // bottom face
            faces.push_back(m_factory->createFace(
                bounds.min + vm::vec3::zero(),
                bounds.min + vm::vec3::pos_x(),
                bounds.min + vm::vec3::pos_y(),
                BrushFaceAttributes(bottomTexture, m_defaultAttribs)));

            return Brush(m_worldBounds, std::move(faces));
        }
        
        Brush BrushBuilder::createBrush(const std::vector<vm::vec3>& points, const std::string& textureName) const {
            return createBrush(Polyhedron3(points), textureName);
        }

        Brush BrushBuilder::createBrush(const Polyhedron3& polyhedron, const std::string& textureName) const {
            assert(polyhedron.closed());

            std::vector<BrushFace> brushFaces;

            const Polyhedron3::FaceList& faces = polyhedron.faces();
            Polyhedron3::FaceList::const_iterator fIt, fEnd;
            for (fIt = std::begin(faces), fEnd = std::end(faces); fIt != fEnd; ++fIt) {
                const Polyhedron3::Face* face = *fIt;
                const Polyhedron3::HalfEdgeList& boundary = face->boundary();

                Polyhedron3::HalfEdgeList::const_iterator bIt = std::begin(boundary);
                const Polyhedron3::HalfEdge* edge1 = *bIt++;
                const Polyhedron3::HalfEdge* edge2 = *bIt++;
                const Polyhedron3::HalfEdge* edge3 = *bIt++;

                const vm::vec3& p1 = edge1->origin()->position();
                const vm::vec3& p2 = edge2->origin()->position();
                const vm::vec3& p3 = edge3->origin()->position();

                brushFaces.push_back(m_factory->createFace(p1, p3, p2, textureName));
            }

            return Brush(m_worldBounds, std::move(brushFaces));
        }
    }
}
