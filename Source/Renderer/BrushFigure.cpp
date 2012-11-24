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

#include "BrushFigure.h"

#include "Model/Brush.h"
#include "Model/Face.h"
#include "Model/Texture.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TexturedPolygonSorter.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        BrushFigure::BrushFigure(TextureRendererManager& textureRendererManager) :
        m_textureRendererManager(textureRendererManager),
        m_faceColor(Color(0.5f, 0.5f, 0.5f, 1.0f)),
        m_applyTinting(false),
        m_faceTintColor(Color(1.0f, 0.0, 0.0f, 1.0f)),
        m_edgeColor(Color(1.0f, 1.0f, 1.0f, 1.0f)),
        m_edgeMode(EMDefault),
        m_grayScale(false),
        m_faceRendererValid(false),
        m_edgeRendererValid(false) {}

        
        void BrushFigure::renderFaces(Vbo& vbo, RenderContext& context) {
            if (!m_faceRendererValid) {
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                if (!m_brushes.empty()) {
                    typedef TexturedPolygonSorter<Model::Texture, Model::Face*> FaceSorter;
                    FaceSorter faceSorter;
                    
                    Model::BrushList::const_iterator brushIt, brushEnd;
                    Model::FaceList::const_iterator faceIt, faceEnd;
                    for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                        const Model::Brush& brush = **brushIt;
                        const Model::FaceList& faces = brush.faces();
                        for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                            Model::Face* face = *faceIt;
                            faceSorter.addPolygon(face->texture(), face, face->vertices().size());
                        }
                    }
                    
                    m_faceRenderer = FaceRendererPtr(new FaceRenderer(vbo, m_textureRendererManager, faceSorter, m_faceColor));
                } else {
                    m_faceRenderer = FaceRendererPtr();
                }
                m_faceRendererValid = true;
            }
            
            if (m_faceRenderer.get() != NULL) {
                SetVboState activateVbo(vbo, Vbo::VboActive);
                if (m_applyTinting)
                    m_faceRenderer->render(context, m_grayScale, m_faceTintColor);
                else
                    m_faceRenderer->render(context, m_grayScale);
            }
        }
        
        void BrushFigure::renderEdges(Vbo& vbo, RenderContext& context) {
            if (!m_edgeRendererValid) {
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                if (!m_brushes.empty()) {
                    if (m_edgeMode == EMDefault)
                        m_edgeRenderer = EdgeRendererPtr(new EdgeRenderer(vbo, m_brushes, Model::EmptyFaceList, m_edgeColor));
                    else
                        m_edgeRenderer = EdgeRendererPtr(new EdgeRenderer(vbo, m_brushes, Model::EmptyFaceList));
                } else {
                    m_edgeRenderer = EdgeRendererPtr();
                }
                m_edgeRendererValid = true;
            }

            if (m_edgeRenderer.get() != NULL) {
                SetVboState activateVbo(vbo, Vbo::VboActive);
                glSetEdgeOffset(0.02f);
                if (m_edgeMode == EMDefault) {
                    m_edgeRenderer->render(context);
                } else {
                    if (m_edgeMode == EMRenderOccluded) {
                        glDisable(GL_DEPTH_TEST);
                        m_edgeRenderer->render(context, m_occludedEdgeColor);
                        glEnable(GL_DEPTH_TEST);
                        glSetEdgeOffset(0.025f);
                    }
                    m_edgeRenderer->render(context, m_edgeColor);
                }
                glResetEdgeOffset();
            }
        }

        void BrushFigure::render(Vbo& vbo, RenderContext& context) {
            renderFaces(vbo, context);
            renderEdges(vbo, context);
        }
    }
}