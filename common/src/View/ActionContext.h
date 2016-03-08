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

#ifndef TrenchBroom_ActionContext_h
#define TrenchBroom_ActionContext_h

#include "StringUtils.h"

namespace TrenchBroom {
    namespace View {
        typedef enum {
            ActionContext_Default         = 1 << 1,
            ActionContext_VertexTool      = 1 << 2,
            ActionContext_CreateComplexBrushTool = 1 << 3,
            ActionContext_ClipTool        = 1 << 4,
            ActionContext_RotateTool      = 1 << 5,
            ActionContext_FlyMode         = 1 << 6,
            ActionContext_NodeSelection   = 1 << 7,
            ActionContext_FaceSelection   = 1 << 8,
            ActionContext_AnyTool         = ActionContext_VertexTool | ActionContext_ClipTool | ActionContext_RotateTool | ActionContext_CreateComplexBrushTool,
            ActionContext_Any             = ActionContext_Default | ActionContext_AnyTool | ActionContext_FlyMode | ActionContext_NodeSelection | ActionContext_FaceSelection
        } ActionContext;

        String actionContextName(int actionContext);

        typedef enum {
            ActionView_Map2D = 0,
            ActionView_Map3D = 1
        } ActionView;

        static const size_t NumActionViews = 2;
    }
}

#endif
