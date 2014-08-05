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

#include "MixedBrushContentsIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Issue.h"
#include "Model/Object.h"
#include "Model/QuickFix.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <cassert>
#include <set>

namespace TrenchBroom {
    namespace Model {
        class MixedBrushContentsQuickFix : public QuickFix {
        public:
            static const QuickFixType Type;
        private:
            int m_contentFlags;
        public:
            MixedBrushContentsQuickFix(const int contentFlags, const String& description) :
            QuickFix(Type, description),
            m_contentFlags(contentFlags) {}

            void apply(Brush* brush, View::ControllerSPtr controller) const {
                controller->setContentFlags(brush->faces(), m_contentFlags);
            }
        };
        
        const QuickFixType MixedBrushContentsQuickFix::Type = QuickFix::freeType();
        
        class MixedBrushContentsIssue : public BrushIssue {
        public:
            static const IssueType Type;
        public:
            MixedBrushContentsIssue(Brush* brush, const GameConfig::FlagsConfig& flagsConfig) :
            BrushIssue(Type, brush) {
                typedef std::set<int> SeenFlags;
                SeenFlags seen;
                seen.insert(0);
                
                const BrushFaceList& faces = BrushIssue::brush()->faces();
                for (size_t i = 0; i < faces.size(); ++i) {
                    const int surfaceContents = faces[i]->surfaceContents();
                    if (seen.count(surfaceContents) == 0) {
                        const StringList flagNames = flagsConfig.flagNames(surfaceContents);
                        const String description = "Set brush content flags to " + StringUtils::join(flagNames, ", ");
                        addDeletableQuickFix(new MixedBrushContentsQuickFix(surfaceContents, description));
                        seen.insert(surfaceContents);
                    }
                }

                addDeletableQuickFix(new MixedBrushContentsQuickFix(0, "Reset brush content flags"));
            }

            String description() const {
                return "Brush has mixed content flags";
            }
            
            void applyQuickFix(const QuickFix* quickFix, View::ControllerSPtr controller) {
                if (quickFix->type() == MixedBrushContentsQuickFix::Type)
                    static_cast<const MixedBrushContentsQuickFix*>(quickFix)->apply(brush(), controller);
            }
        };
        
        const IssueType MixedBrushContentsIssue::Type = Issue::freeType();
        
        IssueType MixedBrushContentsIssueGenerator::type() const {
            return MixedBrushContentsIssue::Type;
        }
        
        const String& MixedBrushContentsIssueGenerator::description() const {
            static const String description("Mixed brush content flags");
            return description;
        }

        MixedBrushContentsIssueGenerator::MixedBrushContentsIssueGenerator(const GameConfig::FlagsConfig& flagsConfig) :
        m_flagsConfig(flagsConfig) {}

        void MixedBrushContentsIssueGenerator::generate(Brush* brush, IssueList& issues) const {
            assert(brush != NULL);
            const BrushFaceList& faces = brush->faces();
            BrushFaceList::const_iterator it = faces.begin();
            BrushFaceList::const_iterator end = faces.end();
            assert(it != end);
            
            const int contentFlags = (*it)->surfaceContents();
            ++it;
            while (it != end) {
                if ((*it)->surfaceContents() != contentFlags)
                    issues.push_back(new MixedBrushContentsIssue(brush, m_flagsConfig));
                ++it;
            }
        }
    }
}
