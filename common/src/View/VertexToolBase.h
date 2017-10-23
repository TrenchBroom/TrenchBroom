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

#ifndef VertexToolBase_h
#define VertexToolBase_h

#include "VecMath.h"
#include "TrenchBroom.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class Grid;
        
        class VertexToolBase : public Tool {
        public:
            typedef enum {
                MR_Continue,
                MR_Deny,
                MR_Cancel
            } MoveResult;
        protected:
            MapDocumentWPtr m_document;
        private:
            size_t m_changeCount;
        protected:
            bool m_ignoreChangeNotifications;
            
            Vec3 m_dragHandlePosition;
            bool m_dragging;
        protected:
            VertexToolBase(MapDocumentWPtr document);
        public:
            virtual ~VertexToolBase();
        public:
            const Grid& grid() const;

            const Model::BrushList& selectedBrushes() const;
        public:
            template <typename M, typename H>
            Model::BrushSet findIncidentBrushes(const M& manager, const H& handle) const {
                const Model::BrushList& brushes = selectedBrushes();
                return manager.findIncidentBrushes(handle, std::begin(brushes), std::end(brushes));
            }
        protected: // Tool interface
            virtual bool doActivate();
            virtual bool doDeactivate();
        };
    }
}

#endif /* VertexToolBase_h */
