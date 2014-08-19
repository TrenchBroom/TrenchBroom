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

#include "WorldBoundsIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/QuickFix.h"
#include "Model/SharedQuickFixes.h"
#include "View/ControllerFacade.h"

namespace TrenchBroom {
    namespace Model {
        class WorldBoundsIssue : public ObjectIssue {
        public:
            static const IssueType Type;
        public:
            WorldBoundsIssue(Object* object) :
            ObjectIssue(Type, object) {
                addSharedQuickFix(DeleteObjectQuickFix::instance());
            }
            
            String description() const {
                return "Object is out of world bounds";
            }
            
            void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller) {
                if (quickFix->type() == DeleteObjectQuickFix::Type)
                    static_cast<const DeleteObjectQuickFix*>(quickFix)->apply(object(), controller);
            }
        };
        
        WorldBoundsIssueGenerator::WorldBoundsIssueGenerator(const BBox3& worldBounds) :
        m_worldBounds(worldBounds) {}
        
        const IssueType WorldBoundsIssue::Type = Issue::freeType();

        IssueType WorldBoundsIssueGenerator::type() const {
            return WorldBoundsIssue::Type;
        }
        
        const String& WorldBoundsIssueGenerator::description() const {
            static const String description("Objects out of world bounds");
            return description;
        }
        
        void WorldBoundsIssueGenerator::generate(Entity* entity, IssueList& issues) const {
            doGenerate(entity, issues);
        }
        
        void WorldBoundsIssueGenerator::generate(Brush* brush, IssueList& issues) const {
            doGenerate(brush, issues);
        }

        void WorldBoundsIssueGenerator::doGenerate(Object* object, IssueList& issues) const {
            assert(object != NULL);
            if (!m_worldBounds.contains(object->bounds()))
                issues.push_back(new WorldBoundsIssue(object));
        }
    }
}
