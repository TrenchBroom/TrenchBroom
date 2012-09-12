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

#ifndef __TrenchBroom__PathRenderer__
#define __TrenchBroom__PathRenderer__

#include "Renderer/RenderTypes.h"
#include "Renderer/Text/Path.h"
#include "Renderer/Text/PathMesh.h"
#include "Utility/GLee.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class Vbo;
        class VboBlock;
        
        namespace Text {
            class PathTesselator;

            class PathRenderer {
            protected:
                typedef std::vector<GLint> IndexBuffer;
                typedef std::vector<GLsizei> CountBuffer;

                PathPtr m_path;
                
                float m_width;
                float m_height;

                VertexArrayPtr m_vertexArray;
                
                void uploadMeshData(PathMeshPtr mesh, Vbo& vbo);
            public:
                PathRenderer(PathPtr path);
                
                inline float width() const {
                    return m_width;
                }
                
                inline float height() const {
                    return m_height;
                }
                
                inline bool prepared() const {
                    return m_path.get() == NULL;
                }
                
                bool prepare(PathTesselator& tesselator, Vbo& vbo);
                void render();
            };
        }
    }
}

#endif /* defined(__TrenchBroom__PathRenderer__) */
