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

#include "EntityRotationDecorator.h"

#include "Model/Entity.h"
#include "Model/Map.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        EntityRotationDecorator::EntityRotationDecorator(const Model::Map& map, const Color& color) :
        EntityDecorator(map),
        m_color(color),
        m_vertexArray(NULL),
        m_valid(false) {}

        void EntityRotationDecorator::render(Vbo& vbo, RenderContext& context) {
            const Model::EntityList& entities = map().entities();
            if (entities.empty())
                return;
            
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (!m_valid || m_vertexArray == NULL) {
                delete m_vertexArray;
                
                unsigned int vertexCount = static_cast<unsigned int>(entities.size() * 2);
                m_vertexArray = new VertexArray(vbo, GL_LINES, vertexCount, Attribute::position3f());
                
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                
                Model::EntityList::const_iterator it, end;
                for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                    const Model::Entity& entity = **it;
                    const Vec3f direction = entity.rotation() * Vec3f::PosX;
                    const Vec3f& center = entity.center();
                    m_vertexArray->addAttribute(center);
                    m_vertexArray->addAttribute(center + 32.0f * direction);
                }
                
                m_valid = true;
            }
            
            ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
            shader.currentShader().setUniformVariable("Color", m_color);
            m_vertexArray->render();
        }
    }
}
