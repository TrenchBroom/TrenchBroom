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

#include "MatchSelectableNodes.h"

#include "Model/EditorContext.h"

namespace TrenchBroom {
    namespace Model {
        MatchSelectableNodes::MatchSelectableNodes(const Model::EditorContext& editorContext) :
        m_editorContext(editorContext) {}

        bool MatchSelectableNodes::operator()(const Model::World* world) const   { return m_editorContext.selectable(world); }
        bool MatchSelectableNodes::operator()(const Model::LayerNode* layer) const   { return m_editorContext.selectable(layer); }
        bool MatchSelectableNodes::operator()(const Model::Group* group) const   { return m_editorContext.selectable(group); }
        bool MatchSelectableNodes::operator()(const Model::Entity* entity) const { return m_editorContext.selectable(entity); }
        bool MatchSelectableNodes::operator()(const Model::BrushNode* brush) const   { return m_editorContext.selectable(brush); }
    }
}
