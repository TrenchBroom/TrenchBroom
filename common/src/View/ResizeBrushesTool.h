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

#ifndef TrenchBroom_ResizeBrushesTool
#define TrenchBroom_ResizeBrushesTool

#include "TrenchBroom.h"
#include "Model/Hit.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

#include <vecmath/vec.h>
#include <vecmath/polygon.h>

#include <tuple>

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class Selection;
        
        class ResizeBrushesTool : public Tool {
        private:
            static const Model::Hit::HitType ResizeHit3D;
            static const Model::Hit::HitType ResizeHit2D;

            using FaceHandle = std::tuple<Model::Brush*, vm::vec3>;

            MapDocumentWPtr m_document;
            std::vector<FaceHandle> m_dragHandles;
            vm::vec3 m_dragOrigin;
            vm::vec3 m_totalDelta;
            bool m_splitBrushes;
            bool m_resizing;
        public:
            explicit ResizeBrushesTool(MapDocumentWPtr document);
            ~ResizeBrushesTool() override;
            
            bool applies() const;
            
            Model::Hit pick2D(const vm::ray3& pickRay, const Model::PickResult& pickResult);
            Model::Hit pick3D(const vm::ray3& pickRay, const Model::PickResult& pickResult);
        private:
            class PickProximateFace;
            Model::Hit pickProximateFace(Model::Hit::HitType hitType, const vm::ray3& pickRay) const;
        public:
            bool hasDragFaces() const;
            Model::BrushFaceList dragFaces() const;
            void updateDragFaces(const Model::PickResult& pickResult);
        private:
            std::vector<FaceHandle> getDragHandles(const Model::Hit& hit) const;
            class MatchFaceBoundary;
            std::vector<FaceHandle> collectDragHandles(const Model::Hit& hit) const;
            Model::BrushFaceList collectDragFaces(Model::BrushFace* face) const;
            std::vector<FaceHandle> getDragHandles(const Model::BrushFaceList& faces) const;
        public:
            bool beginResize(const Model::PickResult& pickResult, bool split);
            bool resize(const vm::ray3& pickRay, const Renderer::Camera& camera);
            vm::vec3 selectDelta(const vm::vec3& relativeDelta, const vm::vec3& absoluteDelta, FloatType mouseDistance) const;

            void commitResize();
            void cancelResize();
        private:
            bool splitBrushes(const vm::vec3& delta);
            Model::BrushFace* findMatchingFace(Model::Brush* brush, const Model::BrushFace* reference) const;
            std::vector<vm::polygon3> dragFaceDescriptors() const;
        private:
            void bindObservers();
            void unbindObservers();
            void nodesDidChange(const Model::NodeList& nodes);
            void selectionDidChange(const Selection& selection);
        };
    }
}

#endif /* defined(TrenchBroom_ResizeBrushesTool) */
