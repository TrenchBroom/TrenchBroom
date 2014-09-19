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

#include "Brush.h"

#include "CollectionUtils.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Entity.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        Brush::Brush(const BBox3& worldBounds, const BrushFaceList& faces) :
        m_geometry(NULL),
        m_contentType(0),
        m_transparent(false),
        m_contentTypeValid(true) {}

        Brush::~Brush() {
            delete m_geometry;
            m_geometry = NULL;
            VectorUtils::clearAndDelete(m_faces);
        }

        class FindBrushOwner : public NodeVisitor, public NodeQuery<Attributable*> {
        private:
            void doVisit(World* world)   { setResult(world); cancel(); }
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   {}
            void doVisit(Entity* entity) { setResult(entity); cancel(); }
            void doVisit(Brush* brush)   {}
        };
        
        Attributable* Brush::entity() const {
            if (parent() == NULL)
                return NULL;
            FindBrushOwner visitor;
            parent()->acceptAndEscalate(visitor);
            if (!visitor.hasResult())
                return NULL;
            return visitor.result();
        }

        const BrushFaceList& Brush::faces() const {
            return m_faces;
        }

        void Brush::faceDidChange() {
            invalidateContentType();
        }

        bool Brush::doCanAddChild(Node* child) const {
            return false;
        }
        
        bool Brush::doCanRemoveChild(Node* child) const {
            return false;
        }
        
        void Brush::doParentWillChange() {
            assert(!selected());
        }

        void Brush::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Brush::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        const BBox3& Brush::doGetBounds() const {}
        void Brush::doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) {}
        bool Brush::doContains(const Node* node) const {}
        bool Brush::doIntersects(const Node* node) const {}

        void Brush::wasSelected() {
            assert(parent() != NULL);
            parent()->childWasSelected();
        }
        
        void Brush::wasDeselected() {
            assert(parent() != NULL);
            parent()->childWasDeselected();
        }
    }
}
