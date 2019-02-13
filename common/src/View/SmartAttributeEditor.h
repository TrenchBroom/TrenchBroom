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

#include <QObject>

class QWidget;

namespace TrenchBroom {
    namespace View {
        /**
         * Subclasses of this build the widgets for the SmartAttributeEditorManager
         */
        class SmartAttributeEditor : public QObject {
            Q_OBJECT
        private:
            View::MapDocumentWPtr m_document;
            
            Model::AttributeName m_name;
            Model::AttributableNodeList m_attributables;
            bool m_active;
        public:
            SmartAttributeEditor(QObject* parent, View::MapDocumentWPtr document);
            virtual ~SmartAttributeEditor();
            
            bool usesName(const Model::AttributeName& name) const;
            
            QWidget* activate(QWidget* parent, const Model::AttributeName& name);
            void update(const Model::AttributableNodeList& attributables);
            void deactivate();
        protected:
            View::MapDocumentSPtr document() const;
            const Model::AttributeName& name() const;
            const Model::AttributableNodeList attributables() const;
            void addOrUpdateAttribute(const Model::AttributeValue& value);
        private:
            /**
             * Creates the smart editor's widget and sets its owner to `parent`.
             */
            QWidget* createVisual(QWidget* parent);
            void destroyVisual();
            void updateVisual(const Model::AttributableNodeList& attributables);
            
            virtual QWidget* doCreateVisual(QWidget* parent) = 0;
            virtual void doDestroyVisual() = 0;
            virtual void doUpdateVisual(const Model::AttributableNodeList& attributables) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_SmartAttributeEditor) */
