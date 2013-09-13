/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
            Model::EntityBrushesMap frontBrushes;
            Model::EntityBrushesMap backBrushes;
        };
        
        class Clipper {
        private:
            enum ClipSide {
                Front,
                Back,
                Both
            };
            
            class ClipPoints {
            private:
                bool m_valid;
                Vec3 m_points[3];
            public:
                ClipPoints();
                ClipPoints(const Vec3& point1, const Vec3& point2, const Vec3& point3);
                void invert();
                bool valid() const;
                const Vec3& operator[](const size_t index) const;
                const Vec3* points() const;
            };
            
            const Renderer::Camera& m_camera;
            ClipSide m_clipSide;
            Vec3 m_points[3];
            Vec3::List m_normals[3];
            size_t m_numPoints;

    public:
            Clipper(const Renderer::Camera& camera);
            bool clipPointValid(const Vec3& point) const;
            void addClipPoint(const Vec3& point, const Model::BrushFace& face);
            void deleteLastClipPoint();
            void toggleClipSide();
            void reset();
            ClipResult clip(const Model::BrushList& brushes, const View::MapDocumentPtr document) const;
        private:
            bool isIdenticalWithAnyPoint(const Vec3& point) const;
            bool isLinearlyDependent(const Vec3& point) const;
            Vec3::List getNormals(const Vec3& point, const Model::BrushFace& face) const;
            Vec3::List getNormals(const Model::BrushFaceList& faces) const;
            ClipPoints clipPoints() const;
            Vec3 selectNormal(const Vec3::List& normals1, const Vec3::List& normals2) const;
            void setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace& frontFace, Model::BrushFace& backFace) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Clipper__) */
