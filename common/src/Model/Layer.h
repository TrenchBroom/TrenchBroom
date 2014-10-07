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

#ifndef __TrenchBroom__Layer__
#define __TrenchBroom__Layer__

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        class Layer : public Node {
        private:
            String m_name;
            bool m_hidden;
            bool m_locked;
        public:
            Layer(const String& name);
            
            const String& name() const;
            void setName(const String& name);
            
            bool hidden() const;
            void setHidden(bool hidden);
            
            bool locked() const;
            void setLocked(bool locked);
        private: // implement Node interface
            Node* doClone(const BBox3& worldBounds) const;
            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            bool doRemoveIfEmpty() const;
            bool doSelectable() const;
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
        private:
            Layer(const Layer&);
            Layer& operator=(const Layer&);
        };
    }
}

#endif /* defined(__TrenchBroom__Layer__) */
