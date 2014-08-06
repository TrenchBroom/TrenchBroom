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
        IssueQuery::IssueQuery(Object* object) :
        m_object(object) {}

        int IssueQuery::compare(const Issue* issue) const {
            return issue->compare(m_object) > 0;
        }

        Issue::~Issue() {
            VectorUtils::clearAndDelete(m_deletableFixes);
        }
        
        IssueType Issue::freeType() {
            static size_t index = 0;
            assert(index < sizeof(IssueType) * 8);
            return 1 << index++;
        }

        int Issue::compare(const Issue* issue) const {
            return doCompare(issue);
        }

        int Issue::compare(const Object* object) const {
            return doCompare(object);
        }

        size_t Issue::seqIndex() const {
            return m_seqIndex;
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

        Issue::Issue(const IssueType type) :
        m_type(type) {
            static size_t seqIndex = 0;
            m_seqIndex = seqIndex++;
        }
        
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
        
        int EntityIssue::doCompare(const Issue* issue) const {
            return issue->compare(m_entity) > 0;
        }
        
        int EntityIssue::doCompare(const Object* object) const {
            if (m_entity < object)
                return -1;
            if (m_entity > object)
                return 1;
            return 0;
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
        
        int BrushIssue::doCompare(const Issue* issue) const {
            return issue->compare(m_brush) > 0;
        }
        
        int BrushIssue::doCompare(const Object* object) const {
            if (m_brush < object)
                return -1;
            if (m_brush > object)
                return 1;
            return 0;
        }
    }
}
