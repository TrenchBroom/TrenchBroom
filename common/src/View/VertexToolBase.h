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
#include "Model/Hit.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class Grid;
        class Lasso;
        
        class VertexToolBase : public Tool {
        public:
            static const Model::Hit::HitType VertexHandleHit;
            static const Model::Hit::HitType EdgeHandleHit;
            static const Model::Hit::HitType FaceHandleHit;
            static const Model::Hit::HitType SplitHandleHit;
            static const Model::Hit::HitType AnyHandleHit;
            
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
            
            virtual void pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const = 0;
        public: // Handle selection
            virtual bool select(const Model::Hit::List& hits, bool addToSelection) = 0;
            virtual void select(const Lasso& lasso, bool modifySelection) = 0;
            virtual bool deselectAll() = 0;
        protected: // Tool interface
            virtual bool doActivate();
            virtual bool doDeactivate();
        };
    }
}

#endif /* VertexToolBase_h */
