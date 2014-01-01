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

#include "FloatPointsIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Issue.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class FloatPointsIssue : public Issue {
        private:
            static const QuickFixType SnapPointsToIntegerFix = 0;
            static const QuickFixType FindIntegerPointsFix = 1;
            static const IssueType Type;
            
            Brush* m_brush;
        public:
            FloatPointsIssue(Brush* brush) :
            Issue(Type),
            m_brush(brush) {
                addQuickFix(QuickFix(SnapPointsToIntegerFix, Type, "Snap plane points to integer"));
                addQuickFix(QuickFix(FindIntegerPointsFix, Type, "Find integer plane points"));
            }
            
            String description() const {
                return "Brush has floating point plane points";
            }
            
            void select(View::ControllerSPtr controller) {
                controller->selectObject(*m_brush);
            }
            
            void applyQuickFix(const QuickFixType fixType, View::ControllerSPtr controller) {
                switch (fixType) {
                    case SnapPointsToIntegerFix:
                        snapPointsToInteger(controller);
                        break;
                    case FindIntegerPointsFix:
                        findIntegerPoints(controller);
                        break;
                    default:
                        break;
                }
            }
        private:
            void snapPointsToInteger(View::ControllerSPtr controller) {
                controller->snapPlanePoints(*m_brush);
            }
            
            void findIntegerPoints(View::ControllerSPtr controller) {
                controller->findPlanePoints(*m_brush);
            }

            bool doGetIgnore(IssueType type) const {
                return m_brush->ignoredIssues() & type;
            }
            
            void doSetIgnore(IssueType type, const bool ignore) {
                m_brush->setIgnoreIssue(type, ignore);
            }
        };
        
        const IssueType FloatPointsIssue::Type = Issue::freeType();
        
        Issue* FloatPointsIssueGenerator::generate(Brush* brush) const {
            assert(brush != NULL);
            const BrushFaceList& faces = brush->faces();
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const BrushFace* face = *it;
                const BrushFace::Points& points = face->points();
                for (size_t i = 0; i < 3; ++i) {
                    const Vec3& point = points[i];
                    if (!point.isInteger())
                        return new FloatPointsIssue(brush);
                }
            }
            
            return NULL;
        }
    }
}
