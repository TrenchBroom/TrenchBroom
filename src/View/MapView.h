/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MapView__
#define __TrenchBroom__MapView__


#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "Notifier.h"
#include "Assets/AssetTypes.h"
#include "Renderer/GL.h"
#include "Renderer/BoundsGuideRenderer.h"
#include "Renderer/Compass.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderResources.h"
#include "Renderer/Vbo.h"
#include "View/BaseMapView.h"
#include "View/ViewTypes.h"

#include <vector>
#include <wx/datetime.h>
#include <wx/longlong.h>

namespace TrenchBroom {
    class Logger;
    
    namespace Model {
        class BrushFace;
        class Object;
        class SelectionResult;
    }
    
    namespace Renderer {
        class RenderContext;
    }
    
    namespace View {
        class AnimationManager;
        class BaseTool;
        class CameraTool;
        class ClipTool;
        class CreateBrushTool;
        class CreateEntityTool;
        class MoveObjectsTool;
        class ResizeBrushesTool;
        class RotateObjectsTool;
        class SelectionTool;
        class TextureTool;
        class VertexTool;
        
        class MapView : public BaseMapView {
        private:
            Logger* m_logger;
            
            Renderer::Vbo m_auxVbo;
            Color m_focusColor;
            
            Renderer::RenderResources m_renderResources;
            Renderer::MapRenderer m_renderer;
            Renderer::Compass m_compass;
            Renderer::BoundsGuideRenderer m_selectionGuide;
            
            CameraTool* m_cameraTool;
            ClipTool* m_clipTool;
            CreateBrushTool* m_createBrushTool;
            CreateEntityTool* m_createEntityTool;
            MoveObjectsTool* m_moveObjectsTool;
            VertexTool* m_vertexTool;
            ResizeBrushesTool* m_resizeBrushesTool;
            RotateObjectsTool* m_rotateObjectsTool;
            SelectionTool* m_selectionTool;
            TextureTool* m_textureTool;
        public:
            MapView(wxWindow* parent, Logger* logger, View::MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera);
            ~MapView();

            Renderer::RenderResources& renderResources();
            
            void centerCameraOnSelection();

            void toggleClipTool();
            bool clipToolActive() const;
            bool canToggleClipSide() const;
            void toggleClipSide();
            bool canPerformClip() const;
            void performClip();
            bool canDeleteLastClipPoint() const;
            void deleteLastClipPoint();
            
            void toggleRotateObjectsTool();
            bool rotateObjectsToolActive() const;
            
            void toggleVertexTool();
            bool vertexToolActive() const;
            bool hasSelectedVertices() const;
            bool canSnapVertices() const;
            void snapVertices(size_t snapTo);
            
            void toggleTextureTool();
            bool textureToolActive() const;
            
            void moveObjects(Math::Direction direction);
            void rotateObjects(RotationAxis axis, bool clockwise);
            void flipObjects(Math::Direction direction);
            void moveTextures(Math::Direction direction, bool snapToGrid);
            void moveVertices(Math::Direction direction);
            
            Vec3 pasteObjectsDelta(const BBox3& bounds) const;

            void OnPopupReparentBrushes(wxCommandEvent& event);
            void OnPopupMoveBrushesToWorld(wxCommandEvent& event);
            void OnPopupCreatePointEntity(wxCommandEvent& event);
            void OnPopupCreateBrushEntity(wxCommandEvent& event);
            void OnUpdatePopupMenuItem(wxUpdateUIEvent& event);
        private:
            void updateReparentBrushesMenuItem(wxUpdateUIEvent& event) const;
            void updateMoveBrushesToWorldMenuItem(wxUpdateUIEvent& event) const;
            Model::Entity* findNewBrushParent(const Model::BrushList& brushes) const;
            bool canReparentBrushes(const Model::BrushList& brushes, const Model::Entity* newParent) const;
            void reparentBrushes(const Model::BrushList brushes, Model::Entity* newParent);
            Assets::EntityDefinition* findEntityDefinition(const Assets::EntityDefinitionGroups& groups, const size_t index) const;
            void createPointEntity(const Assets::PointEntityDefinition& definition);
            void createBrushEntity(const Assets::BrushEntityDefinition& definition);
            
            Vec3 moveDirection(Math::Direction direction) const;
            Vec3f centerCameraOnObjectsPosition(const Model::EntityList& entities, const Model::BrushList& brushes);
            
            void bindObservers();
            void unbindObservers();
            
            void objectDidChange(Model::Object* object);
            void selectionDidChange(const Model::SelectionResult& result);
            
            void createTools();
            void deleteTools();

            void doResetCamera();
            void doInitializeGL();

            void doRender();
            void setupGL(Renderer::RenderContext& context);
            void clearBackground(Renderer::RenderContext& context);
            void renderCoordinateSystem(Renderer::RenderContext& context);
            void renderCoordinateSystem(const Color& xColor, const Color& yColor, const Color& zColor);
            void renderMap(Renderer::RenderContext& context);
            void renderSelectionGuide(Renderer::RenderContext& context);
            void renderCompass(Renderer::RenderContext& context);
            void renderFocusRect(Renderer::RenderContext& context);
            
            void doShowPopupMenu();
            wxMenu* makeEntityGroupsMenu(const Assets::EntityDefinitionGroups& groups, int id);

            void bindEvents();

            static const Renderer::RenderResources::GLAttribs& attribs();
            static int depthBits();
            static bool multisample();
            static Renderer::TextureFont& defaultFont(Renderer::RenderResources& renderResources);
        };
    }
}

#endif /* defined(__TrenchBroom__MapView__) */
