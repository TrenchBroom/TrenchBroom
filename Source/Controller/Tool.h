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

#ifndef TrenchBroom_Tool_h
#define TrenchBroom_Tool_h

#include <cassert>

#include "Controller/Input.h"
#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Model/MapDocument.h"
#include "Renderer/Figure.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "Utility/CommandProcessor.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <wx/cmdproc.h>
#include <wx/event.h>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
        class MapDocument;
    }
    
    namespace Renderer {
        class Camera;
        class RenderContext;
        class Vbo;
    }
    
    namespace View {
        class EditorView;
    }
    
    namespace Controller {
        class Tool {
        protected:
            typedef enum {
                DTNone,
                DTDrag,
                DTDragTarget
            } DragType;
        private:
            View::DocumentViewHolder& m_documentViewHolder;
            
            bool m_activatable;
            bool m_active;
            bool m_suppressed;
            DragType m_dragType;
            String m_dragPayload;
            Tool* m_nextTool;
            
            Renderer::Figure::List m_deleteFigures;
            
            void deleteFigures() {
                while (!m_deleteFigures.empty()) delete m_deleteFigures.back(), m_deleteFigures.pop_back();
            }
        protected:
            Tool(View::DocumentViewHolder& documentViewHolder, bool activatable) :
            m_documentViewHolder(documentViewHolder),
            m_activatable(activatable),
            m_active(!m_activatable),
            m_suppressed(false),
            m_dragType(DTNone),
            m_nextTool(NULL) {}
            
            inline Model::MapDocument& document() const {
                return m_documentViewHolder.document();
            }
            
            inline View::EditorView& view() const {
                return m_documentViewHolder.view();
            }
            
            inline void postEvent(wxEvent& event) {
                if (!m_documentViewHolder.valid())
                    return;
                
                View::EditorView& view = m_documentViewHolder.view();
                event.SetEventObject(&view);
                view.ProcessEvent(event);
            }
            
            inline void beginCommandGroup(const wxString& name) {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::BeginGroup(document.GetCommandProcessor(), name);
            }
            
            inline void endCommandGroup() {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::EndGroup(document.GetCommandProcessor());
            }
            
            inline void rollbackCommandGroup() {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::RollbackGroup(document.GetCommandProcessor());
            }
            
            inline void discardCommandGroup() {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::DiscardGroup(document.GetCommandProcessor());
            }
            
            inline bool submitCommand(wxCommand* command, bool store = true) {
                if (!m_documentViewHolder.valid())
                    return false;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                return document.GetCommandProcessor()->Submit(command, store);
            }
            
            inline void blockUndo() {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::Block(document.GetCommandProcessor());
            }
            
            inline void unblockUndo() {
                if (!m_documentViewHolder.valid())
                    return;
                
                Model::MapDocument& document = m_documentViewHolder.document();
                CommandProcessor::Unblock(document.GetCommandProcessor());
            }

            inline Tool* nextTool() const { return m_nextTool; }
            
            /* Activation Protocol */
            virtual bool handleActivate(InputState& inputState) { return true; }
            virtual bool handleDeactivate(InputState& inputState) { return true; }
            virtual bool handleIsModal(InputState& inputState) { return false; }
            
            /* Feedback Protocol */
            virtual void handlePick(InputState& inputState) {}
            virtual void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {}
            virtual void handleFreeRenderResources() {}
            
            inline void deleteFigure(Renderer::Figure* figure) {
                m_deleteFigures.push_back(figure);
            }

            /* Input Protocol */
            virtual void handleModifierKeyChange(InputState& inputState) {}
            virtual bool handleMouseDown(InputState& inputState) { return false; }
            virtual bool handleMouseUp(InputState& inputState) { return false; }
            virtual bool handleMouseDClick(InputState& inputState) { return false; }
            virtual void handleMouseMove(InputState& inputState) {}
            virtual void handleScroll(InputState& inputState) {}
            
            /* Drag Protocol */
            inline DragType dragType() const {
                return m_dragType;
            }
            
            virtual bool handleStartDrag(InputState& inputState) { return false; }
            virtual bool handleDrag(InputState& inputState) { return true; }
            virtual void handleEndDrag(InputState& inputState) {}
            virtual void handleCancelDrag(InputState& inputState) {}

            /* Drag Target Protocol */
            virtual bool handleDragEnter(InputState& inputState, const String& payload) { return false; }
            virtual void handleDragMove(InputState& inputState, const String& payload) {}
            virtual void handleDragLeave(InputState& inputState, const String& payload) {}
            virtual bool handleDragDrop(InputState& inputState, const String& payload) { return false; }
            
            virtual void handleObjectsChange(InputState& inputState) {}
            virtual void handleEditStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {}
            virtual void handleCameraChange(InputState& inputState) {}
        public:
            virtual ~Tool() {
                deleteFigures();
            }
            
            inline void setNextTool(Tool* tool) {
                m_nextTool = tool;
            }

            /* Activation Protocol */
            
            inline bool active() const {
                if (!m_activatable)
                    return true;
                return m_active;
            }
            
            inline void activate(InputState& inputState) {
                if (m_activatable) {
                    assert(!active());
                    if (m_suppressed || handleActivate(inputState))
                        m_active = !m_active;
                }
            }
            
            inline void deactivate(InputState& inputState) {
                if (m_activatable) {
                    assert(active());
                    if (handleDeactivate(inputState))
                        m_active = !m_active;
                }
            }
            
            inline void setSuppressed(InputState& inputState, bool suppressed, Tool* except = NULL) {
                if (m_activatable && this != except) {
                    bool wasActive = (active() && !m_suppressed);
                    m_suppressed = suppressed;
                    if ((active() && !m_suppressed) && !wasActive)
                        handleActivate(inputState);
                    else if (!(active() && !m_suppressed) && wasActive)
                        handleDeactivate(inputState);
                }
                if (nextTool() != NULL)
                    nextTool()->setSuppressed(inputState, suppressed, except);
            }
            
            inline bool isModal(InputState& inputState) {
                return (active() /*&& !m_suppressed*/) && handleIsModal(inputState);
            }
            
            inline Tool* modalTool(InputState& inputState) {
                if (isModal(inputState))
                    return this;
                if (nextTool() != NULL)
                    return nextTool()->modalTool(inputState);
                return NULL;
            }

            /* Feedback Protocol */

            inline void render(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
                deleteFigures();
                if ((active() && !m_suppressed))
                    handleRender(inputState, vbo, renderContext);
                if (nextTool() != NULL)
                    nextTool()->render(inputState, vbo, renderContext);
            }
            
            inline void freeRenderResources() {
                handleFreeRenderResources();
                if (nextTool() != NULL)
                    nextTool()->freeRenderResources();
            }
            
            inline void updateHits(InputState& inputState) {
                if ((active() && !m_suppressed))
                    handlePick(inputState);
                if (nextTool() != NULL)
                    nextTool()->updateHits(inputState);
            }
            
            /* Input Protocol */
            
            void modifierKeyChange(InputState& inputState) {
                if ((active() && !m_suppressed))
                    handleModifierKeyChange(inputState);
                if (nextTool() != NULL)
                    nextTool()->modifierKeyChange(inputState);
            }
            
            Tool* mouseDown(InputState& inputState) {
                if ((active() && !m_suppressed) && handleMouseDown(inputState))
                    return this;
                if (nextTool() != NULL)
                    return nextTool()->mouseDown(inputState);
                return NULL;
            }
            
            Tool* mouseUp(InputState& inputState) {
                if ((active() && !m_suppressed) && handleMouseUp(inputState))
                    return this;
                if (nextTool() != NULL)
                    return nextTool()->mouseUp(inputState);
                return NULL;
            }
            
            Tool* mouseDClick(InputState& inputState) {
                if ((active() & !m_suppressed) && handleMouseDClick(inputState))
                    return this;
                if (nextTool() != NULL)
                    return nextTool()->mouseDClick(inputState);
                return NULL;
            }
            
            void mouseMove(InputState& inputState) {
                if ((active() && !m_suppressed))
                    handleMouseMove(inputState);
                if (nextTool() != NULL)
                    nextTool()->mouseMove(inputState);
            }
            
            void scroll(InputState& inputState) {
                if ((active() && !m_suppressed))
                    handleScroll(inputState);
                if (nextTool() != NULL)
                    nextTool()->scroll(inputState);
            }
            
            /* Drag Protocol */
            
            Tool* startDrag(InputState& inputState) {
                assert(dragType() == DTNone);
                if ((active() && !m_suppressed) && handleStartDrag(inputState)) {
                    m_dragType = DTDrag;
                    return this;
                }
                if (nextTool() != NULL)
                    return nextTool()->startDrag(inputState);
                return NULL;
            }
            
            bool drag(InputState& inputState) {
                assert((active() && !m_suppressed));
                assert(dragType() == DTDrag);
                return handleDrag(inputState);
            }
            
            void endDrag(InputState& inputState) {
                assert((active() && !m_suppressed));
                assert(dragType() == DTDrag);
                handleEndDrag(inputState);
                m_dragType = DTNone;
            }
            
            void cancelDrag(InputState& inputState) {
                assert((active() && !m_suppressed));
                assert(dragType() == DTDrag);
                handleCancelDrag(inputState);
                m_dragType = DTNone;
            }
            
            /* DragTarget Protocol */
            
            Tool* dragEnter(InputState& inputState, const String& payload) {
                assert(dragType() == DTNone);
                if ((active() && !m_suppressed) && handleDragEnter(inputState, payload)) {
                    m_dragType = DTDragTarget;
                    m_dragPayload = payload;
                    return this;
                }
                if (nextTool() != NULL)
                    return nextTool()->dragEnter(inputState, payload);
                return NULL;
            }
            
            void dragMove(InputState& inputState) {
                assert((active() && !m_suppressed));
                assert(dragType() == DTDragTarget);
                handleDragMove(inputState, m_dragPayload);
            }
            
            void dragLeave(InputState& inputState) {
                assert((active() && !m_suppressed));
                assert(dragType() == DTDragTarget);
                handleDragLeave(inputState, m_dragPayload);
                m_dragPayload = "";
                m_dragType = DTNone;
            }
            
            bool dragDrop(InputState& inputState) {
                assert((active() && !m_suppressed));
                assert(dragType() == DTDragTarget);
                bool success = handleDragDrop(inputState, m_dragPayload);
                m_dragPayload = "";
                m_dragType = DTNone;
                return success;
            }
            
            void objectsChange(InputState& inputState) {
                handleObjectsChange(inputState);
                if (nextTool() != NULL)
                    nextTool()->objectsChange(inputState);
            }
            
            void editStateChange(InputState& inputState, const Model::EditStateChangeSet& changeSet) {
                handleEditStateChange(inputState, changeSet);
                if (nextTool() != NULL)
                    nextTool()->editStateChange(inputState, changeSet);
            }
            
            void cameraChange(InputState& inputState) {
                handleCameraChange(inputState);
                if (nextTool() != NULL)
                    nextTool()->cameraChange(inputState);
            }
        };
        
        class PlaneDragTool : public Tool {
        private:
            Plane m_plane;
            Vec3f m_lastPoint;
            Vec3f m_refPoint;
        protected:
            inline const Plane& dragPlane() const {
                assert(dragType() == DTDrag);
                return m_plane;
            }
            
            virtual bool handleStartPlaneDrag(InputState& inputState, Plane& plane, Vec3f& initialPoint) = 0;
            virtual bool handlePlaneDrag(InputState& inputState, const Vec3f& lastPoint, const Vec3f& curPoint, Vec3f& refPoint) = 0;
            virtual void handleEndPlaneDrag(InputState& inputState) = 0;
            
            bool handleStartDrag(InputState& inputState) {
                if (handleStartPlaneDrag(inputState, m_plane, m_lastPoint)) {
                    m_refPoint = m_lastPoint;
                    return true;
                }
                return false;
            }
            
            bool handleDrag(InputState& inputState) {
                float distance = m_plane.intersectWithRay(inputState.pickRay());
                if (Math::isnan(distance))
                    return true;
                
                Vec3f curPoint = inputState.pickRay().pointAtDistance(distance);
                if (curPoint.equals(m_lastPoint))
                    return true;
                
                bool result = handlePlaneDrag(inputState, m_lastPoint, curPoint, m_refPoint);
                m_lastPoint = curPoint;
                return result;
            }
            
            void handleEndDrag(InputState& inputState) {
                handleEndPlaneDrag(inputState);
            }
        public:
            PlaneDragTool(View::DocumentViewHolder& documentViewHolder, bool activatable) :
            Tool(documentViewHolder, activatable) {}
            
            virtual ~PlaneDragTool() {}
        };
    }
}

#endif
