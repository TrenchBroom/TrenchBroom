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

#include "ClipTool.h"

#include "Controller/AddObjectsCommand.h"
#include "Controller/ChangeEditStateCommand.h"
#include "Controller/RemoveObjectsCommand.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Model/Picker.h"
#include "Model/Texture.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/BrushFigure.h"
#include "Renderer/PointHandleHighlightFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SharedResources.h"
#include "Renderer/SphereFigure.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "View/EditorView.h"
#include "Utility/Grid.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Model {
        ClipHandleHit::ClipHandleHit(const Vec3f& hitPoint, float distance, unsigned int index) :
        Hit(HitType::ClipHandleHit, hitPoint, distance),
        m_index(index) {
            assert(m_index < 3);
        }
        
        bool ClipHandleHit::pickable(Filter& filter) const {
            return true;
        }
    }
    
    namespace Controller {
        void ClipTool::updateBrushes() {
            m_frontBrushes.clear();
            m_backBrushes.clear();
            
            const Model::BrushList& brushes = document().editStateManager().selectedBrushes();
            if (m_numPoints == 0) {
                m_frontBrushes = brushes;
            } else {
                Vec3f planePoints[3];
                if (m_numPoints == 1) {
                    planePoints[0] = m_points[0].snapped();
                    if (m_normals[0].firstComponent() == Axis::AZ) {
                        planePoints[1] = planePoints[0] + 128.0f * Vec3f::PosY;
                        planePoints[2] = planePoints[0] + 128.0f * Vec3f::PosX;
                    } else {
                        planePoints[1] = planePoints[0] + 128.0f * Vec3f::PosZ;
                        planePoints[2] = planePoints[0] + 128.0f * m_normals[0].firstAxis();
                    }
                } else if (m_numPoints == 2) {
                    planePoints[0] = m_points[0].snapped();
                    planePoints[2] = m_points[1].snapped();
                    if (m_normals[0].firstComponent() != m_normals[1].firstComponent()) {
                        planePoints[1] = planePoints[0] + 128.0f * m_normals[0].firstAxis();
                    } else {
                        Vec3f temp = planePoints[1] - planePoints[0];
                        temp.normalize();
                        if (eq(std::abs(temp.dot(m_normals[0].firstAxis())), 1.0f)) {
                            if (m_normals[0].firstComponent() == Axis::AZ)
                                planePoints[1] = planePoints[0] + 128.0f * view().camera().direction().firstAxis();
                            else
                                planePoints[1] = planePoints[0] + 128.0f * Vec3f::PosZ;
                        } else {
                            planePoints[1] = planePoints[0] + 128.0f * m_normals[0].firstAxis();
                        }
                    }
                } else {
                    planePoints[0] = m_points[0].snapped();
                    planePoints[1] = m_points[1].snapped();
                    planePoints[2] = m_points[2].snapped();
                }
                
                const BBox& worldBounds = document().map().worldBounds();
                String textureName = document().mruTexture() != NULL ? document().mruTexture()->name() : Model::Texture::Empty;
                
                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    
                    Model::Brush* frontBrush = new Model::Brush(worldBounds, brush);
                    Model::Face* frontFace = new Model::Face(worldBounds, planePoints[0], planePoints[1], planePoints[2], textureName);
                    if (frontBrush->addFace(frontFace))
                        m_frontBrushes.push_back(frontBrush);
                    else
                        delete frontBrush;
                    
                    Model::Brush* backBrush = new Model::Brush(worldBounds, brush);
                    Model::Face* backFace = new Model::Face(worldBounds, planePoints[0], planePoints[2], planePoints[1], textureName);
                    if (backBrush->addFace(backFace))
                        m_backBrushes.push_back(backBrush);
                    else
                        delete backBrush;
                }
            }
            
            m_frontBrushFigure->setBrushes(m_frontBrushes);
            m_backBrushFigure->setBrushes(m_backBrushes);
        }
        
        bool ClipTool::handleActivate(InputState& inputState) {
            m_numPoints = 0;
            m_hitIndex = -1;
            m_clipSide = CMFront;
            view().viewOptions().setRenderSelection(false);
            
            assert(m_frontBrushFigure == NULL);
            assert(m_backBrushFigure == NULL);
            
            Renderer::TextureRendererManager& textureRendererManager = document().sharedResources().textureRendererManager();
            m_frontBrushFigure = new Renderer::BrushFigure(textureRendererManager);
            m_backBrushFigure = new Renderer::BrushFigure(textureRendererManager);
            
            updateBrushes();
            
            return true;
        }
        
        bool ClipTool::handleDeactivate(InputState& inputState) {
            deleteFigure(m_frontBrushFigure);
            m_frontBrushFigure = NULL;
            deleteFigure(m_backBrushFigure);
            m_backBrushFigure = NULL;
            
            view().viewOptions().setRenderSelection(true);
            return true;
        }
        
        bool ClipTool::handleIsModal(InputState& inputState) {
            return true;
        }
        
        void ClipTool::handlePick(InputState& inputState) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            float handleRadius = prefs.getFloat(Preferences::HandleRadius);
            float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
            float maxDistance = prefs.getFloat(Preferences::MaximumHandleDistance);
            
            for (unsigned int i = 0; i < m_numPoints; i++) {
                float distance = inputState.pickRay().intersectWithSphere(m_points[i], handleRadius, scalingFactor, maxDistance);
                if (!Math::isnan(distance)) {
                    Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                    inputState.pickResult().add(new Model::ClipHandleHit(hitPoint, distance, i));
                }
            }
            
            if (dragType() != DTNone)
                return;
            
            Model::ClipHandleHit* handleHit = static_cast<Model::ClipHandleHit*>(inputState.pickResult().first(Model::HitType::ClipHandleHit, true, m_filter));
            if (handleHit != NULL) {
                m_hitIndex = static_cast<int>(handleHit->index());
                m_directHit = true;
            } else {
                m_hitIndex = -1;
                m_directHit = false;
                
                Model::FaceHit* faceHit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, m_filter));
                if (faceHit != NULL) {
                    Utility::Grid& grid = document().grid();
                    const Model::Face& face = faceHit->face();
                    Vec3f point = grid.snap(faceHit->hitPoint(), face.boundary());
                    Vec3f dir = point - inputState.pickRay().origin;
                    dir.normalize();
                    
                    for (unsigned int i = 0; i < m_numPoints && m_hitIndex == -1; i++) {
                        if (point.equals(m_points[i])) {
                            m_hitIndex = static_cast<int>(i);
                            return;
                        }
                    }
                    
                    if (m_hitIndex == -1 && m_numPoints < 3) {
                        m_hitIndex = static_cast<int>(m_numPoints);
                        m_points[m_numPoints] = point;
                        m_normals[m_numPoints] = face.boundary().normal;
                    }
                }
            }
        }
        
        void ClipTool::handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            if (m_numPoints == 0 || m_clipSide == CMFront || m_clipSide == CMBoth) {
                m_frontBrushFigure->setFaceColor(prefs.getColor(Preferences::FaceColor));
                m_frontBrushFigure->setApplyTinting(true);
                m_frontBrushFigure->setFaceTintColor(prefs.getColor(Preferences::ClippedFaceColor));
                m_frontBrushFigure->setEdgeColor(prefs.getColor(Preferences::ClippedEdgeColor));
                m_frontBrushFigure->setOccludedEdgeColor(prefs.getColor(Preferences::OccludedClippedEdgeColor));
                m_frontBrushFigure->setEdgeMode(Renderer::BrushFigure::EMRenderOccluded);
                m_frontBrushFigure->setGrayScale(false);
            } else {
                m_frontBrushFigure->setFaceColor(prefs.getColor(Preferences::FaceColor));
                m_frontBrushFigure->setApplyTinting(false);
                m_frontBrushFigure->setEdgeColor(prefs.getColor(Preferences::EdgeColor));
                m_frontBrushFigure->setEdgeMode(Renderer::BrushFigure::EMDefault);
                m_frontBrushFigure->setGrayScale(true);
            }
            if (m_clipSide == CMBack || m_clipSide == CMBoth) {
                m_backBrushFigure->setFaceColor(prefs.getColor(Preferences::FaceColor));
                m_backBrushFigure->setApplyTinting(true);
                m_backBrushFigure->setFaceTintColor(prefs.getColor(Preferences::ClippedFaceColor));
                m_backBrushFigure->setEdgeColor(prefs.getColor(Preferences::ClippedEdgeColor));
                m_backBrushFigure->setOccludedEdgeColor(prefs.getColor(Preferences::OccludedClippedEdgeColor));
                m_backBrushFigure->setEdgeMode(Renderer::BrushFigure::EMRenderOccluded);
                m_backBrushFigure->setGrayScale(false);
            } else {
                m_backBrushFigure->setFaceColor(prefs.getColor(Preferences::FaceColor));
                m_backBrushFigure->setApplyTinting(false);
                m_backBrushFigure->setEdgeColor(prefs.getColor(Preferences::EdgeColor));
                m_backBrushFigure->setEdgeMode(Renderer::BrushFigure::EMDefault);
                m_backBrushFigure->setGrayScale(true);
            }
            
            m_frontBrushFigure->renderFaces(vbo, renderContext);
            m_backBrushFigure->renderFaces(vbo, renderContext);
            m_frontBrushFigure->renderEdges(vbo, renderContext);
            m_backBrushFigure->renderEdges(vbo, renderContext);
            
            if (m_numPoints > 0 || m_hitIndex > -1) {
                
                Renderer::ActivateShader pointHandleShader(renderContext.shaderManager(), Renderer::Shaders::PointHandleShader);
                pointHandleShader.currentShader().setUniformVariable("CameraPosition", renderContext.camera().position());
                pointHandleShader.currentShader().setUniformVariable("ScalingFactor", prefs.getFloat(Preferences::HandleScalingFactor));
                pointHandleShader.currentShader().setUniformVariable("MaximumDistance", prefs.getFloat(Preferences::MaximumHandleDistance));

                Renderer::SphereFigure sphereFigure(prefs.getFloat(Preferences::HandleRadius), 1);
                for (unsigned int i = 0; i < m_numPoints; i++) {
                    pointHandleShader.currentShader().setUniformVariable("Position", Vec4f(m_points[i], 1.0f));
                    glDisable(GL_DEPTH_TEST);
                    pointHandleShader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::OccludedClipHandleColor));
                    sphereFigure.render(vbo, renderContext);
                    glEnable(GL_DEPTH_TEST);
                    pointHandleShader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::ClipHandleColor));
                    sphereFigure.render(vbo, renderContext);
                }
                
                if (m_hitIndex > -1) {
                    if (static_cast<unsigned int>(m_hitIndex) < m_numPoints) {
                        if (dragType() == DTDrag ||
                            inputState.pickResult().first(Model::HitType::ClipHandleHit, true, m_filter) != NULL) {
                            const Color& color = prefs.getColor(Preferences::VertexHandleColor);
                            const float radius = prefs.getFloat(Preferences::HandleRadius);
                            const float scalingFactor = prefs.getFloat(Preferences::HandleScalingFactor);
                            
                            glDisable(GL_DEPTH_TEST);
                            Renderer::PointHandleHighlightFigure highlightFigure(m_points[m_hitIndex], color, radius, scalingFactor);
                            highlightFigure.render(vbo, renderContext);
                            glEnable(GL_DEPTH_TEST);
                        }
                    } else {
                        pointHandleShader.currentShader().setUniformVariable("Position", Vec4f(m_points[m_hitIndex], 1.0f));
                        glDisable(GL_DEPTH_TEST);
                        pointHandleShader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::OccludedClipHandleColor));
                        sphereFigure.render(vbo, renderContext);
                        glEnable(GL_DEPTH_TEST);
                        pointHandleShader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::ClipHandleColor));
                        sphereFigure.render(vbo, renderContext);
                    }
                }

                if (m_numPoints > 1) {
                    Renderer::ActivateShader planeShader(renderContext.shaderManager(), Renderer::Shaders::HandleShader);
                    Renderer::VertexArray* linesArray = NULL;
                    Renderer::VertexArray* triangleArray = NULL;
                    
                    Renderer::SetVboState mapVbo(vbo, Renderer::Vbo::VboMapped);
                    
                    linesArray = new Renderer::VertexArray(vbo, GL_LINE_LOOP, m_numPoints,
                                                           Renderer::Attribute::position3f());
                    for (unsigned int i = 0; i < m_numPoints; i++)
                        linesArray->addAttribute(m_points[i]);
                    
                    if (m_numPoints == 3) {
                        triangleArray = new Renderer::VertexArray(vbo, GL_TRIANGLES, m_numPoints,
                                                                  Renderer::Attribute::position3f());
                        for (unsigned int i = 0; i < m_numPoints; i++)
                            triangleArray->addAttribute(m_points[i]);
                    }
                    
                    Renderer::SetVboState activateVbo(vbo, Renderer::Vbo::VboActive);
                    glDisable(GL_DEPTH_TEST);
                    planeShader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::OccludedClipHandleColor));
                    linesArray->render();
                    glEnable(GL_DEPTH_TEST);
                    planeShader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::ClipHandleColor));
                    linesArray->render();
                    
                    if (m_numPoints == 3) {
                        glDisable(GL_DEPTH_TEST);
                        glDisable(GL_CULL_FACE);
                        planeShader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::ClipPlaneColor));
                        triangleArray->render();
                        glEnable(GL_CULL_FACE);
                        glEnable(GL_DEPTH_TEST);
                    }
                    
                    delete linesArray;
                    delete triangleArray;
                }
            }
        }
        
        void ClipTool::handleFreeRenderResources() {
            delete m_frontBrushFigure;
            m_frontBrushFigure = NULL;
            delete m_backBrushFigure;
            m_backBrushFigure = NULL;
        }

        bool ClipTool::handleMouseUp(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            if (m_hitIndex < static_cast<int>(m_numPoints) || m_numPoints == 3)
                return false;
            
            m_numPoints++;
            m_hitIndex = -1;
            updateBrushes();
            
            return true;
        }
        
        bool ClipTool::handleStartDrag(InputState& inputState) {
            if (inputState.mouseButtons() != MouseButtons::MBLeft ||
                inputState.modifierKeys() != ModifierKeys::MKNone)
                return false;
            
            Model::ClipHandleHit* handleHit = static_cast<Model::ClipHandleHit*>(inputState.pickResult().first(Model::HitType::ClipHandleHit, true, m_filter));
            if (handleHit == NULL)
                return false;
            
            m_hitIndex = static_cast<int>(handleHit->index());
            return true;
        }
        
        bool ClipTool::handleDrag(InputState& inputState) {
            assert(m_hitIndex >= 0 && m_hitIndex < static_cast<int>(m_numPoints));
            
            Model::FaceHit* faceHit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, m_filter));
            if  (faceHit == NULL) {
                const Plane plane(m_normals[m_hitIndex], m_points[m_hitIndex]);
                const float distance = plane.intersectWithRay(inputState.pickRay());
                if (Math::isnan(distance))
                    return true;
                
                const Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                
                Utility::Grid& grid = document().grid();
                Vec3f point = grid.snap(hitPoint, plane);
                m_points[m_hitIndex] = point;
            } else {
                const Plane& plane = faceHit->face().boundary();
                const Vec3f& hitPoint = faceHit->hitPoint();
                
                Utility::Grid& grid = document().grid();
                Vec3f point = grid.snap(hitPoint, plane);

                m_points[m_hitIndex] = point;
                m_normals[m_hitIndex] = plane.normal;
            }
            
            updateBrushes();
            return true;
        }
        
        void ClipTool::handleEndDrag(InputState& inputState) {
        }
        
        void ClipTool::handleObjectsChange(InputState& inputState) {
            if (active())
                updateBrushes();
        }
        
        void ClipTool::handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
            if (active())
                updateBrushes();
        }

        ClipTool::ClipTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        Tool(documentViewHolder, inputController, true),
        m_filter(view().filter()),
        m_numPoints(0),
        m_hitIndex(-1),
        m_directHit(false),
        m_clipSide(CMFront),
        m_frontBrushFigure(NULL),
        m_backBrushFigure(NULL) {}
        
        void ClipTool::toggleClipSide() {
            assert(active());
            switch (m_clipSide) {
                case CMFront:
                    m_clipSide = CMBack;
                    break;
                case CMBack:
                    m_clipSide = CMBoth;
                    break;
                default:
                    m_clipSide = CMFront;
            }
        }

        void ClipTool::deleteLastPoint() {
            assert(active());
            assert(m_numPoints > 0);
            m_numPoints--;
            updateBrushes();
        }
        
        void ClipTool::performClip() {
            assert(active());
            assert(m_numPoints > 0);

            Model::BrushList addBrushes;
            switch (m_clipSide) {
                case CMFront:
                    addBrushes = m_frontBrushes;
                    break;
                case CMBack:
                    addBrushes = m_backBrushes;
                    break;
                default: {
                    addBrushes.insert(addBrushes.end(), m_frontBrushes.begin(), m_frontBrushes.end());
                    addBrushes.insert(addBrushes.end(), m_backBrushes.begin(), m_backBrushes.end());
                    break;
                }
            }
            
            AddObjectsCommand* addCommand = AddObjectsCommand::addBrushes(document(), addBrushes);
            ChangeEditStateCommand* changeEditStateCommand = ChangeEditStateCommand::replace(document(), addBrushes);
            
            const Model::BrushList& removeBrushes = document().editStateManager().selectedBrushes();
            RemoveObjectsCommand* removeCommand = RemoveObjectsCommand::removeObjects(document(), Model::EmptyEntityList, removeBrushes);

            beginCommandGroup(wxT("Clip"));
            submitCommand(addCommand);
            submitCommand(changeEditStateCommand);
            submitCommand(removeCommand);
            endCommandGroup();
            
            m_numPoints = 0;
            m_hitIndex = -1;
            updateBrushes();
       }
    }
}
