/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include <map>
#include <set>

namespace TrenchBroom {
    namespace Assets {
        class EntityModelManager;
    }
    
    namespace Model {
        class Entity;
        class ModelFilter;
    }
    
    namespace Renderer {
        class MeshRenderer;
        class RenderContext;
        
        class EntityModelRenderer {
        private:
            typedef std::map<Model::Entity*, MeshRenderer*> EntityMap;
            
            Assets::EntityModelManager& m_entityModelManager;
            const Model::ModelFilter& m_filter;
            EntityMap m_entities;
            
            bool m_applyTinting;
            Color m_tintColor;
        public:
            EntityModelRenderer(Assets::EntityModelManager& entityModelManager, const Model::ModelFilter& filter);
            ~EntityModelRenderer();
            
            void addEntity(Model::Entity* entity);
            void addEntities(const Model::EntityList& entities);
            void addEntities(const Model::EntitySet& entities);
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
