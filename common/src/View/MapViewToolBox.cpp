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
#include "View/EdgeTool.h"
#include "View/FaceTool.h"
#include "View/MapDocument.h"
#include "View/MoveObjectsTool.h"
#include "View/ResizeBrushesTool.h"
#include "View/RotateObjectsTool.h"
#include "View/ScaleObjectsTool.h"
#include "View/ShearObjectsTool.h"
#include "View/SelectionTool.h"
#include "View/VertexTool.h"

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

        ScaleObjectsTool* MapViewToolBox::scaleObjectsTool() const {
            return m_scaleObjectsTool.get();
        }

        ShearObjectsTool* MapViewToolBox::shearObjectsTool() const {
            return m_shearObjectsTool.get();
        }

        VertexTool* MapViewToolBox::vertexTool() const {
            return m_vertexTool.get();
        }

        EdgeTool* MapViewToolBox::edgeTool() const {
            return m_edgeTool.get();
        }

        FaceTool* MapViewToolBox::faceTool() const {
            return m_faceTool.get();
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

        vm::vec3 MapViewToolBox::rotateToolCenter() const {
            assert(rotateObjectsToolActive());
            return m_rotateObjectsTool->rotationCenter();
        }

        void MapViewToolBox::moveRotationCenter(const vm::vec3& delta) {
            assert(rotateObjectsToolActive());
            const vm::vec3 center = m_rotateObjectsTool->rotationCenter();
            m_rotateObjectsTool->setRotationCenter(center + delta);
        }

        void MapViewToolBox::toggleScaleObjectsTool() {
            toggleTool(scaleObjectsTool());
        }

        bool MapViewToolBox::scaleObjectsToolActive() const {
            return toolActive(scaleObjectsTool());
        }

        void MapViewToolBox::toggleShearObjectsTool() {
            toggleTool(shearObjectsTool());
        }

        bool MapViewToolBox::shearObjectsToolActive() const {
            return toolActive(shearObjectsTool());
        }

        bool MapViewToolBox::anyVertexToolActive() const {
            return vertexToolActive() || edgeToolActive() || faceToolActive();
        }

        void MapViewToolBox::toggleVertexTool() {
            toggleTool(vertexTool());
        }

        bool MapViewToolBox::vertexToolActive() const {
            return toolActive(vertexTool());
        }

        void MapViewToolBox::toggleEdgeTool() {
            toggleTool(edgeTool());
        }

        bool MapViewToolBox::edgeToolActive() const {
            return toolActive(edgeTool());
        }

        void MapViewToolBox::toggleFaceTool() {
            toggleTool(faceTool());
        }

        bool MapViewToolBox::faceToolActive() const {
            return toolActive(faceTool());
        }

        void MapViewToolBox::moveVertices(const vm::vec3& delta) {
            assert(anyVertexToolActive());
            if (vertexToolActive())
                vertexTool()->moveSelection(delta);
            else if (edgeToolActive())
                edgeTool()->moveSelection(delta);
            else if (faceToolActive())
                faceTool()->moveSelection(delta);
        }

        void MapViewToolBox::createTools(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl) {
            m_clipTool.reset(new ClipTool(document));
            m_createComplexBrushTool.reset(new CreateComplexBrushTool(document));
            m_createEntityTool.reset(new CreateEntityTool(document));
            m_createSimpleBrushTool.reset(new CreateSimpleBrushTool(document));
            m_moveObjectsTool.reset(new MoveObjectsTool(document));
            m_resizeBrushesTool.reset(new ResizeBrushesTool(document));
            m_rotateObjectsTool.reset(new RotateObjectsTool(document));
            m_scaleObjectsTool.reset(new ScaleObjectsTool(document));
            m_shearObjectsTool.reset(new ShearObjectsTool(document));
            m_vertexTool.reset(new VertexTool(document));
            m_edgeTool.reset(new EdgeTool(document));
            m_faceTool.reset(new FaceTool(document));

            deactivateWhen(createComplexBrushTool(), moveObjectsTool());
            deactivateWhen(createComplexBrushTool(), resizeBrushesTool());
            deactivateWhen(createComplexBrushTool(), createSimpleBrushTool());
            deactivateWhen(rotateObjectsTool(), moveObjectsTool());
            deactivateWhen(rotateObjectsTool(), resizeBrushesTool());
            deactivateWhen(rotateObjectsTool(), createSimpleBrushTool());
            deactivateWhen(scaleObjectsTool(), moveObjectsTool());
            deactivateWhen(scaleObjectsTool(), resizeBrushesTool());
            deactivateWhen(scaleObjectsTool(), createSimpleBrushTool());
            deactivateWhen(shearObjectsTool(), moveObjectsTool());
            deactivateWhen(shearObjectsTool(), resizeBrushesTool());
            deactivateWhen(shearObjectsTool(), createSimpleBrushTool());
            deactivateWhen(vertexTool(), moveObjectsTool());
            deactivateWhen(vertexTool(), resizeBrushesTool());
            deactivateWhen(vertexTool(), createSimpleBrushTool());
            deactivateWhen(edgeTool(), moveObjectsTool());
            deactivateWhen(edgeTool(), resizeBrushesTool());
            deactivateWhen(edgeTool(), createSimpleBrushTool());
            deactivateWhen(faceTool(), moveObjectsTool());
            deactivateWhen(faceTool(), resizeBrushesTool());
            deactivateWhen(faceTool(), createSimpleBrushTool());
            deactivateWhen(clipTool(), moveObjectsTool());
            deactivateWhen(clipTool(), resizeBrushesTool());
            deactivateWhen(clipTool(), createSimpleBrushTool());

            registerTool(moveObjectsTool(), bookCtrl);
            registerTool(rotateObjectsTool(), bookCtrl);
            registerTool(scaleObjectsTool(), bookCtrl);
            registerTool(shearObjectsTool(), bookCtrl);
            registerTool(resizeBrushesTool(), bookCtrl);
            registerTool(createComplexBrushTool(), bookCtrl);
            registerTool(clipTool(), bookCtrl);
            registerTool(vertexTool(), bookCtrl);
            registerTool(edgeTool(), bookCtrl);
            registerTool(faceTool(), bookCtrl);
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
