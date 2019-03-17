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

#ifndef TrenchBroom_EntityAttributeEditor
#define TrenchBroom_EntityAttributeEditor

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxTextCtrl;

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
        class AttributeDefinition;
    }

    namespace View {
        class Selection;
        class SplitterWindow2;
        class EntityAttributeGrid;
        class EntityAttributeSelectedCommand;
        class SmartAttributeEditorManager;

        /**
         * Panel containing the EntityAttributeGrid (the key/value editor table),
         * smart editor, and documentation text view.
         */
        class EntityAttributeEditor : public wxPanel {
        private:
            View::MapDocumentWPtr m_document;
            EntityAttributeGrid* m_attributeGrid;
            SmartAttributeEditorManager* m_smartEditorManager;
            SplitterWindow2* m_documentationSplitter;
            wxTextCtrl* m_documentationText;
            String m_lastSelectedAttributeName;
            const Assets::EntityDefinition* m_currentDefinition;
        public:
            EntityAttributeEditor(wxWindow* parent, MapDocumentWPtr document);
            ~EntityAttributeEditor() override;

            void OnIdle(wxIdleEvent& event);
        private:
            void bindObservers();
            void unbindObservers();

            void selectionDidChange(const Selection& selection);
            void nodesDidChange(const Model::NodeList& nodes);

            void updateIfSelectedEntityDefinitionChanged();
            void updateDocumentationAndSmartEditor();

            /**
             * Returns a description of the options for ChoiceAttributeOption and FlagsAttributeDefinition,
             * other subclasses return an empty string.
             */
            static wxString optionDescriptions(const Assets::AttributeDefinition& definition);

            void updateDocumentation(const String &attributeName);
            void createGui(wxWindow* parent, MapDocumentWPtr document);
        };
    }
}

#endif /* defined(TrenchBroom_EntityAttributeEditor) */
