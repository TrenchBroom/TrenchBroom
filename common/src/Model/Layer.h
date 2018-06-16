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

#ifndef TrenchBroom_Layer
#define TrenchBroom_Layer

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        class Layer : public Node {
        private:
            String m_name;

            mutable BBox3 m_bounds;
            mutable bool m_boundsValid;
        public:
            Layer(const String& name, const BBox3& worldBounds);
            
            void setName(const String& name);
        private: // implement Node interface
            const String& doGetName() const override;
            const BBox3& doGetBounds() const override;
            
            Node* doClone(const BBox3& worldBounds) const override;
            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;
            void doNodeBoundsDidChange(const BBox3& oldBounds) override;
            bool doSelectable() const override;

            void doPick(const Ray3& ray, PickResult& pickResult) const override;
            void doFindNodesContaining(const Vec3& point, NodeList& result) override;

            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;

            FloatType doIntersectWithRay(const Ray3& ray) const override;
        private:
            void invalidateBounds();
            void validateBounds() const;
        private:
            Layer(const Layer&);
            Layer& operator=(const Layer&);
        };
    }
}

#endif /* defined(TrenchBroom_Layer) */
