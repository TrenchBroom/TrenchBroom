/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__EntityModelRenderer__
#define __TrenchBroom__EntityModelRenderer__

#include "Color.h"
#include "Assets/ModelDefinition.h"
#include "Model/ModelTypes.h"
#include "Renderer/Vbo.h"

#include <map>
#include <set>

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class ModelFilter;
    }
    
    namespace Renderer {
        class MeshRenderer;
        class RenderContext;
        
        class EntityModelRenderer {
        private:
            typedef std::map<Assets::ModelSpecification, MeshRenderer*> RendererCache;
            typedef std::map<Model::Entity*, MeshRenderer*> EntityMap;
            typedef std::set<Assets::ModelSpecification> MismatchCache;
            
            const Model::ModelFilter& m_filter;
            Vbo::Ptr m_vbo;
            RendererCache m_renderers;
            EntityMap m_entities;
            MismatchCache m_mismatches;
            
            bool m_applyTinting;
            Color m_tintColor;
        public:
            EntityModelRenderer(const Model::ModelFilter& filter);
            ~EntityModelRenderer();
            
            void addEntity(Model::Entity* entity);
            void addEntities(const Model::EntityList& entities);
            void updateEntity(Model::Entity* entity);
            void updateEntities(const Model::EntityList& entities);
            void removeEntity(Model::Entity* entity);
            void removeEntities(const Model::EntityList& entities);
            void clear();
            
            bool applyTinting() const;
            void setApplyTinting(const bool applyTinting);
            const Color& tintColor() const;
            void setTintColor(const Color& tintColor);
            
            void render(RenderContext& context);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityModelRenderer__) */
