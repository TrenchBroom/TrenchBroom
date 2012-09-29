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

#ifndef TrenchBroom_EntityRendererManager_h
#define TrenchBroom_EntityRendererManager_h

#include "Utility/String.h"

#include <map>
#include <vector>
#include <set>

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class PointEntityDefinition;
        class PointEntityModel;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        class EntityRenderer;
        class Palette;
        class Vbo;
        
        class EntityRendererManager {
        private:
            typedef std::map<String, EntityRenderer*> EntityRendererCache;
            typedef std::set<String> MismatchCache;
            
            String m_quakePath;
            const Palette* m_palette;
            Utility::Console& m_console;
            
            Vbo* m_vbo;
            EntityRendererCache m_entityRenderers;
            MismatchCache m_mismatches;
            bool m_valid;

            const String entityRendererKey(const Model::PointEntityModel& modelInfo, const StringList& searchPaths);
            EntityRenderer* entityRenderer(const Model::PointEntityModel& modelInfo, const StringList& mods);
        public:
            EntityRendererManager(Utility::Console& console);
            ~EntityRendererManager();
            
            EntityRenderer* entityRenderer(const Model::PointEntityDefinition& entityDefinition, const StringList& mods);
            EntityRenderer* entityRenderer(const Model::Entity& entity, const StringList& mods);
            void clear();
            
            void setQuakePath(const String& quakePath);
            void setPalette(const Palette& palette);
            
            void activate();
            void deactivate();
        };
    }
}

#endif
