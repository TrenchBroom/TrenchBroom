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

#ifndef TrenchBroom_EntityFigure_h
#define TrenchBroom_EntityFigure_h

#include "Renderer/Figures/Figure.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Controller {
        class Editor;
    }
    
    namespace Model {
        class EntityDefinition;
    }
    
    namespace Renderer {
        class EntityRenderer;
        class EntityRendererManager;
        class Vbo;
        class VboBlock;
        
        class EntityFigure : public Figure {
        protected:
            Model::EntityDefinition& m_entityDefinition;
            bool m_renderBounds;
            bool m_valid;
            VboBlock* m_boundsBlock;
            unsigned int m_vertexCount;
            
            Controller::Editor& m_editor;
            EntityRenderer* m_entityRenderer;
            Vec3f m_position;
        public:
            EntityFigure(Controller::Editor& editor, Model::EntityDefinition& entityDefinition, bool renderBounds);
            virtual ~EntityFigure();

            virtual void setPosition(const Vec3f& position);
            virtual void render(RenderContext& context, Vbo& vbo);
        };
    }
}

#endif
