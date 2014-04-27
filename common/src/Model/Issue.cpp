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

#include "Issue.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        Issue::~Issue() {
            VectorUtils::clearAndDelete(m_deletableFixes);
        }
        
        IssueType Issue::freeType() {
            static size_t index = 0;
            assert(index < sizeof(IssueType) * 8);
            return 1 << index++;
        }

        IssueType Issue::type() const {
            return m_type;
        }

        bool Issue::hasType(IssueType mask) const {
            return (m_type & mask) != 0;
        }

        const QuickFix::List& Issue::quickFixes() const {
            return m_quickFixes;
        }

        void Issue::applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller) {}

        bool Issue::isHidden() const {
            return doIsHidden(m_type);
        }

        void Issue::setHidden(const bool hidden) {
            doSetHidden(m_type, hidden);
        }

        Issue* Issue::previous() const {
            return m_previous;
        }
        
        Issue* Issue::next() const {
            return m_next;
        }

        void Issue::insertAfter(Issue* previous) {
            if (previous != NULL) {
                Issue* next = previous->m_next;
                
                Issue* lastSuccessor = this;
                while (lastSuccessor->next() != NULL)
                    lastSuccessor = lastSuccessor->next();
                
                m_previous = previous;
                m_previous->m_next = this;
                
                lastSuccessor->m_next = next;
                if (next != NULL)
                    next->m_previous = lastSuccessor;
            }
        }
        
        void Issue::insertBefore(Issue* next) {
            if (next != NULL) {
                Issue* previous = next->m_previous;
                
                Issue* lastSuccessor = this;
                while (lastSuccessor->next() != NULL)
                    lastSuccessor = lastSuccessor->next();
                
                if (previous != NULL)
                    previous->m_next = this;
                m_previous = previous;
                
                next->m_previous = lastSuccessor;
                lastSuccessor->m_next = next;
            }
        }
        
        void Issue::remove(Issue* last) {
            if (last == NULL)
                last = this;
            
            if (m_previous != NULL)
                m_previous->m_next = last->m_next;
            if (last->m_next != NULL)
                last->m_next->m_previous = m_previous;
            m_previous = NULL;
            last->m_next = NULL;
        }

        Issue::Issue(const IssueType type) :
        m_type(type),
        m_previous(NULL),
        m_next(NULL) {}
        
        void Issue::addSharedQuickFix(const QuickFix& quickFix) {
            m_quickFixes.push_back(&quickFix);
        }

        void Issue::addDeletableQuickFix(const QuickFix* quickFix) {
            m_quickFixes.push_back(quickFix);
            m_deletableFixes.push_back(quickFix);
        }

        size_t EntityIssue::filePosition() const {
            return entity()->filePosition();
        }
        
        void EntityIssue::select(View::ControllerSPtr controller) {
            const BrushList& brushes = entity()->brushes();
            if (brushes.empty())
                controller->selectObject(entity());
            else
                controller->selectObjects(VectorUtils::cast<Model::Object*>(brushes));
        }
        
        EntityIssue::EntityIssue(const IssueType type, Entity* entity) :
        Issue(type),
        m_entity(entity) {
            assert(m_entity != NULL);
        }
        
        Entity* EntityIssue::entity() const {
            return m_entity;
        }

        bool EntityIssue::doIsHidden(const IssueType type) const {
            return entity()->isIssueHidden(this);
        }
        
        void EntityIssue::doSetHidden(const IssueType type, const bool hidden) {
            entity()->setIssueHidden(type, hidden);
        }
        
        size_t BrushIssue::filePosition() const {
            return brush()->filePosition();
        }
        
        void BrushIssue::select(View::ControllerSPtr controller) {
            controller->selectObject(brush());
        }
        
        BrushIssue::BrushIssue(const IssueType type, Brush* brush) :
        Issue(type),
        m_brush(brush) {
            assert(brush != NULL);
        }
        
        Brush* BrushIssue::brush() const {
            return m_brush;
        }
        
        bool BrushIssue::doIsHidden(const IssueType type) const {
            return brush()->isIssueHidden(this);
        }
        
        void BrushIssue::doSetHidden(const IssueType type, const bool hidden) {
            brush()->setIssueHidden(type, hidden);
        }
    }
}
