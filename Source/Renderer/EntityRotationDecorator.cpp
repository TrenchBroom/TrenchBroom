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

#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/Console.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"
#include "View/ViewOptions.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        EntityRotationDecorator::EntityRotationDecorator(const Model::MapDocument& document, const Color& fillColor, const Color& outlineColor) :
        EntityDecorator(document),
        m_fillColor(fillColor),
        m_outlineColor(outlineColor) {}
        
        void EntityRotationDecorator::render(Vbo& vbo, RenderContext& context) {
            if (!context.viewOptions().showEntities() || !context.viewOptions().showEntityBounds())
                return;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList entities = editStateManager.allSelectedEntities();
            if (entities.empty())
                return;
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            static const float maxDistance2 = prefs.getFloat(Preferences::SelectedInfoOverlayFadeDistance) * prefs.getFloat(Preferences::SelectedInfoOverlayFadeDistance);
            
            Vec2f t1, t2, t3;
            arrowHead(12.0f, 6.0f, t1, t2, t3);
            Vec3f triangle[3];
            triangle[0] = Vec3f(t1, 0.0f);
            triangle[1] = Vec3f(t2, 0.0f);
            triangle[2] = Vec3f(t3, 0.0f);
            
            Vec3f::List vertices;
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                const Model::Entity& entity = **it;
                if (entity.rotated() && context.filter().entityVisible(entity)) {
                    const Vec3f direction = entity.rotation() * Vec3f::PosX;
                    const Vec3f& center = entity.center();
                    const Planef plane(direction, center);
                    const Vec3f toCam = center - context.camera().position();
                    if (toCam.lengthSquared() > maxDistance2)
                        continue;
                    Vec3f onPlane = plane.project(toCam);
                    if (onPlane.null())
                        continue;
                    onPlane.normalize();
                    
                    const Vec3f rotZ = entity.rotation() * Vec3f::NegZ;
                    const float angle = angleFrom(rotZ, onPlane, direction);
                    
                    const Mat4f matrix = translationMatrix(center) * rotationMatrix(angle, -direction) * rotationMatrix(entity.rotation()) * translationMatrix(16.0f * Vec3f::PosX);
                    for (size_t i = 0; i < 3; i++)
                        vertices.push_back(matrix * triangle[i]);
                }
            }
            
            unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
            if (vertexCount == 0)
                return;
            
            VertexArray vertexArray(vbo, GL_TRIANGLES, vertexCount, Attribute::position3f(), 0);
            SetVboState mapVbo(vbo, Vbo::VboMapped);
            vertexArray.addAttributes(vertices);
            
            SetVboState activateVbo(vbo, Vbo::VboActive);
            ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
            
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT, GL_LINE);
            shader.setUniformVariable("Color", Color(m_outlineColor));
            vertexArray.render();
            glPolygonMode(GL_FRONT, GL_FILL);
            shader.setUniformVariable("Color", Color(m_fillColor));
            vertexArray.render();
            glDepthMask(GL_TRUE);
        }
    }
}
