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

#ifndef __TrenchBroom__Clipper__
#define __TrenchBroom__Clipper__

#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        struct ClipResult {
            Model::ParentChildrenMap frontBrushes;
            Model::ParentChildrenMap backBrushes;
        };
        
        class Clipper {
        private:
            enum ClipSide {
                ClipSide_Front,
                ClipSide_Back,
                ClipSide_Both
            };
            
            struct ClipHandlePoint {
                Vec3 position;
                Vec3::List normals;
            };
            
            class ClipHandlePoints {
            private:
                ClipHandlePoint m_points[3];
                size_t m_numPoints;
            public:
                ClipHandlePoints();
                
                size_t numPoints() const;
                size_t indexOfPoint(const Vec3& position) const;
                const ClipHandlePoint& operator[](size_t index) const;

                bool canAddPoint(const Vec3& position) const;
                void addPoint(const Vec3& position, const Vec3::List& normals);
                
                bool canUpdatePoint(size_t index, const Vec3& position);
                void updatePoint(size_t index, const Vec3& position, const Vec3::List& normals);
                
                void deleteLastPoint();
                void deleteAllPoints();
            private:
                bool identicalWithAnyPoint(const Vec3& position, size_t disregardIndex) const;
                bool linearlyDependent(const Vec3& p1, const Vec3& p2, const Vec3& p3) const;
            };
            
            class ClipPoints {
            private:
                bool m_valid;
                Vec3 m_points[3];
            public:
                ClipPoints(const ClipHandlePoints& handlePoints);

                bool valid() const;
                const Vec3& operator[](size_t index) const;
                const Vec3* points() const;
                
                void invertPlaneNormal();
            private:
                static Vec3 selectNormal(const Vec3::List& normals1, const Vec3::List& normals2);
            };
            
            ClipHandlePoints m_handlePoints;
            ClipPoints m_clipPoints;
            ClipSide m_clipSide;
    public:
            Clipper();

            size_t numPoints() const;
            Vec3::List clipPointPositions() const;
            size_t indexOfPoint(const Vec3& position) const;

            bool keepFrontBrushes() const;
            bool keepBackBrushes() const;

            void setPoints(const Vec3& position, const Model::BrushFace* face, const Renderer::Camera& camera);
            
            bool canAddClipPoint(const Vec3& position) const;
            void addClipPoint(const Vec3& position, const Model::BrushFace* face, const Renderer::Camera& camera);

            bool canUpdateClipPoint(size_t index, const Vec3& position);
            void updateClipPoint(size_t index, const Vec3& position, const Model::BrushFace* face);

            void deleteLastClipPoint();
            void reset();

            void toggleClipSide();
            ClipResult clip(const Model::BrushList& brushes, const View::MapDocumentSPtr document) const;
        private:
            Vec3::List getNormals(const Vec3& point, const Model::BrushFace* face) const;
            Vec3::List getNormals(const Model::BrushFaceList& faces) const;
            
            void updateClipPoints();
            void setClipPlaneNormal(const Renderer::Camera& camera);
            void setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace* frontFace, Model::BrushFace* backFace) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Clipper__) */
