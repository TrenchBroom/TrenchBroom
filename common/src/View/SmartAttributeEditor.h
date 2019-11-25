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

#ifndef TrenchBroom_SmartAttributeEditor
#define TrenchBroom_SmartAttributeEditor

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <vector>

#include <QWidget>

namespace TrenchBroom {
    namespace View {
        class SmartAttributeEditor : public QWidget {
            Q_OBJECT
        private:
            MapDocumentWPtr m_document;

            Model::AttributeName m_name;
            std::vector<Model::AttributableNode*> m_attributables;
            bool m_active;
        public:
            explicit SmartAttributeEditor(MapDocumentWPtr document, QWidget* parent = nullptr);
            virtual ~SmartAttributeEditor();

            bool usesName(const Model::AttributeName& name) const;

            void activate(const Model::AttributeName& name);
            void update(const std::vector<Model::AttributableNode*>& attributables);
            void deactivate();
        protected:
            MapDocumentSPtr document() const;
            const Model::AttributeName& name() const;
            const std::vector<Model::AttributableNode*> attributables() const;
            void addOrUpdateAttribute(const Model::AttributeValue& value);
        private:
            virtual void doUpdateVisual(const std::vector<Model::AttributableNode*>& attributables) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_SmartAttributeEditor) */
