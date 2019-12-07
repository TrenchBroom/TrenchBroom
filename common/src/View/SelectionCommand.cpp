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

#include "SelectionCommand.h"

#include "Macros.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/World.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/string_format.h>

#include <sstream>
#include <string>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SelectionCommand::Type = Command::freeType();

        SelectionCommand::Ptr SelectionCommand::select(const std::vector<Model::Node*>& nodes) {
            return Ptr(new SelectionCommand(Action_SelectNodes, nodes, {}));
        }

        SelectionCommand::Ptr SelectionCommand::select(const std::vector<Model::BrushFace*>& faces) {
            return Ptr(new SelectionCommand(Action_SelectFaces, {}, faces));
        }

        SelectionCommand::Ptr SelectionCommand::convertToFaces() {
            return Ptr(new SelectionCommand(Action_ConvertToFaces, {}, {}));
        }

        SelectionCommand::Ptr SelectionCommand::selectAllNodes() {
            return Ptr(new SelectionCommand(Action_SelectAllNodes, {}, {}));
        }

        SelectionCommand::Ptr SelectionCommand::selectAllFaces() {
            return Ptr(new SelectionCommand(Action_SelectAllFaces, {}, {}));
        }

        SelectionCommand::Ptr SelectionCommand::deselect(const std::vector<Model::Node*>& nodes) {
            return Ptr(new SelectionCommand(Action_DeselectNodes, nodes, {}));
        }

        SelectionCommand::Ptr SelectionCommand::deselect(const std::vector<Model::BrushFace*>& faces) {
            return Ptr(new SelectionCommand(Action_DeselectFaces, {}, faces));
        }

        SelectionCommand::Ptr SelectionCommand::deselectAll() {
            return Ptr(new SelectionCommand(Action_DeselectAll, {}, {}));
        }

        static Model::BrushFaceReference::List faceRefs(const std::vector<Model::BrushFace*>& faces) {
            Model::BrushFaceReference::List result;
            for (Model::BrushFace* face : faces)
                result.push_back(Model::BrushFaceReference(face));
            return result;
        }

        static std::vector<Model::BrushFace*> resolveFaceRefs(const Model::BrushFaceReference::List& refs) {
            std::vector<Model::BrushFace*> result;
            for (const Model::BrushFaceReference& ref : refs)
                result.push_back(ref.resolve());
            return result;
        }

        SelectionCommand::SelectionCommand(const Action action, const std::vector<Model::Node*>& nodes, const std::vector<Model::BrushFace*>& faces) :
        UndoableCommand(Type, makeName(action, nodes, faces)),
        m_action(action),
        m_nodes(nodes),
        m_faceRefs(faceRefs(faces)) {}

        std::string SelectionCommand::makeName(const Action action, const std::vector<Model::Node*>& nodes, const std::vector<Model::BrushFace*>& faces) {
            std::stringstream result;
            switch (action) {
                case Action_SelectNodes:
                    result << "Select " << nodes.size() << " " << kdl::str_plural(nodes.size(), "Object", "Objects");
                    break;
                case Action_SelectFaces:
                    result << "Select " << faces.size() << " " << kdl::str_plural(nodes.size(), "Brush Face", "Brush Faces");
                    break;
                case Action_SelectAllNodes:
                    result << "Select All Objects";
                    break;
                case Action_SelectAllFaces:
                    result << "Select All Brush Faces";
                    break;
                case Action_ConvertToFaces:
                    result << "Convert to Brush Face Selection";
                    break;
                case Action_DeselectNodes:
                    result << "Deselect " << nodes.size() << " " << kdl::str_plural(nodes.size(), "Object", "Objects");
                    break;
                case Action_DeselectFaces:
                    result << "Deselect " << faces.size() << " " << kdl::str_plural(nodes.size(), "Brush Face", "Brush Faces");
                    break;
                case Action_DeselectAll:
                    return "Select None";
                switchDefault()
            }
            return result.str();
        }

        bool SelectionCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_previouslySelectedNodes = document->selectedNodes().nodes();
            m_previouslySelectedFaceRefs = faceRefs(document->selectedBrushFaces());

            switch (m_action) {
                case Action_SelectNodes:
                    document->performSelect(m_nodes);
                    break;
                case Action_SelectFaces:
                    document->performSelect(resolveFaceRefs(m_faceRefs));
                    break;
                case Action_SelectAllNodes:
                    document->performSelectAllNodes();
                    break;
                case Action_SelectAllFaces:
                    document->performSelectAllBrushFaces();
                    break;
                case Action_ConvertToFaces:
                    document->performConvertToBrushFaceSelection();
                    break;
                case Action_DeselectNodes:
                    document->performDeselect(m_nodes);
                    break;
                case Action_DeselectFaces:
                    document->performDeselect(resolveFaceRefs(m_faceRefs));
                    break;
                case Action_DeselectAll:
                    document->performDeselectAll();
                    break;
            }
            return true;
        }

        bool SelectionCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performDeselectAll();
            if (!m_previouslySelectedNodes.empty())
                document->performSelect(m_previouslySelectedNodes);
            if (!m_previouslySelectedFaceRefs.empty())
                document->performSelect(resolveFaceRefs(m_previouslySelectedFaceRefs));
            return true;
        }

        bool SelectionCommand::doIsRepeatDelimiter() const {
            return true;
        }

        bool SelectionCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SelectionCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
