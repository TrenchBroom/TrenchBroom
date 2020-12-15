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

#include "LongPropertyKeyIssueGenerator.h"

#include "Model/BrushNode.h"
#include "Model/RemoveEntityAttributesQuickFix.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/MapFacade.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class LongPropertyKeyIssueGenerator::LongPropertyKeyIssue : public EntityPropertyIssue {
        public:
            static const IssueType Type;
        private:
            const std::string m_propertyKey;
        public:
            LongPropertyKeyIssue(EntityNodeBase* node, const std::string& propertyKey) :
                EntityPropertyIssue(node),
                m_propertyKey(propertyKey) {}

            const std::string& propertyKey() const override {
                return m_propertyKey;
            }
        private:
            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                return "Entity property key '" + m_propertyKey.substr(0, 8) + "...' is too long.";
            }
        };

        const IssueType LongPropertyKeyIssueGenerator::LongPropertyKeyIssue::Type = Issue::freeType();

        LongPropertyKeyIssueGenerator::LongPropertyKeyIssueGenerator(const size_t maxLength) :
        IssueGenerator(LongPropertyKeyIssue::Type, "Long entity property keys"),
        m_maxLength(maxLength) {
            addQuickFix(new RemoveEntityAttributesQuickFix(LongPropertyKeyIssue::Type));
        }

        void LongPropertyKeyIssueGenerator::doGenerate(EntityNodeBase* node, IssueList& issues) const {
            for (const EntityProperty& property : node->entity().properties()) {
                const std::string& propertyKey = property.key();
                if (propertyKey.size() >= m_maxLength) {
                    issues.push_back(new LongPropertyKeyIssue(node, propertyKey));
                }
            }
        }
    }
}
