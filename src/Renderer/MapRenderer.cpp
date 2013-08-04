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
#include "Controller/SelectionCommand.h"
#include "GL/GL.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Renderer/Camera.h"
#include "Renderer/Mesh.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        MapRenderer::MapRenderer(FontManager& fontManager, const Model::Filter& filter) :
        m_fontManager(fontManager),
        m_filter(filter),
        m_auxVbo(0xFFFFF),
        m_brushRenderer(m_filter),
        m_entityRenderer(m_fontManager, m_filter) {}
        
        void MapRenderer::render(RenderContext& context) {
            setupGL(context);
            
            clearBackground(context);
            renderCoordinateSystem(context);
            renderGeometry(context);
            renderEntities(context);
        }

        void MapRenderer::commandDone(Controller::Command::Ptr command) {
            if (command->type() == Controller::NewDocumentCommand::Type) {
                clearState();
                Controller::NewDocumentCommand::Ptr newDocumentCommand = Controller::Command::cast<Controller::NewDocumentCommand>(command);
                Model::Map* map = newDocumentCommand->map();
                loadMap(*map);
            } else if (command->type() == Controller::OpenDocumentCommand::Type) {
                clearState();
                Controller::OpenDocumentCommand::Ptr openDocumentCommand = Controller::Command::cast<Controller::OpenDocumentCommand>(command);
                Model::Map* map = openDocumentCommand->map();
                loadMap(*map);
            } else if (command->type() == Controller::SelectionCommand::Type) {
                m_brushRenderer.invalidate();
                m_entityRenderer.invalidateBounds();
            }
        }
        
        void MapRenderer::commandUndone(Controller::Command::Ptr command) {
            if (command->type() == Controller::SelectionCommand::Type) {
                m_brushRenderer.invalidate();
                m_entityRenderer.invalidateBounds();
            }
        }

        void MapRenderer::setupGL(RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
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

        void MapRenderer::renderGeometry(RenderContext& context) {
            m_brushRenderer.render(context);
        }

        void MapRenderer::renderEntities(RenderContext& context) {
            m_entityRenderer.render(context);
        }

        void MapRenderer::clearState() {
            m_brushRenderer.clear();
            m_entityRenderer.clear();
        }

        void MapRenderer::loadMap(Model::Map& map) {
            m_brushRenderer.addBrushes(map.brushes());
            m_entityRenderer.addEntities(map.entities());
        }
    }
}
