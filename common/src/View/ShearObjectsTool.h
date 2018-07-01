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

#ifndef TrenchBroom_ShearObjectsTool
#define TrenchBroom_ShearObjectsTool

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Hit.h"
#include "Model/ModelTypes.h"
#include "View/Tool.h"
#include "View/ScaleObjectsTool.h"
#include "View/RotateObjectsHandle.h"
#include "View/ShearObjectsToolPage.h"
#include "BBox.h"

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }
    
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class ShearObjectsTool : public Tool {
        public:
            static const Model::Hit::HitType ScaleToolFaceHit;
            static const Model::Hit::HitType ScaleToolEdgeHit;
            static const Model::Hit::HitType ScaleToolCornerHit;

        private:
            MapDocumentWPtr m_document;
            ShearObjectsToolPage* m_toolPage;

            /**
             * Point on the initial pick ray that's closest to the handle being dragged.
             * Note, when dragging "back faces" the mouse can start far from the bbox.
             * In this case the m_dragOrigin can be far from the bbox being resized, and close to the camera instead.
             *
             */
            Vec3 m_dragOrigin;
            /**
             * Total amount of drag accumulated since beginResize()
             *
             * Only used to decide whether to commit the change at the end of the drag
             * in ShearObjectsTool::commitResize
             */
            Vec3 m_totalDelta;

            /**
             * debug temporary
             */
            Vec3 m_handlePos;

            Model::Hit m_dragStartHit; // contains the drag type (face/edge/corner)
            bool m_resizing;

            /**
             * bounds in beginResize()
             * Only valid during a drag (when m_resizing is true).
             */
            BBox3 m_bboxAtDragStart;
            bool m_isShearing;
            AnchorPos m_anchorPos;
            bool m_scaleAllAxes;
        public: // debug only
            
            Vec3 dragOrigin() const { return m_dragOrigin; }
            
        public:
            explicit ShearObjectsTool(MapDocumentWPtr document);
            ~ShearObjectsTool() override;
            
            bool applies() const;

            void pickBackSides(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
            void pick2D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
            void pick3D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
        public:
            BBox3 bounds() const;
        public:
        
            // getting highlighted handles
            std::vector<Polygon3f> polygonsHighlightedByDrag() const;

            Vec3 handlePos() const {
                return m_handlePos;
            }

            bool hasDragPolygon() const;
            Polygon3f dragPolygon() const;
            
            bool hasDragEdge() const;
            Edge3f dragEdge() const;
            
            bool hasDragCorner() const;
            Vec3f dragCorner() const;

            bool hasDragAnchor() const;
            Vec3f dragAnchor() const;

            /**
             * If inside a drag, returns the bbox at the start of the drag.
             * Otherwise, returns the current bounds(). for rendering sheared bbox.
             */
            BBox3 bboxAtDragStart() const;

            Mat4x4 bboxShearMatrix() const;
            Polygon3f shearHandle() const;
            /**
             * This can only be called when a drag is not in progress
             */
            void setShearing(bool shearing);
            bool isShearing() const;
            
            // regular handles
            Vec3::List cornerHandles() const;

            void updateDragFaces(const Model::PickResult& pickResult);

            // persist the state of modifier keys
            void setAnchorPos(AnchorPos pos);
            AnchorPos anchorPos() const;

            void setScaleAllAxes(bool allAxes);
            bool scaleAllAxes() const;

        public:
            bool beginResize(const Model::PickResult& pickResult);
            bool resize(const Ray3& pickRay, const Renderer::Camera& camera, bool vertical);
            
            void commitResize();
            void cancelResize();
        private:
            void bindObservers();
            void unbindObservers();
            void nodesDidChange(const Model::NodeList& nodes);
            void selectionDidChange(const Selection& selection);

        private:
            //wxWindow* doCreatePage(wxWindow* parent) override;
        };
    }
}

#endif /* defined(TrenchBroom_ShearObjectsTool) */
