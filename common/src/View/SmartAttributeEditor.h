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

#include <memory>
#include <string>
#include <vector>

#include <QWidget>

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;
    }

    namespace View {
        class MapDocument;

        class SmartAttributeEditor : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;

            std::string m_name;
            std::vector<Model::AttributableNode*> m_attributables;
            bool m_active;
        public:
            explicit SmartAttributeEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            virtual ~SmartAttributeEditor();

            bool usesName(const std::string& name) const;

            void activate(const std::string& name);
            void update(const std::vector<Model::AttributableNode*>& attributables);
            void deactivate();
        protected:
            std::shared_ptr<MapDocument> document() const;
            const std::string& name() const;
            const std::vector<Model::AttributableNode*> attributables() const;
            void addOrUpdateAttribute(const std::string& value);
        private:
            virtual void doUpdateVisual(const std::vector<Model::AttributableNode*>& attributables) = 0;
        };
    }
}


