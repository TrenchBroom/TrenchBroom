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

#include "EdgeRenderer.h"

#include "GL/glew.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Face.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"

namespace TrenchBroom {
    namespace Renderer {
        unsigned int EdgeRenderer::vertexCount(const Model::BrushList& brushes, const Model::FaceList& faces) {
            Model::BrushList::const_iterator brushIt, brushEnd;
            Model::FaceList::const_iterator faceIt, faceEnd;
            unsigned int vertexCount = 0;
            
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush& brush = **brushIt;
                vertexCount += brush.edges().size();
            }
            
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                const Model::Face& face = **faceIt;
                vertexCount += face.edges().size();
            }
            
            return 2 * vertexCount;
        }
        
        void EdgeRenderer::writeEdgeData(Vbo& vbo, const Model::BrushList& brushes, const Model::FaceList& faces) {
            m_vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_LINES, vertexCount(brushes, faces),
                                                           VertexAttribute::position3f()));

            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush& brush = **brushIt;
                const Model::EdgeList& edges = brush.edges();
                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::Edge& edge = **edgeIt;
                    m_vertexArray->addAttribute(edge.start->position);
                    m_vertexArray->addAttribute(edge.end->position);
                }
            }
            
            Model::FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                const Model::Face& face = **faceIt;
                const Model::EdgeList& edges = face.edges();
                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::Edge& edge = **edgeIt;
                    m_vertexArray->addAttribute(edge.start->position);
                    m_vertexArray->addAttribute(edge.end->position);
                }
            }
        }
        
        void EdgeRenderer::writeEdgeData(Vbo& vbo, const Model::BrushList& brushes, const Model::FaceList& faces, const Color& defaultColor) {
            m_vertexArray = VertexArrayPtr(new VertexArray(vbo, GL_LINES, vertexCount(brushes, faces),
                                                           VertexAttribute::position3f(),
                                                           VertexAttribute::color4f()));

            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush& brush = **brushIt;
                const Model::Entity* entity = brush.entity();
                const Model::EntityDefinition* definition = entity != NULL ? entity->definition() : NULL;
                const Color& color = (entity != NULL && !entity->worldspawn() && definition != NULL && definition->type() == Model::EntityDefinition::BrushEntity) ? definition->color() : defaultColor;
                
                const Model::EdgeList& edges = brush.edges();
                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::Edge& edge = **edgeIt;
                    m_vertexArray->addAttribute(edge.start->position);
                    m_vertexArray->addAttribute(color);
                    m_vertexArray->addAttribute(edge.end->position);
                    m_vertexArray->addAttribute(color);
                }
            }

            Model::FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                const Model::Face& face = **faceIt;
                const Model::Brush* brush = face.brush();
                const Model::Entity* entity = brush->entity();
                const Model::EntityDefinition* definition = entity != NULL ? entity->definition() : NULL;
                const Color& color = (entity != NULL && !entity->worldspawn() && definition != NULL && definition->type() == Model::EntityDefinition::BrushEntity) ? definition->color() : defaultColor;
                
                const Model::EdgeList& edges = face.edges();
                Model::EdgeList::const_iterator edgeIt, edgeEnd;
                for (edgeIt = edges.begin(), edgeEnd = edges.end(); edgeIt != edgeEnd; ++edgeIt) {
                    const Model::Edge& edge = **edgeIt;
                    m_vertexArray->addAttribute(edge.start->position);
                    m_vertexArray->addAttribute(color);
                    m_vertexArray->addAttribute(edge.end->position);
                    m_vertexArray->addAttribute(color);
                }
            }
        }

        EdgeRenderer::EdgeRenderer(Vbo& vbo, const Model::BrushList& brushes, const Model::FaceList& faces) {
            writeEdgeData(vbo, brushes, faces);
        }
        
        EdgeRenderer::EdgeRenderer(Vbo& vbo, const Model::BrushList& brushes, const Model::FaceList& faces, const Color& defaultColor) {
            writeEdgeData(vbo, brushes, faces, defaultColor);
        }


        void EdgeRenderer::render(RenderContext& context) {
            ShaderManager& shaderManager = context.shaderManager();
            ShaderProgram& coloredEdgeProgram = shaderManager.shaderProgram(Shaders::ColoredEdgeShader);
            if (coloredEdgeProgram.activate()) {
                m_vertexArray->render();
                coloredEdgeProgram.deactivate();
            }
        }
        
        void EdgeRenderer::render(RenderContext& context, const Color& color) {
            ShaderManager& shaderManager = context.shaderManager();
            ShaderProgram& edgeProgram = shaderManager.shaderProgram(Shaders::EdgeShader);
            if (edgeProgram.activate()) {
                edgeProgram.setUniformVariable("Color", color);
                m_vertexArray->render();
                edgeProgram.deactivate();
            }
        }
    }
}