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

#include "ActionContext.h"

#include <kdl/string_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace View {
        bool actionContextMatches(ActionContext::Type lhs, ActionContext::Type rhs) {
            return actionContextMatches(lhs, rhs, ActionContext::AnyView) &&
                   (actionContextMatches(lhs, rhs, ActionContext::AnyTool) ||
                    actionContextMatches(lhs, rhs, ActionContext::AnySelection));
        }

        bool actionContextMatches(const ActionContext::Type lhs, const ActionContext::Type rhs, const ActionContext::Type mask) {
            return (lhs & rhs & mask) != 0;
        }

        std::string actionContextName(const ActionContext::Type actionContext) {
            if (actionContext == ActionContext::Any) {
                return "Any";
            }

            std::vector<std::string> actionContexts;
            if (actionContext & ActionContext::NodeSelection) {
                actionContexts.push_back("Objects");
            }
            if (actionContext & ActionContext::FaceSelection) {
                actionContexts.push_back("Textures");
            }

            if ((actionContext & ActionContext::AnyTool) == ActionContext::AnyTool) {
                actionContexts.push_back("Any Tool");
            } else {
                if (actionContext & ActionContext::CreateComplexBrushTool) {
                    actionContexts.push_back("Create Brush Tool");
                }
                if (actionContext & ActionContext::ClipTool) {
                    actionContexts.push_back("Clip Tool");
                }
                if (actionContext & ActionContext::RotateTool) {
                    actionContexts.push_back("Rotate Tool");
                }
                if (actionContext & ActionContext::AnyVertexTool) {
                    actionContexts.push_back("Any Vertex Tool");
                }
            }

            if ((actionContext & ActionContext::AnyView) == ActionContext::AnyView) {
                actionContexts.push_back("Any View");
            } else if (actionContext & ActionContext::View3D) {
                actionContexts.push_back("3D View");
            } else if (actionContext & ActionContext::View2D) {
                actionContexts.push_back("2D View");
            }

            return kdl::str_join(actionContexts, ", ");
        }
    }
}
