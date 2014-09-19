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
        m_contentTypeValid(true) {
            addFaces(faces);
            rebuildGeometry(worldBounds);
        }

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

        void Brush::addFaces(const BrushFaceList& faces) {
            addFaces(faces.begin(), faces.end(), faces.size());
        }

        void Brush::addFace(BrushFace* face) {
            assert(face != NULL);
            assert(face->brush() == NULL);
            assert(!VectorUtils::contains(m_faces, face));
            
            m_faces.push_back(face);
            face->setBrush(this);
            invalidateContentType();
            if (face->selected())
                incFamilyMemberSelectionCount(1);
        }

        void Brush::removeFace(BrushFace* face) {
            m_faces.erase(doRemoveFace(face), m_faces.end());
        }

        BrushFaceList::iterator Brush::doRemoveFace(BrushFace* face) {
            assert(face != NULL);

            BrushFaceList::iterator it = std::remove(m_faces.begin(), m_faces.end(), face);
            assert(it != m_faces.end());
            detachFace(face);
            return it;
        }

        void Brush::detachFaces(const BrushFaceList& faces) {
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it)
                detachFace(*it);
        }

        void Brush::detachFace(BrushFace* face) {
            assert(face != NULL);
            assert(face->brush() == this);

            if (face->selected())
                decFamilyMemberSelectionCount(1);
            face->setBrush(NULL);
        }
        
        void Brush::rebuildGeometry(const BBox3& worldBounds) {
            delete m_geometry;
            m_geometry = new BrushGeometry(worldBounds);
            const AddFaceResult result = m_geometry->addFaces(m_faces);
            
            // detach all faces and just clear the face list instead of removing all faces for better performance
            detachFaces(m_faces);
            m_faces.clear();
            
            VectorUtils::deleteAll(result.droppedFaces);

            BrushFaceList::const_iterator it, end;
            for (it = result.addedFaces.begin(), result.addedFaces.end(); it != end; ++it) {
                BrushFace* face = *it;
                addFace(face);
                face->invalidate();
            }

            childDidChange();
        }

        void Brush::invalidateContentType() {
            m_contentTypeValid = false;
        }
        
        bool Brush::doCanAddChild(const Node* child) const {
            return false;
        }
        
        bool Brush::doCanRemoveChild(const Node* child) const {
            return false;
        }
        
        bool Brush::doSelectable() const {
            return true;
        }
        
        void Brush::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Brush::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        const BBox3& Brush::doGetBounds() const {
            assert(m_geometry != NULL);
            return m_geometry->bounds;
        }
        
        void Brush::doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) {}
        bool Brush::doContains(const Node* node) const {}
        bool Brush::doIntersects(const Node* node) const {}
    }
}
