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

#ifndef TRENCHBROOM_VIEW_FORWARD_H
#define TRENCHBROOM_VIEW_FORWARD_H

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;
        class MapDocument;

        class Grid;

        class Selection;

        class InputState;

        class Tool;
        class ClipTool;
        class CreateComplexBrushTool;
        class CreateEntityTool;
        class CreateSimpleBrushTool;
        class VertexTool;

        class Command;
        class UndoableCommand;

        class VertexHandleManager;
        class VertexHandleManagerBase;
        template <typename H> class VertexHandleManagerBaseT;

        class MapFrame;
        class GLContextManager;

        class MapViewBase;
        class MapViewToolBox;

        class CompilationContext;
        class CompilationRunner;
        class CompilationProfileManager;

        class BorderLine;
    }
}

#endif //TRENCHBROOM_VIEW_FORWARD_H
