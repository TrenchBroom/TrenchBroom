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

#include "CreateEntityFromMenuHelper.h"

#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Renderer/EntityFigure.h"
#include "Renderer/RenderContext.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Controller {
        CreateEntityFromMenuHelper::CreateEntityFromMenuHelper(Model::MapDocument& document) :
        m_document(document),
        m_entity(NULL),
        m_figure(NULL) {}
        
        CreateEntityFromMenuHelper::~CreateEntityFromMenuHelper() {
            delete m_figure;
            m_figure = NULL;
            delete m_entity;
            m_entity = NULL;
        }
        
        void CreateEntityFromMenuHelper::show(Model::PointEntityDefinition& definition, const Vec3f& origin) {
            if (m_entity != NULL && m_entity->definition() != &definition) {
                delete m_figure;
                m_figure = NULL;
                delete m_entity;
                m_entity = NULL;
            }
            
            if (m_entity == NULL) {
                m_entity = new Model::Entity(m_document.map().worldBounds());
                m_entity->setProperty(Model::Entity::ClassnameKey, definition.name());
                m_entity->setDefinition(&definition);
                m_entity->transform(Mat4f::Identity.translated(origin), Mat4f::Identity, false, false);
            }
        }
        
        void CreateEntityFromMenuHelper::hide() {
            delete m_figure;
            m_figure = NULL;
            delete m_entity;
            m_entity = NULL;
        }
        
        void CreateEntityFromMenuHelper::render(Renderer::Vbo& vbo, Renderer::RenderContext& context) {
            if (m_entity == NULL)
                return;
            
            if (m_figure == NULL)
                m_figure = new Renderer::EntityFigure(m_document, *m_entity);
            m_figure->render(vbo, context);
        }
    }
}
