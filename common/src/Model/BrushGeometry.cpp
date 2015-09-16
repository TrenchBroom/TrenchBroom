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

#include "BrushGeometry.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        void restoreFaceLinks(BrushGeometry* geometry) {
            assert(geometry != NULL);
            restoreFaceLinks(*geometry);
        }
        
        void restoreFaceLinks(BrushGeometry& geometry) {
            BrushGeometry::Face* first = geometry.faces().front();
            BrushGeometry::Face* current = first;
            do {
                BrushFace* face = current->payload();
                if (face != NULL)
                    face->setGeometry(current);
                current = current->next();
            } while (current != first);
        }

        SetTempFaceLinks::SetTempFaceLinks(Brush* brush, BrushGeometry& tempGeometry) :
        m_brush(brush) {
            assert(m_brush != NULL);
            restoreFaceLinks(tempGeometry);
        }
        
        SetTempFaceLinks::~SetTempFaceLinks() {
            restoreFaceLinks(m_brush->m_geometry);
            assert(m_brush->checkGeometry());
        }
    }
}
