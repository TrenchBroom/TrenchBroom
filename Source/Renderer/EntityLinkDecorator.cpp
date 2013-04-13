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

#include "EntityLinkDecorator.h"

#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/Console.h"
#include "Utility/List.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        void EntityLinkDecorator::makeLink(Model::Entity& source, Model::Entity& target, Vec3f::List& vertices) const {
            vertices.push_back(source.center());
            vertices.push_back(target.center());
        }

        void EntityLinkDecorator::buildLinks(RenderContext& context, Model::Entity& entity, size_t depth, Model::EntitySet& visitedEntities, Vec3f::List& selectedLinks, Vec3f::List& unselectedLinks) const {
            if (context.viewOptions().linkDisplayMode() == View::ViewOptions::LinkDisplayLocal && depth > 1)
                return;

            Model::EntitySet::iterator visitedIt = visitedEntities.lower_bound(&entity);
            if (visitedIt != visitedEntities.end() && *visitedIt == &entity)
                return;

            visitedEntities.insert(visitedIt, &entity);
            const bool entityVisible = context.filter().entityVisible(entity);

            Model::EntityList::const_iterator entityIt, entityEnd;
            
            const Model::EntityList& linkTargets = entity.linkTargets();
            for (entityIt = linkTargets.begin(), entityEnd = linkTargets.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& target = **entityIt;
                if (entityVisible && context.filter().entityVisible(target) &&
                    (context.viewOptions().linkDisplayMode() != View::ViewOptions::LinkDisplayLocal || entity.selected() || entity.partiallySelected() || target.selected() || target.partiallySelected()))
                    makeLink(target, entity, entity.selected() || entity.partiallySelected() || target.selected() || target.partiallySelected() ? selectedLinks : unselectedLinks);
                if (context.viewOptions().linkDisplayMode() != View::ViewOptions::LinkDisplayLocal || depth == 0)
                    buildLinks(context, target, depth + 1, visitedEntities, selectedLinks, unselectedLinks);
            }
            
            const Model::EntityList& linkSources = entity.linkSources();
            for (entityIt = linkSources.begin(), entityEnd = linkSources.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& source = **entityIt;
                if (context.viewOptions().linkDisplayMode() != View::ViewOptions::LinkDisplayLocal || depth <= 1)
                    buildLinks(context, source, depth + 1, visitedEntities, selectedLinks, unselectedLinks);
            }
            
            const Model::EntityList& killTargets = entity.killTargets();
            for (entityIt = killTargets.begin(), entityEnd = killTargets.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& target = **entityIt;
                if (entityVisible && context.filter().entityVisible(target) &&
                    (context.viewOptions().linkDisplayMode() != View::ViewOptions::LinkDisplayLocal || entity.selected() || entity.partiallySelected() || target.selected() || target.partiallySelected()))
                    makeLink(target, entity, entity.selected() || entity.partiallySelected() || target.selected() || target.partiallySelected() ? selectedLinks : unselectedLinks);
                if (context.viewOptions().linkDisplayMode() != View::ViewOptions::LinkDisplayLocal || depth == 0)
                    buildLinks(context, target, depth + 1, visitedEntities, selectedLinks, unselectedLinks);
            }
            
            const Model::EntityList& killSources = entity.killSources();
            for (entityIt = killSources.begin(), entityEnd = killSources.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& source = **entityIt;
                if (context.viewOptions().linkDisplayMode() != View::ViewOptions::LinkDisplayLocal || depth <= 1)
                    buildLinks(context, source, depth + 1, visitedEntities, selectedLinks, unselectedLinks);
            }
        }

        EntityLinkDecorator::EntityLinkDecorator(const Model::MapDocument& document, const Color& color) :
        EntityDecorator(document),
        m_color(color),
        m_selectedLinkArray(NULL),
        m_unselectedLinkArray(NULL),
        m_valid(false) {}

        EntityLinkDecorator::~EntityLinkDecorator() {
            delete m_selectedLinkArray;
            m_selectedLinkArray = NULL;
            delete m_unselectedLinkArray;
            m_unselectedLinkArray = NULL;
        }

        void EntityLinkDecorator::render(Vbo& vbo, RenderContext& context) {
            if (context.viewOptions().linkDisplayMode() == View::ViewOptions::LinkDisplayNone)
                return;

            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (!m_valid) {
                delete m_selectedLinkArray;
                m_selectedLinkArray = NULL;
                delete m_unselectedLinkArray;
                m_unselectedLinkArray = NULL;

                Vec3f::List selectedVertices, unselectedVertices;
                Model::EntitySet visitedEntities;

                const Model::EntityList& entities = context.viewOptions().linkDisplayMode() == View::ViewOptions::LinkDisplayAll ? document().map().entities() : document().editStateManager().allSelectedEntities();
                Model::EntityList::const_iterator it, end;
                for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                    Model::Entity& entity = **it;
                    buildLinks(context, entity, 0, visitedEntities, selectedVertices, unselectedVertices);
                }
                
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                if (!selectedVertices.empty()) {
                    m_selectedLinkArray = new VertexArray(vbo, GL_LINES, static_cast<unsigned int>(selectedVertices.size()), Attribute::position3f(), 0);
                    m_selectedLinkArray->addAttributes(selectedVertices);
                }
                
                if (!unselectedVertices.empty()) {
                    m_unselectedLinkArray = new VertexArray(vbo, GL_LINES, static_cast<unsigned int>(unselectedVertices.size()), Attribute::position3f(), 0);
                    m_unselectedLinkArray->addAttributes(unselectedVertices);
                }

                m_valid = true;
            }

            if (m_selectedLinkArray == NULL && m_unselectedLinkArray == NULL)
                return;

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

            ActivateShader shader(context.shaderManager(), Shaders::EntityLinkShader);
            shader.currentShader().setUniformVariable("CameraPosition", context.camera().position());
            shader.currentShader().setUniformVariable("MaxDistance", 512.0f);

            // render the "occluded" portion without depth-test
            glLineWidth(2.0f);
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);

            if (m_unselectedLinkArray != NULL) {
                shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::OccludedEntityLinkColor));
                m_unselectedLinkArray->render();
            }
            
            if (m_selectedLinkArray != NULL) {
                shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::OccludedSelectedEntityLinkColor));
                m_selectedLinkArray->render();
            }
            
            glEnable(GL_DEPTH_TEST);

            if (m_unselectedLinkArray != NULL) {
                shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::EntityLinkColor));
                m_unselectedLinkArray->render();
            }

            if (m_selectedLinkArray != NULL) {
                shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::SelectedEntityLinkColor));
                m_selectedLinkArray->render();
            }
            
            glDepthMask(GL_TRUE);
            glLineWidth(1.0f);
        }
    }
}
