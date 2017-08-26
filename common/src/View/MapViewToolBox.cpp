/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
#include "View/MapDocument.h"
#include "View/MoveObjectsTool.h"
#include "View/ResizeBrushesTool.h"
#include "View/RotateObjectsTool.h"
#include "View/SelectionTool.h"
#include "View/VertexTool.h"
#include "View/VertexToolOld.h"

namespace TrenchBroom {
    namespace View {
        MapViewToolBox::MapViewToolBox(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl) :
        m_document(document) {
            createTools(document, bookCtrl);
            bindObservers();
        }
        
        MapViewToolBox::~MapViewToolBox() {
            unbindObservers();
        }
        
        ClipTool* MapViewToolBox::clipTool() const {
            return m_clipTool.get();
        }

        CreateComplexBrushTool* MapViewToolBox::createComplexBrushTool() const {
            return m_createComplexBrushTool.get();
        }
        
        CreateEntityTool* MapViewToolBox::createEntityTool() const {
            return m_createEntityTool.get();
        }

        CreateSimpleBrushTool* MapViewToolBox::createSimpleBrushTool() const {
            return m_createSimpleBrushTool.get();
        }

        MoveObjectsTool* MapViewToolBox::moveObjectsTool() const {
            return m_moveObjectsTool.get();
        }
        
        ResizeBrushesTool* MapViewToolBox::resizeBrushesTool() const {
            return m_resizeBrushesTool.get();
        }
        
        RotateObjectsTool* MapViewToolBox::rotateObjectsTool() const {
            return m_rotateObjectsTool.get();
        }
        
        VertexTool* MapViewToolBox::vertexTool() const {
            return m_vertexTool.get();
        }
        
        EdgeTool* MapViewToolBox::edgeTool() const {
            // TODO implement
            // return m_edgeTool.get();
            return nullptr;
        }
        
        FaceTool* MapViewToolBox::faceTool() const {
            // TODO implement
            // return m_faceTool.get();
            return nullptr;
        }

        VertexToolOld* MapViewToolBox::vertexToolOld() const {
            return m_vertexToolOld.get();
        }

        void MapViewToolBox::toggleCreateComplexBrushTool() {
            toggleTool(createComplexBrushTool());
        }
        
        bool MapViewToolBox::createComplexBrushToolActive() const {
            return toolActive(createComplexBrushTool());
        }

        void MapViewToolBox::performCreateComplexBrush() {
            m_createComplexBrushTool->createBrush();
        }

        void MapViewToolBox::toggleClipTool() {
            toggleTool(clipTool());
        }
        
        bool MapViewToolBox::clipToolActive() const {
            return toolActive(clipTool());
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
            toggleTool(rotateObjectsTool());
        }
        
        bool MapViewToolBox::rotateObjectsToolActive() const {
            return toolActive(rotateObjectsTool());
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
            toggleTool(vertexTool());
        }
        
        bool MapViewToolBox::vertexToolActive() {
            return toolActive(vertexTool());
        }
        
        void MapViewToolBox::toggleEdgeTool() {
            // TODO implement
        }
        
        bool MapViewToolBox::edgeToolActive() {
            // TODO implement
            return false;
        }
        
        void MapViewToolBox::toggleFaceTool() {
            // TODO implement
        }
        
        bool MapViewToolBox::faceToolActive() {
            // TODO implement
            return false;
        }

        void MapViewToolBox::toggleVertexToolOld() {
            toggleTool(vertexToolOld());
        }
        
        bool MapViewToolBox::vertexToolOldActive() const {
            return toolActive(vertexToolOld());
        }

        void MapViewToolBox::moveVertices(const Vec3& delta) {
            assert(vertexToolOldActive());
            m_vertexToolOld->moveVerticesAndRebuildBrushGeometry(delta);
        }

        void MapViewToolBox::createTools(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl) {
            m_clipTool.reset(new ClipTool(document));
            m_createComplexBrushTool.reset(new CreateComplexBrushTool(document));
            m_createEntityTool.reset(new CreateEntityTool(document));
            m_createSimpleBrushTool.reset(new CreateSimpleBrushTool(document));
            m_moveObjectsTool.reset(new MoveObjectsTool(document));
            m_resizeBrushesTool.reset(new ResizeBrushesTool(document));
            m_rotateObjectsTool.reset(new RotateObjectsTool(document));
            m_vertexTool.reset(new VertexTool(document));
            // TODO: create edge and face tool
            m_vertexToolOld.reset(new VertexToolOld(document));
            
            deactivateWhen(createComplexBrushTool(), moveObjectsTool());
            deactivateWhen(createComplexBrushTool(), resizeBrushesTool());
            deactivateWhen(createComplexBrushTool(), createSimpleBrushTool());
            deactivateWhen(rotateObjectsTool(), moveObjectsTool());
            deactivateWhen(rotateObjectsTool(), resizeBrushesTool());
            deactivateWhen(rotateObjectsTool(), createSimpleBrushTool());
            deactivateWhen(vertexToolOld(), moveObjectsTool());
            deactivateWhen(vertexToolOld(), resizeBrushesTool());
            deactivateWhen(vertexToolOld(), createSimpleBrushTool());
            deactivateWhen(vertexTool(), moveObjectsTool());
            deactivateWhen(vertexTool(), resizeBrushesTool());
            deactivateWhen(vertexTool(), createSimpleBrushTool());
            deactivateWhen(clipTool(), moveObjectsTool());
            deactivateWhen(clipTool(), resizeBrushesTool());
            deactivateWhen(clipTool(), createSimpleBrushTool());
            
            registerTool(moveObjectsTool(), bookCtrl);
            registerTool(rotateObjectsTool(), bookCtrl);
            registerTool(resizeBrushesTool(), bookCtrl);
            registerTool(createComplexBrushTool(), bookCtrl);
            registerTool(clipTool(), bookCtrl);
            registerTool(vertexTool(), bookCtrl);
            registerTool(vertexToolOld(), bookCtrl);
            registerTool(createEntityTool(), bookCtrl);
            registerTool(createSimpleBrushTool(), bookCtrl);
        }
        
        void MapViewToolBox::registerTool(Tool* tool, wxBookCtrlBase* bookCtrl) {
            tool->createPage(bookCtrl);
            addTool(tool);
        }

        void MapViewToolBox::bindObservers() {
            toolActivatedNotifier.addObserver(this, &MapViewToolBox::toolActivated);
            toolDeactivatedNotifier.addObserver(this, &MapViewToolBox::toolDeactivated);
            
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &MapViewToolBox::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MapViewToolBox::documentWasNewedOrLoaded);
        }
        
        void MapViewToolBox::unbindObservers() {
            toolActivatedNotifier.removeObserver(this, &MapViewToolBox::toolActivated);
            toolDeactivatedNotifier.removeObserver(this, &MapViewToolBox::toolDeactivated);
            
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.addObserver(this, &MapViewToolBox::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.addObserver(this, &MapViewToolBox::documentWasNewedOrLoaded);
            }
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

        void MapViewToolBox::documentWasNewedOrLoaded(MapDocument* document) {
            deactivateAllTools();
        }
    }
}
