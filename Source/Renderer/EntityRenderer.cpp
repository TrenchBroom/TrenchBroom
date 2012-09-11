/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityRenderer.h"

#include "Model/Entity.h"
#include "Renderer/Shader/Shader.h"
#include "Utility/GLee.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        void EntityRenderer::render(ShaderProgram& shaderProgram, const Model::Entity& entity) {
            render(shaderProgram, entity.origin(), static_cast<float>(entity.angle()));
        }

        void EntityRenderer::render(ShaderProgram& shaderProgram, const Vec3f& position, float angle) {
            Mat4f matrix;
            matrix.translate(position);
            
            if (angle != 0.0f) {
                if (angle == -1.0f)
                    matrix.rotate(90.0f, Vec3f::PosX);
                else if (angle == -2.0f)
                    matrix.rotate(-90.0f, Vec3f::PosX);
                else
                    matrix.rotate(angle, Vec3f::PosZ);
            }

            shaderProgram.setUniformVariable("Transformation", matrix);
            render(shaderProgram);
        }
    }
}