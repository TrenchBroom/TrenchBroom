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

#include "MapViewToolBox.h"
#include "Model/EditorContext.h"
#include "View/ClipTool.h"
#include "View/CreateComplexBrushTool.h"
#include "View/CreateEntityTool.h"
#include "View/CreateSimpleBrushTool.h"
#include "View/MoveObjectsTool.h"
#include "View/ResizeBrushesTool.h"
#include "View/RotateObjectsTool.h"
#include "View/SelectionTool.h"
#include "View/VertexTool.h"

namespace TrenchBroom {
    namespace View {
        MapViewToolBox::MapViewToolBox(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl) :
        m_document(document),
        m_clipTool(NULL),
        m_createComplexBrushTool(NULL),
        m_createEntityTool(NULL),
        m_createSimpleBrushTool(NULL),
        m_moveObjectsTool(NULL),
        m_resizeBrushesTool(NULL),
        m_rotateObjectsTool(NULL),
        m_selectionTool(NULL),
        m_vertexTool(NULL) {
            createTools(document, bookCtrl);
            bindObservers();
        }
        
        MapViewToolBox::~MapViewToolBox() {
            unbindObservers();
            destroyTools();
        }
        
        ClipTool* MapViewToolBox::clipTool() {
            return m_clipTool;
        }

        CreateComplexBrushTool* MapViewToolBox::createComplexBrushTool() {
            return m_createComplexBrushTool;
        }
        
        CreateEntityTool* MapViewToolBox::createEntityTool() {
            return m_createEntityTool;
        }

        CreateSimpleBrushTool* MapViewToolBox::createSimpleBrushTool() {
            return m_createSimpleBrushTool;
        }

        MoveObjectsTool* MapViewToolBox::moveObjectsTool() {
            return m_moveObjectsTool;
        }
        
        ResizeBrushesTool* MapViewToolBox::resizeBrushesTool() {
            return m_resizeBrushesTool;
        }
        
        RotateObjectsTool* MapViewToolBox::rotateObjectsTool() {
            return m_rotateObjectsTool;
        }
        
        SelectionTool* MapViewToolBox::selectionTool() {
            return m_selectionTool;
        }
        
        VertexTool* MapViewToolBox::vertexTool() {
            return m_vertexTool;
        }

        void MapViewToolBox::toggleCreateComplexBrushTool() {
            toggleTool(m_createComplexBrushTool);
        }
        
        bool MapViewToolBox::createComplexBrushToolActive() const {
            return toolActive(m_createComplexBrushTool);
        }

        void MapViewToolBox::toggleClipTool() {
            toggleTool(m_clipTool);
        }
        
        bool MapViewToolBox::clipToolActive() const {
            return toolActive(m_clipTool);
        }
        
        void MapViewToolBox::toggleClipSide() {
            assert(clipToolActive());
            m_clipTool->toggleSide();
        }
            
        void MapViewToolBox::performClip() {
            assert(clipToolActive());
            m_clipTool->performClip();
        }
        
        void MapViewToolBox::removeLastClipPoint() {
            assert(clipToolActive());
            m_clipTool->removeLastPoint();
        }

        void MapViewToolBox::toggleRotateObjectsTool() {
            toggleTool(m_rotateObjectsTool);
        }
        
        bool MapViewToolBox::rotateObjectsToolActive() const {
            return toolActive(m_rotateObjectsTool);
        }

        double MapViewToolBox::rotateToolAngle() const {
            assert(rotateObjectsToolActive());
            return m_rotateObjectsTool->angle();
        }
        
        const Vec3 MapViewToolBox::rotateToolCenter() const {
            assert(rotateObjectsToolActive());
            return m_rotateObjectsTool->rotationCenter();
        }

        void MapViewToolBox::moveRotationCenter(const Vec3& delta) {
            assert(rotateObjectsToolActive());
            const Vec3 center = m_rotateObjectsTool->rotationCenter();
            m_rotateObjectsTool->setRotationCenter(center + delta);
        }

        void MapViewToolBox::toggleVertexTool() {
            toggleTool(m_vertexTool);
        }
        
