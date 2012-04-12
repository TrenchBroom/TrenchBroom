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

#ifndef TrenchBroom_TransientOptions_h
#define TrenchBroom_TransientOptions_h

namespace TrenchBroom {
    namespace Controller {

        typedef enum {
            RM_TEXTURED,
            RM_FLAT,
            RM_WIREFRAME
        } ERenderMode;
        
        typedef enum {
            IM_NONE, // no isolation
            IM_WIREFRAME, // render unselected geometry as wireframe, ignore while picking
            IM_DISCARD // do not render unselected geometry, ignore while picking
        } EIsolationMode;

        class TransientOptions {
        public:
            ERenderMode renderMode;
            EIsolationMode isolationMode;
            bool renderEntities;
            bool renderEntityClassnames;
            bool renderBrushes;
            bool renderOrigin;
            bool renderGrid;
            float originAxisLength;
            bool renderSizeGuides;
            bool lockTextures;
            TransientOptions() : renderMode(RM_TEXTURED), isolationMode(IM_NONE), renderEntities(true), renderEntityClassnames(true), renderBrushes(true), renderOrigin(true), renderGrid(true), originAxisLength(64), renderSizeGuides(true), lockTextures(true) {}
        };
    }
}

#endif
