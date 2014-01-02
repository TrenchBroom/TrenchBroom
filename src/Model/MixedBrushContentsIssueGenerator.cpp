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

#include "MixedBrushContentsIssueGenerator.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Issue.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <cassert>
#include <set>

namespace TrenchBroom {
    namespace Model {
        class MixedBrushContentsIssue : public Issue {
        private:
            static const IssueType Type;
            
            Brush* m_brush;
        public:
            MixedBrushContentsIssue(Brush* brush, const GameConfig::FlagsConfig& flagsConfig) :
            Issue(Type),
            m_brush(brush) {
                typedef std::set<int> SeenFlags;
                SeenFlags seen;
                seen.insert(0);
                
                const BrushFaceList& faces = m_brush->faces();
                for (size_t i = 0; i < faces.size(); ++i) {
                    const int surfaceContents = faces[i]->surfaceContents();
                    if (seen.count(surfaceContents) == 0) {
                        const StringList flagNames = flagsConfig.flagNames(surfaceContents);
                        const String str = "Set brush content flags to " + StringUtils::join(flagNames, ", ");
                        addQuickFix(QuickFix(i, Type, str));
                        seen.insert(surfaceContents);
                    }
                }

                addQuickFix(QuickFix(faces.size(), Type, "Reset brush content flags"));
            }
            
            size_t filePosition() const {
                return m_brush->filePosition();
            }

            String description() const {
                return "Brush has mixed content flags";
            }
            
            void select(View::ControllerSPtr controller) {
                controller->selectObject(*m_brush);
            }
            
            void applyQuickFix(const QuickFixType fixType, View::ControllerSPtr controller) {
                const BrushFaceList& faces = m_brush->faces();
                if (fixType == faces.size()) {
                    controller->setContentFlags(faces, 0);
                } else {
                    const BrushFace* faceTemplate = faces[fixType];
                    controller->setContentFlags(faces, faceTemplate->surfaceContents());
                }
            }
        private:
            bool doGetIgnore(IssueType type) const {
                return m_brush->ignoredIssues() & type;
            }

            void doSetIgnore(IssueType type, const bool ignore) {
                m_brush->setIgnoreIssue(type, ignore);
            }
        };
        
        const IssueType MixedBrushContentsIssue::Type = Issue::freeType();
        
        MixedBrushContentsIssueGenerator::MixedBrushContentsIssueGenerator(const GameConfig::FlagsConfig& flagsConfig) :
        m_flagsConfig(flagsConfig) {}

        Issue* MixedBrushContentsIssueGenerator::generate(Brush* brush) const {
            assert(brush != NULL);
            const BrushFaceList& faces = brush->faces();
            BrushFaceList::const_iterator it = faces.begin();
            BrushFaceList::const_iterator end = faces.end();
            assert(it != end);
            
            const int contentFlags = (*it)->surfaceContents();
            ++it;
            while (it != end) {
                if ((*it)->surfaceContents() != contentFlags)
                    return new MixedBrushContentsIssue(brush, m_flagsConfig);
                ++it;
            }

            return NULL;
        }
    }
}
