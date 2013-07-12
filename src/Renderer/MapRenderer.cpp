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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapRenderer.h"

#include "Color.h"
#include "Preferences.h"
#include "Controller/NewDocumentCommand.h"
#include "Controller/OpenDocumentCommand.h"
#include "GL/GL.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        struct BuildBrushEdges {
            VertexSpecs::P3::Vertex::List vertices;
            inline void operator()(Model::Brush::Ptr brush) {
                brush->addEdges(vertices);
            }
        };
        
        struct BuildBrushFaceMesh {
            Model::BrushFace::Mesh mesh;
            inline void operator()(Model::BrushFace::Ptr face) {
                face->addToMesh(mesh);
            }
        };
        
        struct BuildBrushFaceMeshFilter {
            inline bool operator()(Model::Entity::Ptr entity) {
                return true;
            }
            
            inline bool operator()(Model::Brush::Ptr brush) {
                return true;
            }
            
            inline bool operator()(Model::BrushFace::Ptr face) {
                return true;
            }
        };
        
        MapRenderer::MapRenderer() :
        m_auxVbo(0xFFFF),
        m_edgeVbo(0xFFFF) {}
        
        void MapRenderer::render(RenderContext& context) {
            setupGL(context);
            
            clearBackground(context);
            renderCoordinateSystem(context);
            renderEdges(context);
        }

        void MapRenderer::commandDone(Controller::Command::Ptr command) {
            if (command->type() == Controller::NewDocumentCommand::Type) {
                clearState();
                Controller::NewDocumentCommand::Ptr newDocumentCommand = Controller::Command::cast<Controller::NewDocumentCommand>(command);
                Model::Map::Ptr map = newDocumentCommand->map();
                loadMap(map);
            } else if (command->type() == Controller::OpenDocumentCommand::Type) {
                clearState();
                Controller::OpenDocumentCommand::Ptr openDocumentCommand = Controller::Command::cast<Controller::OpenDocumentCommand>(command);
                Model::Map::Ptr map = openDocumentCommand->map();
                loadMap(map);
            }
        }
        
        void MapRenderer::commandUndone(Controller::Command::Ptr command) {
        }

        void MapRenderer::setupGL(RenderContext& context) {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glFrontFace(GL_CW);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glShadeModel(GL_SMOOTH);
            // glResetEdgeOffset();
            
            const Camera& camera = context.camera();
            const Camera::Viewport& viewport = camera.viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
        }

        void MapRenderer::clearBackground(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
            glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        void MapRenderer::renderCoordinateSystem(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& xAxisColor = prefs.getColor(Preferences::XAxisColor);
            const Color& yAxisColor = prefs.getColor(Preferences::YAxisColor);
            const Color& zAxisColor = prefs.getColor(Preferences::ZAxisColor);

            typedef VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices;
            
            vertices.push_back(Vertex(Vec3f(-128.0f, 0.0f, 0.0f), xAxisColor));
            vertices.push_back(Vertex(Vec3f( 128.0f, 0.0f, 0.0f), xAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, -128.0f, 0.0f), yAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f,  128.0f, 0.0f), yAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, 0.0f, -128.0f), zAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, 0.0f,  128.0f), zAxisColor));
            
            VertexArray array(m_auxVbo, GL_LINES, vertices);
            
            SetVboState setVboState(m_auxVbo);
            setVboState.active();
            array.render();
        }

        void MapRenderer::renderEdges(RenderContext& context) {
            ShaderManager& shaderManager = context.shaderManager();
            ActiveShader edgeShader(shaderManager, Shaders::EdgeShader);
            edgeShader.set("Color", Color(1.0f, 1.0f, 1.0f, 1.0f));
            
            SetVboState activateVbo(m_edgeVbo);
            activateVbo.active();
            m_edgeArray.render();
        }

        void MapRenderer::clearState() {
            m_edgeArray = VertexArray();
        }

        void MapRenderer::loadMap(Model::Map::Ptr map) {
            BuildBrushFaceMeshFilter filter;

            BuildBrushFaceMesh buildFaces;
            map->eachBrushFace(buildFaces, filter);
            
            BuildBrushEdges buildEdges;
            map->eachBrush(buildEdges, filter);
            
            m_edgeArray = VertexArray(m_edgeVbo, GL_LINES, buildEdges.vertices);
        }
    }
}
