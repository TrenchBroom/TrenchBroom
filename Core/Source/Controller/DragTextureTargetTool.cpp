/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DragTextureTargetTool.h"
#include "Controller/Tool.h"
#include "Model/Assets/Texture.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Face.h"
#include "Model/Map/Picker.h"
#include "Model/Selection.h"

namespace TrenchBroom {
    namespace Controller {
        bool DragTextureTargetTool::accepts(const DragInfo& info) {
            return info.name == "Texture";
        }
        
        bool DragTextureTargetTool::drop(const DragInfo& info) {
            if (info.name == "Texture") {
                Model::Assets::Texture* texture = static_cast<Model::Assets::Texture*>(info.payload);

                Model::Hit* hit = info.event.hits->first(Model::TB_HT_FACE, false);
                if (hit != NULL) {
                    Model::Face* face = static_cast<Model::Face*>(hit->object);
                    if (!face->selected()) {
                        Model::Brush* brush = face->brush();
                        Model::Selection& selection = m_editor.map().selection();
                        selection.removeAll();
                        selection.addBrush(*brush);
                        m_editor.map().setTexture(texture);
                    }
                    return true;
                }
            }
            
            return false;
        }
    }
}