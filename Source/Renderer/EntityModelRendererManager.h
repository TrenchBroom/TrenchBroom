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

#ifndef TrenchBroom_EntityModelRendererManager_h
#define TrenchBroom_EntityModelRendererManager_h

#include "Utility/String.h"

#include <map>
#include <vector>
#include <set>

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class PointEntityDefinition;
        class ModelDefinition;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        class EntityModelRenderer;
        class Palette;
        class Vbo;
        
        class EntityModelRendererManager {
        private:
            typedef std::map<String, EntityModelRenderer*> EntityModelRendererCache;
            typedef std::set<String> MismatchCache;
            
            const Palette* m_palette;
            Utility::Console& m_console;
            
            Vbo* m_vbo;
            EntityModelRendererCache m_modelRenderers;
            MismatchCache m_mismatches;
            bool m_valid;

            const String modelRendererKey(const Model::ModelDefinition& modelDefinition, const StringList& searchPaths);
            EntityModelRenderer* modelRenderer(const Model::ModelDefinition& modelDefinition, const StringList& searchPaths);
        public:
            EntityModelRendererManager(Utility::Console& console);
            ~EntityModelRendererManager();
            
            EntityModelRenderer* modelRenderer(const Model::PointEntityDefinition& entityDefinition, const StringList& searchPaths);
            EntityModelRenderer* modelRenderer(const Model::Entity& entity, const StringList& searchPaths);
            void clear();
            void clearMismatches();
            
            void setPalette(const Palette& palette);
            
            void activate();
            void deactivate();
        };
    }
}

#endif
