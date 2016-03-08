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

#include "ActionContext.h"

namespace TrenchBroom {
    namespace View {
        String actionContextName(const int actionContext) {
            if (actionContext == ActionContext_Any)
                return "Any";
            
            StringList actionContexts;
            if (actionContext & ActionContext_NodeSelection)
                actionContexts.push_back("Objects");
            if (actionContext & ActionContext_FaceSelection)
                actionContexts.push_back("Textures");

            if ((actionContext & ActionContext_AnyTool) == ActionContext_AnyTool) {
                actionContexts.push_back("Any Tool");
            } else {
                if (actionContext & ActionContext_CreateComplexBrushTool)
                    actionContexts.push_back("Create Brush Tool");
                if (actionContext & ActionContext_VertexTool)
                    actionContexts.push_back("Vertex Tool");
                if (actionContext & ActionContext_ClipTool)
                    actionContexts.push_back("Clip Tool");
                if (actionContext & ActionContext_RotateTool)
                    actionContexts.push_back("Rotate Tool");
            }

            if (actionContext & ActionContext_FlyMode)
                actionContexts.push_back("Fly Mode");
            
            return StringUtils::join(actionContexts, ", ");
        }
    }
}
