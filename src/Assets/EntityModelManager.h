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

#ifndef __TrenchBroom__EntityModelManager__
#define __TrenchBroom__EntityModelManager__

#include "Assets/ModelDefinition.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"
#include "Renderer/Vbo.h"

#include <map>
#include <set>

namespace TrenchBroom {
    namespace Renderer {
        class MeshRenderer;
    }
    
    namespace Assets {
        class EntityModel;
        
        class EntityModelManager {
        private:
            typedef std::map<IO::Path, EntityModel*> ModelCache;
            typedef std::set<IO::Path> ModelMismatches;
            
            typedef std::map<Assets::ModelSpecification, Renderer::MeshRenderer*> RendererCache;
            typedef std::set<Assets::ModelSpecification> RendererMismatches;
            
            Model::GamePtr m_game;
            mutable Renderer::Vbo m_vbo;

            mutable ModelCache m_models;
            mutable ModelMismatches m_modelMismatches;
            mutable RendererCache m_renderers;
            mutable RendererMismatches m_rendererMismatches;
            mutable bool m_prepared;
        public:
            EntityModelManager();
            ~EntityModelManager();
            
            void clear();
            void reset(Model::GamePtr game);
            EntityModel* model(const IO::Path& path) const;
            Renderer::MeshRenderer* renderer(const Assets::ModelSpecification& spec) const;
            void activateVbo();
            void deactivateVbo();
        private:
            void prepareRenderers();
        };
    }
}

#endif /* defined(__TrenchBroom__EntityModelManager__) */
