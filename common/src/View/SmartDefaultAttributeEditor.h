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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "View/SmartPropertyEditor.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        /**
         * Placeholder for when there is no smart editor. Just an empty QWidget.
         */
        class SmartDefaultAttributeEditor : public SmartAttributeEditor {
            Q_OBJECT
        public:
            explicit SmartDefaultAttributeEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void doUpdateVisual(const std::vector<Model::EntityNodeBase*>& attributables) override;
        };
    }
}

