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

#ifndef TrenchBroom_ModelFactory
#define TrenchBroom_ModelFactory

#include "FloatType.h"

#include <kdl/result_forward.h>

#include <vecmath/forward.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        enum class BrushError;
        class BrushFace;
        class BrushNode;
        class BrushFaceAttributes;
        class Entity;
        class EntityNode;
        class GroupNode;
        class LayerNode;
        enum class MapFormat;
        class WorldNode;

        class ModelFactory {
        public:
            virtual ~ModelFactory();

            MapFormat format() const;
            WorldNode* createWorld(Entity entity) const;
            LayerNode* createLayer(const std::string& name) const;
            GroupNode* createGroup(const std::string& name) const;
            EntityNode* createEntity(Entity entity) const;
            BrushNode* createBrush(Brush brush) const;

            /**
             * Creates a face using TB's default texture projection for the current map format
             * returned by format() and the given plane.
             *
             * Used when creating new faces when we don't have a particular texture alignment to request.
             * On Valve format maps, this differs from createFaceFromStandard() by creating a face-aligned texture projection,
             * whereas createFaceFromStandard() creates an axis-aligned texture projection.
             *
             * The returned face has a TexCoordSystem matching format().
             */
            kdl::result<BrushFace, BrushError> createFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const;
            /**
             * Creates a face from a Standard texture projection, converting it to Valve if necessary.
             *
             * Used when loading/pasting a Standard format map.
             *
             * The returned face has a TexCoordSystem matching format().
             */
            kdl::result<BrushFace, BrushError> createFaceFromStandard(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const;
            /**
             * Creates a face from a Valve texture projection, converting it to Standard if necessary.
             *
             * Used when loading/pasting a Valve format map.
             *
             * The returned face has a TexCoordSystem matching format().
             */
            kdl::result<BrushFace, BrushError> createFaceFromValve(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const;
        private:
            virtual MapFormat doGetFormat() const = 0;
            virtual WorldNode* doCreateWorld(Entity entity) const = 0;
            virtual LayerNode* doCreateLayer(const std::string& name) const = 0;
            virtual GroupNode* doCreateGroup(const std::string& name) const = 0;
            virtual EntityNode* doCreateEntity(Entity entity) const = 0;
            virtual BrushNode* doCreateBrush(Brush brush) const;
            virtual kdl::result<BrushFace, BrushError> doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const = 0;
            virtual kdl::result<BrushFace, BrushError> doCreateFaceFromStandard(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const = 0;
            virtual kdl::result<BrushFace, BrushError> doCreateFaceFromValve(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_ModelFactory) */
