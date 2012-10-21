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

#ifndef __TrenchBroom__SetFaceAttributesCommand__
#define __TrenchBroom__SetFaceAttributesCommand__

#include "Controller/SnapshotCommand.h"
#include "Model/FaceTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Face;
        class MapDocument;
        class Texture;
    }
    
    namespace Controller {
        class SetFaceAttributesCommand : public SnapshotCommand {
        protected:
            Model::FaceList m_faces;
            
            float m_xOffset;
            float m_yOffset;
            float m_xScale;
            float m_yScale;
            float m_rotation;
            Model::Texture* m_texture;
            
            bool m_setXOffset;
            bool m_setYOffset;
            bool m_setXScale;
            bool m_setYScale;
            bool m_setRotation;
            bool m_setTexture;

            bool performDo();
            bool performUndo();

        public:
            SetFaceAttributesCommand(Model::MapDocument& document, const Model::FaceList& faces, const wxString& name);

            inline void setXOffset(float xOffset) {
                m_xOffset = xOffset;
                m_setXOffset = true;
            }

            inline void setYOffset(float yOffset) {
                m_yOffset = yOffset;
                m_setYOffset = true;
            }
            
            inline void setXScale(float xScale) {
                m_xScale = xScale;
                m_setXScale = true;
            }
            
            inline void setYScale(float yScale) {
                m_yScale = yScale;
                m_setYScale = true;
            }

            inline void setRotation(float rotation) {
                m_rotation = rotation;
                m_setRotation = true;
            }

            inline void setTexture(Model::Texture* texture) {
                m_texture = texture;
                m_setTexture = true;
            }
            
            void setTemplate(const Model::Face& face);
        };
    }
}

#endif /* defined(__TrenchBroom__SetFaceAttributesCommand__) */
