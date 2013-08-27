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
#include "Controller/Command.h"
#include "Controller/RemoveObjectsCommand.h"
#include "Controller/ReparentBrushesCommand.h"
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
        Vec3f ClipTool::selectNormal(const Vec3f::List& normals1, const Vec3f::List& normals2) const {
            assert(!normals1.empty());
            
            Vec3f sum;
            // first, try to find two normals with the same first axis
            for (size_t i = 0; i < normals1.size(); i++) {
                const Vec3f& normal1 = normals1[i];
                for (size_t j =  0; j < normals2.size(); j++) {
                    const Vec3f& normal2 = normals2[j];
                    if (normal1.firstAxis() == normal2.firstAxis())
                        return normal1;
                }
                sum += normal1;
            }
            
            for (size_t i = 0; i < normals2.size(); i++)
                sum += normals2[i];
            
            return sum / static_cast<float>((normals1.size() + normals2.size()));
        }

        void ClipTool::updateBrushes() {
            m_frontBrushes.clear();
            m_backBrushes.clear();
            
            Model::BrushList allFrontBrushes, allBackBrushes;
            
            Renderer::Camera& camera = view().camera();
            Vec3f planePoints[3];
            bool validPlane = false;
            if (m_numPoints > 0) {
                if (m_numPoints == 1) {
                    assert(!m_normals[0].empty());

                    if (m_normals[0].size() <= 2) { // the point is not on a vertex
                        validPlane = true;
                        planePoints[0] = m_points[0].rounded();
                        
                        const Vec3f normal = m_normals[0].size() == 1 ? m_normals[0][0] : (m_normals[0][0] + m_normals[0][1]) / 2.0f;
                        planePoints[1] = planePoints[0] + 128.0f * Vec3f::PosZ;
                        if (normal.firstComponent() == Axis::AZ) {
                            const Vec3f dir = camera.direction().firstComponent() != Axis::AZ ? camera.direction().firstAxis() : camera.direction().secondAxis();
                            planePoints[2] = planePoints[0] + 128.0f * dir;
                        } else {
                            planePoints[2] = planePoints[0] + 128.0f * normal.firstAxis();
                        }
                    }
                } else if (m_numPoints == 2) {
                    assert(!m_normals[0].empty());
                    assert(!m_normals[1].empty());
                    
                    validPlane = true;
                    planePoints[0] = m_points[0].rounded();
                    planePoints[2] = m_points[1].rounded();
                    
                    const Vec3f normal = selectNormal(m_normals[0], m_normals[1]);
                    planePoints[1] = planePoints[0] + 128.0f * normal.firstAxis();
                } else {
                    validPlane = true;
                    planePoints[0] = m_points[0].rounded();
                    planePoints[1] = m_points[1].rounded();
                    planePoints[2] = m_points[2].rounded();
                }
                
                // make sure the plane's normal points towards the camera or to its left if the camera position is on the plane
                if (validPlane) {
                    Planef plane;
                    plane.setPoints(planePoints[0], planePoints[1], planePoints[2]);
                    if (plane.pointStatus(camera.position()) == PointStatus::PSInside) {
                        if (plane.normal.dot(camera.right()) < 0.0f)
                            std::swap(planePoints[1], planePoints[2]);
                    } else {
                        if (plane.normal.dot(camera.direction()) > 0.0f)
                            std::swap(planePoints[1], planePoints[2]);
                    }
                }
            }
            
            const Model::BrushList& brushes = document().editStateManager().selectedBrushes();
            if (validPlane) {
                const BBoxf& worldBounds = document().map().worldBounds();
                const bool forceIntegerFacePoints = document().map().forceIntegerFacePoints();
                const String textureName = document().mruTexture() != NULL ? document().mruTexture()->name() : Model::Texture::Empty;
                
                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    Model::Entity* entity = brush.entity();
                    Model::Face* frontFace = new Model::Face(worldBounds, forceIntegerFacePoints, planePoints[0], planePoints[1], planePoints[2], textureName);
                    Model::Face* backFace = new Model::Face(worldBounds, forceIntegerFacePoints, planePoints[0], planePoints[2], planePoints[1], textureName);
                    
                    // determine the texture for the new faces
                    // we will use the texture of the face whose normal is closest to the newly inserted face
                    const Model::FaceList& faces = brush.faces();
                    Model::FaceList::const_iterator faceIt = faces.begin();
                    Model::FaceList::const_iterator faceEnd = faces.end();
                    const Model::Face* bestFrontFace = *faceIt++;
                    const Model::Face* bestBackFace = bestFrontFace;
                    
                    while (faceIt != faceEnd) {
                        const Model::Face* face = *faceIt++;
                        
                        const Vec3f bestFrontDiff = bestFrontFace->boundary().normal - frontFace->boundary().normal;
                        const Vec3f frontDiff = face->boundary().normal - frontFace->boundary().normal;
                        if (frontDiff.lengthSquared() < bestFrontDiff.lengthSquared())
                            bestFrontFace = face;
                        
                        const Vec3f bestBackDiff = bestBackFace->boundary().normal - backFace->boundary().normal;
                        const Vec3f backDiff = face->boundary().normal - backFace->boundary().normal;
                        if (backDiff.lengthSquared() < bestBackDiff.lengthSquared())
                            bestBackFace = face;
                    }
                    
                    frontFace->setAttributes(*bestFrontFace);
                    backFace->setAttributes(*bestBackFace);
                    
                    Model::Brush* frontBrush = new Model::Brush(worldBounds, forceIntegerFacePoints, brush);
                    if (frontBrush->clip(*frontFace)) {
                        m_frontBrushes[entity].push_back(frontBrush);
                        allFrontBrushes.push_back(frontBrush);
                    } else {
                        delete frontBrush;
                    }
                    
                    Model::Brush* backBrush = new Model::Brush(worldBounds, forceIntegerFacePoints, brush);
                    if (backBrush->clip(*backFace)) {
                        m_backBrushes[entity].push_back(backBrush);
                        allBackBrushes.push_back(backBrush);
                    } else {
                        delete backBrush;
                    }
                }
            } else {
                allFrontBrushes = brushes;
                m_frontBrushes = entityBrushes(allFrontBrushes);
            }

            m_frontBrushFigure->setBrushes(allFrontBrushes);
            m_backBrushFigure->setBrushes(allBackBrushes);
        }
        
        Vec3f::List ClipTool::getNormals(const Vec3f& hitPoint, const Model::Face& hitFace) const {
            bool found = false;
            Vec3f::List normals;
            
            const Model::Brush& brush = *hitFace.brush();
            const Model::VertexList& vertices = hitFace.vertices();
            Model::VertexList::const_iterator vIt, vEnd;
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd && !found; ++vIt) {
                const Model::Vertex& vertex = **vIt;
                if (hitPoint.equals(vertex.position)) {
                    found = true;
                    const Model::FaceList incidentFaces = brush.incidentFaces(vertex);
                    Model::FaceList::const_iterator fIt, fEnd;
                    for (fIt = incidentFaces.begin(), fEnd = incidentFaces.end(); fIt != fEnd; ++fIt) {
                        const Model::Face& incidentFace = **fIt;
                        normals.push_back(incidentFace.boundary().normal);
                    }
                }
            }
            
            if (!found) {
                const Model::EdgeList& edges = hitFace.edges();
                Model::EdgeList::const_iterator eIt, eEnd;
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd && !found; ++eIt) {
                    const Model::Edge& edge = **eIt;
                    if (edge.contains(hitPoint)) {
                        normals.push_back(edge.left->face->boundary().normal);
                        normals.push_back(edge.right->face->boundary().normal);
                        found = true;
                    }
                }
            }
            
            if (!found)
                normals.push_back(hitFace.boundary().normal);
            
            return normals;
        }

        bool ClipTool::isPointIdenticalWithExistingPoint(const Vec3f& point) const {
            for (size_t i = 0; i < m_numPoints; i++)
                if (m_points[i] == point)
                    return true;
            return false;
        }

        bool ClipTool::isPointLinearlyDependent(const Vec3f& point) const {
            if (m_numPoints < 2)
                return false;
            const Vec3f v1 = (point - m_points[0]).normalized();
            const Vec3f v2 = (point - m_points[1]).normalized();
            const float dot = v1.dot(v2);
            return Math<float>::eq(std::abs(dot), 1.0f);
        }

        bool ClipTool::handleActivate(InputState& inputState) {
            m_numPoints = 0;
            m_hitIndex = -1;
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
                if (!Math<float>::isnan(distance)) {
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
                    const Vec3f point = grid.snap(faceHit->hitPoint(), face.boundary());
                    
                    for (unsigned int i = 0; i < m_numPoints && m_hitIndex == -1; i++) {
                        if (point.equals(m_points[i])) {
                            m_hitIndex = static_cast<int>(i);
                            return;
                        }
                    }
                    
                    if (m_hitIndex == -1 && m_numPoints < 3 &&
                        !isPointLinearlyDependent(point)) {
                        m_hitIndex = static_cast<int>(m_numPoints);
                        m_points[m_numPoints] = point;
                        m_normals[m_numPoints] = getNormals(point, face);
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
                pointHandleShader.setUniformVariable("CameraPosition", renderContext.camera().position());
                pointHandleShader.setUniformVariable("ScalingFactor", prefs.getFloat(Preferences::HandleScalingFactor));
                pointHandleShader.setUniformVariable("MaximumDistance", prefs.getFloat(Preferences::MaximumHandleDistance));

                Renderer::SphereFigure sphereFigure(prefs.getFloat(Preferences::HandleRadius), 1);
                for (unsigned int i = 0; i < m_numPoints; i++) {
                    pointHandleShader.setUniformVariable("Position", Vec4f(m_points[i], 1.0f));
                    glDisable(GL_DEPTH_TEST);
                    pointHandleShader.setUniformVariable("Color", prefs.getColor(Preferences::OccludedClipHandleColor));
                    sphereFigure.render(vbo, renderContext);
                    glEnable(GL_DEPTH_TEST);
                    pointHandleShader.setUniformVariable("Color", prefs.getColor(Preferences::ClipHandleColor));
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
                        pointHandleShader.setUniformVariable("Position", Vec4f(m_points[m_hitIndex], 1.0f));
                        glDisable(GL_DEPTH_TEST);
                        pointHandleShader.setUniformVariable("Color", prefs.getColor(Preferences::OccludedClipHandleColor));
                        sphereFigure.render(vbo, renderContext);
                        glEnable(GL_DEPTH_TEST);
                        pointHandleShader.setUniformVariable("Color", prefs.getColor(Preferences::ClipHandleColor));
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
                    planeShader.setUniformVariable("Color", prefs.getColor(Preferences::OccludedClipHandleColor));
                    linesArray->render();
                    glEnable(GL_DEPTH_TEST);
                    planeShader.setUniformVariable("Color", prefs.getColor(Preferences::ClipHandleColor));
                    linesArray->render();
                    
                    if (m_numPoints == 3) {
                        glDisable(GL_DEPTH_TEST);
                        glDisable(GL_CULL_FACE);
                        planeShader.setUniformVariable("Color", prefs.getColor(Preferences::ClipPlaneColor));
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
            
            Controller::Command* command = new Controller::DocumentCommand(Controller::Command::ClipToolChange, document());
            submitCommand(command, false);

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
                const Planef plane(m_normals[m_hitIndex].front(), m_points[m_hitIndex]);
                const float distance = plane.intersectWithRay(inputState.pickRay());
                if (Math<float>::isnan(distance))
                    return true;
                
                const Vec3f hitPoint = inputState.pickRay().pointAtDistance(distance);
                
                Utility::Grid& grid = document().grid();
                const Vec3f point = grid.snap(hitPoint, plane);
                if (!isPointIdenticalWithExistingPoint(point) &&
                    !isPointLinearlyDependent(point))
                    m_points[m_hitIndex] = point;
            } else {
                const Planef& plane = faceHit->face().boundary();
                const Vec3f& hitPoint = faceHit->hitPoint();
                
                Utility::Grid& grid = document().grid();
                const Vec3f point = grid.snap(hitPoint, plane);

                if (!isPointIdenticalWithExistingPoint(point) &&
                    !isPointLinearlyDependent(point)) {
                    m_points[m_hitIndex] = point;
                    m_normals[m_hitIndex] = getNormals(point, faceHit->face());
                }
            }
            
            updateBrushes();
            
            Controller::Command* command = new Controller::DocumentCommand(Controller::Command::ClipToolChange, document());
            submitCommand(command, false);

            return true;
        }
        
        void ClipTool::handleEndDrag(InputState& inputState) {
        }
        
        bool ClipTool::handleNavigateUp(InputState& inputState) {
            assert(active());
            if (m_numPoints > 0) {
                m_numPoints = 0;
                updateBrushes();
                
                Controller::Command* command = new Controller::DocumentCommand(Controller::Command::ClipToolChange, document());
                submitCommand(command, false);
                
                return true;
            }
            
            return false;
        }

        void ClipTool::handleUpdate(const Command& command, InputState& inputState) {
            if (active()) {
                switch (command.type()) {
                    case Controller::Command::LoadMap:
                    case Controller::Command::ClearMap:
                    case Controller::Command::TransformObjects:
                    case Controller::Command::ResizeBrushes:
                        updateBrushes();
                        break;
                    default:
                        break;
                }
            }
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

            Controller::Command* command = new Controller::DocumentCommand(Controller::Command::ClipToolChange, document());
            submitCommand(command, false);
        }
        
        void ClipTool::performClip() {
            assert(active());
            assert(m_numPoints > 0);

            Model::EntityBrushesMap addBrushes;
            switch (m_clipSide) {
                case CMFront:
                    addBrushes = m_frontBrushes;
                    break;
                case CMBack:
                    addBrushes = m_backBrushes;
                    break;
                default:
                    addBrushes = mergeEntityBrushes(m_frontBrushes, m_backBrushes);
                    break;
            }
            
            const Model::BrushList removeBrushes = document().editStateManager().selectedBrushes();

            beginCommandGroup(wxT("Clip"));
            submitCommand(ChangeEditStateCommand::deselectAll(document()));

            if (!addBrushes.empty()) {
                Model::BrushList allBrushes;
                Model::EntityBrushesMap::const_iterator it, end;
                for (it = addBrushes.begin(), end = addBrushes.end(); it != end; ++it) {
                    Model::Entity* entity = it->first;
                    
                    const Model::BrushList& entityBrushes = it->second;
                    allBrushes.insert(allBrushes.end(), entityBrushes.begin(), entityBrushes.end());
                    
                    submitCommand(AddObjectsCommand::addBrushes(document(), entityBrushes));
                    if (!entity->worldspawn())
                        submitCommand(ReparentBrushesCommand::reparent(document(), entityBrushes, *entity));
                }
                submitCommand(ChangeEditStateCommand::select(document(), allBrushes));
            }

            submitCommand(RemoveObjectsCommand::removeBrushes(document(), removeBrushes));
            endCommandGroup();
            
            m_numPoints = 0;
            m_hitIndex = -1;
            
            // only update if there are still brushes left because otherwise we have been deactivated
            if (active())
                updateBrushes();
            
            Controller::Command* command = new Controller::DocumentCommand(Controller::Command::ClipToolChange, document());
            submitCommand(command, false);
       }
    }
}
