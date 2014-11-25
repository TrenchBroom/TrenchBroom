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
#include "Renderer/Renderable.h"

#include <map>
#include <set>

namespace TrenchBroom {
    namespace Assets {
        class EntityModel;
        class EntityModelManager;
    }
    
    namespace Model {
        class EditorContext;
        class Entity;
    }
    
    namespace Renderer {
        class TexturedTriangleMeshRenderer;
        class RenderBatch;
        class RenderContext;
        
        class EntityModelRenderer : public Renderable {
        private:
            typedef std::map<Model::Entity*, TexturedTriangleMeshRenderer*> EntityMap;
            typedef std::vector<Assets::EntityModel*> EntityModelList;
            typedef std::vector<TexturedTriangleMeshRenderer*> RendererList;
            
            Assets::EntityModelManager& m_entityModelManager;
            const Model::EditorContext& m_editorContext;
            
            EntityMap m_entities;
            EntityModelList m_unpreparedModels;
            RendererList m_unpreparedRenderers;
            
            bool m_applyTinting;
            Color m_tintColor;
            
            bool m_showHiddenEntities;
        public:
            EntityModelRenderer(Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext);
            ~EntityModelRenderer();
            
            void addEntity(Model::Entity* entity);
            void updateEntity(Model::Entity* entity);
            void removeEntity(Model::Entity* entity);
            
            template <typename I>
            void addEntities(I cur, I end) {
                while (cur != end) {
                    addEntity(*cur);
                    ++cur;
                }
            }
            
            template <typename I>
            void updateEntities(I cur, I end) {
                while (cur != end) {
                    updateEntity(*cur);
                    ++cur;
                }
            }
            
            template <typename I>
            void removeEntities(I cur, I end) {
                while (cur != end) {
                    removeEntity(*cur);
                    ++cur;
                }
            }
            
            void clear();
            
            bool applyTinting() const;
            void setApplyTinting(const bool applyTinting);
            const Color& tintColor() const;
            void setTintColor(const Color& tintColor);
            
            bool showHiddenEntities() const;
            void setShowHiddenEntities(bool showHiddenEntities);
            
            void render(RenderBatch& renderBatch);
        private:
            void doPrepare(Vbo& vbo);
            void prepareModels();
            void prepareRenderers(Vbo& vbo);
            
            class MeshFunc;
            void doRender(RenderContext& renderContext);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityModelRenderer__) */
