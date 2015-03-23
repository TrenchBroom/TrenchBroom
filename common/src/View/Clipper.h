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

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class BrushRenderer;
    }
    
    namespace Model {
        class ModelFactory;
    }
    
    namespace View {
        class Clipper {
        private:
            enum ClipSide {
                ClipSide_Front,
                ClipSide_Both,
                ClipSide_Back
            };
        private:
            MapDocumentWPtr m_document;
            
            ClipSide m_clipSide;
            Model::ParentChildrenMap m_frontBrushes;
            Model::ParentChildrenMap m_backBrushes;
            
            Renderer::BrushRenderer* m_remainingBrushRenderer;
            Renderer::BrushRenderer* m_clippedBrushRenderer;
        public:
            Clipper(MapDocumentWPtr document);
        protected:
            void update();
        private:
            void clearBrushes();
            void clipBrushes(const Vec3& point1, const Vec3& point2, const Vec3& point3);
            
            bool canClip() const;
            void setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace* frontFace, Model::BrushFace* backFace) const;
            
            void clearRenderers();
            void updateRenderers();
            
            class AddBrushesToRendererVisitor;
            void addBrushesToRenderer(const Model::ParentChildrenMap& map, Renderer::BrushRenderer* renderer);
            
            bool keepFrontBrushes() const;
            bool keepBackBrushes() const;
        private:
            bool doCanClip() const;
            bool doGetPoints(Vec3& point1, Vec3& point2, Vec3& point3) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Clipper__) */
