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

#include "NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        BaseNodeVisitor::BaseNodeVisitor() :
        m_cancelled(false) {}
        
        BaseNodeVisitor::~BaseNodeVisitor() {}
        
        bool BaseNodeVisitor::cancelled() const {
            return m_cancelled;
        }
        
        void BaseNodeVisitor::cancel() {
            m_cancelled = true;
        }
        
        NodeVisitor::~NodeVisitor() {}
        
        void NodeVisitor::visit(World* world) {
            doVisit(world);
        }
        
        void NodeVisitor::visit(Layer* layer) {
            doVisit(layer);
        }
        
        void NodeVisitor::visit(Group* group) {
            doVisit(group);
        }
        
        void NodeVisitor::visit(Entity* entity) {
            doVisit(entity);
        }
        
        void NodeVisitor::visit(Brush* brush) {
            doVisit(brush);
        }
        
        ConstNodeVisitor::~ConstNodeVisitor() {}
        
        void ConstNodeVisitor::visit(const World* world) {
            doVisit(world);
        }
        
        void ConstNodeVisitor::visit(const Layer* layer) {
            doVisit(layer);
        }
        
        void ConstNodeVisitor::visit(const Group* group) {
            doVisit(group);
        }
        
        void ConstNodeVisitor::visit(const Entity* entity) {
            doVisit(entity);
        }
        
        void ConstNodeVisitor::visit(const Brush* brush) {
            doVisit(brush);
        }
    }
}
