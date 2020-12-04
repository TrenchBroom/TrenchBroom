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

#pragma once

#include "View/SmartAttributeEditor.h"

#include <memory>
#include <vector>

class QString;
class QWidget;
class QScrollArea;

namespace TrenchBroom {
    namespace View {
        class FlagsEditor;
        class MapDocument;

        class SmartFlagsEditor : public SmartAttributeEditor {
            Q_OBJECT
        private:
            static const size_t NumFlags = 24;
            static const size_t NumCols = 3;

            QScrollArea* m_scrolledWindow;
            FlagsEditor* m_flagsEditor;
            bool m_ignoreUpdates;
        public:
            explicit SmartFlagsEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void createGui();
            void doUpdateVisual(const std::vector<Model::AttributableNode*>& attributables) override;
            void resetScrollPos();

            void getFlags(const std::vector<Model::AttributableNode*>& attributables, QStringList& labels, QStringList& tooltips) const;
            void getFlagValues(const std::vector<Model::AttributableNode*>& attributables, int& setFlags, int& mixedFlags) const;
            int getFlagValue(const Model::AttributableNode* attributable) const;

            void flagChanged(size_t index, int value, int setFlag, int mixedFlag);
        };
    }
}