        bool MapViewToolBox::vertexToolActive() const {
            return toolActive(m_vertexTool);
        }

        void MapViewToolBox::moveVertices(const Vec3& delta) {
            assert(vertexToolActive());
            m_vertexTool->moveVerticesAndRebuildBrushGeometry(delta);
        }

        void MapViewToolBox::createTools(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl) {
            m_clipTool = new ClipTool(document);
            m_createComplexBrushTool = new CreateComplexBrushTool(document);
            m_createEntityTool = new CreateEntityTool(document);
            m_createSimpleBrushTool = new CreateSimpleBrushTool(document);
            m_moveObjectsTool = new MoveObjectsTool(document);
            m_resizeBrushesTool = new ResizeBrushesTool(document);
            m_rotateObjectsTool = new RotateObjectsTool(document);
            m_selectionTool = new SelectionTool(document);
            m_vertexTool = new VertexTool(document);
            
            deactivateWhen(m_createComplexBrushTool, m_moveObjectsTool);
            deactivateWhen(m_createComplexBrushTool, m_resizeBrushesTool);
            deactivateWhen(m_createComplexBrushTool, m_createSimpleBrushTool);
            deactivateWhen(m_rotateObjectsTool, m_moveObjectsTool);
            deactivateWhen(m_rotateObjectsTool, m_resizeBrushesTool);
            deactivateWhen(m_rotateObjectsTool, m_createSimpleBrushTool);
            deactivateWhen(m_vertexTool, m_moveObjectsTool);
            deactivateWhen(m_vertexTool, m_resizeBrushesTool);
            deactivateWhen(m_vertexTool, m_createSimpleBrushTool);
            deactivateWhen(m_clipTool, m_moveObjectsTool);
            deactivateWhen(m_clipTool, m_resizeBrushesTool);
            deactivateWhen(m_clipTool, m_createSimpleBrushTool);
            
            registerTool(m_moveObjectsTool, bookCtrl);
            registerTool(m_rotateObjectsTool, bookCtrl);
            registerTool(m_resizeBrushesTool, bookCtrl);
            registerTool(m_createComplexBrushTool, bookCtrl);
            registerTool(m_clipTool, bookCtrl);
            registerTool(m_vertexTool, bookCtrl);
            registerTool(m_createEntityTool, bookCtrl);
            registerTool(m_selectionTool, bookCtrl);
            registerTool(m_createSimpleBrushTool, bookCtrl);
        }
        
        void MapViewToolBox::destroyTools() {
            delete m_vertexTool;
            delete m_selectionTool;
            delete m_rotateObjectsTool;
            delete m_resizeBrushesTool;
            delete m_moveObjectsTool;
            delete m_createSimpleBrushTool;
            delete m_createEntityTool;
            delete m_createComplexBrushTool;
            delete m_clipTool;
        }
        
        void MapViewToolBox::registerTool(Tool* tool, wxBookCtrlBase* bookCtrl) {
            tool->createPage(bookCtrl);
            addTool(tool);
        }

        void MapViewToolBox::bindObservers() {
            toolActivatedNotifier.addObserver(this, &MapViewToolBox::toolActivated);
            toolDeactivatedNotifier.addObserver(this, &MapViewToolBox::toolDeactivated);
        }
        
        void MapViewToolBox::unbindObservers() {
            toolActivatedNotifier.removeObserver(this, &MapViewToolBox::toolActivated);
            toolDeactivatedNotifier.removeObserver(this, &MapViewToolBox::toolDeactivated);
        }
        
        void MapViewToolBox::toolActivated(Tool* tool) {
            updateEditorContext();
            tool->showPage();
        }
        
        void MapViewToolBox::toolDeactivated(Tool* tool) {
            updateEditorContext();
            m_moveObjectsTool->showPage();
        }

        void MapViewToolBox::updateEditorContext() {
            MapDocumentSPtr document = lock(m_document);
            Model::EditorContext& editorContext = document->editorContext();
            editorContext.setBlockSelection(createComplexBrushToolActive());
        }
    }
}
