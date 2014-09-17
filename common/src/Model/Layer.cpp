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

#include "Layer.h"

#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        Layer::Layer(const String& name) :
        m_name(name) {}
        
        const String& Layer::name() const {
            return m_name;
        }
        
        void Layer::setName(const String& name) {
            m_name = name;
        }

        class CanAddChildToLayer : public ConstNodeVisitor {
        private:
            bool m_result;
        public:
            CanAddChildToLayer() :
            m_result(false) {}
            
            bool result() const {
                return m_result;
            }
        private:
            void doVisit(const World* world)   { m_result = false; }
            void doVisit(const Layer* layer)   { m_result = false; }
            void doVisit(const Group* group)   { m_result = true; }
            void doVisit(const Entity* entity) { m_result = true; }
            void doVisit(const Brush* brush)   { m_result = true; }
        };

        bool Layer::doCanAddChild(Node* child) const {
            CanAddChildToLayer visitor;
            child->accept(visitor);
            return visitor.result();
        }
        
        bool Layer::doCanRemoveChild(Node* child) const {
            return true;
        }
        
        void Layer::doAncestorDidChange() {}
        
        void Layer::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Layer::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }
    }
}
