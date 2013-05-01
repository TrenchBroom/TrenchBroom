/*
 Copyright (C) 2010-2012 Kristian Duske

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

#ifndef __TrenchBroom__SmartPropertyEditor__
#define __TrenchBroom__SmartPropertyEditor__

#include "Model/EntityProperty.h"
#include "Model/EntityTypes.h"
#include "Utility/SharedPointer.h"
#include "Utility/String.h"
#include "View/DocumentViewHolder.h"

#include <map>

#include <wx/wx.h>

class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }

    namespace View {
        class EditorView;
        class SmartPropertyEditorManager;

        class SmartPropertyEditor {
        private:
            SmartPropertyEditorManager& m_manager;
            bool m_active;
            Model::PropertyKey m_property;
        protected:
            virtual wxWindow* createVisual(wxWindow* parent) = 0;
            virtual void destroyVisual() = 0;
            virtual void updateVisual() = 0;

            Model::MapDocument& document() const;
            EditorView& view() const;

            const Model::EntityList selectedEntities() const;
            void setPropertyValue(const Model::PropertyValue& value, const wxString& commandName);

            inline const Model::PropertyKey& property() const {
                return m_property;
            }
        public:
            SmartPropertyEditor(SmartPropertyEditorManager& manager);
            virtual ~SmartPropertyEditor() {}

            void activate(wxWindow* parent);
            void deactivate();
            void update();
            void setProperty(const Model::PropertyKey& property);
        };

        class DefaultPropertyEditor : public SmartPropertyEditor {
        private:
            wxStaticText* m_text;
        protected:
            virtual wxWindow* createVisual(wxWindow* parent);
            virtual void destroyVisual();
            virtual void updateVisual();
        public:
            DefaultPropertyEditor(SmartPropertyEditorManager& manager);
            virtual ~DefaultPropertyEditor();
        };

        class SmartPropertyEditorManager {
        private:
            typedef std::tr1::shared_ptr<SmartPropertyEditor> EditorPtr;
            typedef std::map<Model::PropertyKey, EditorPtr> EditorMap;

            DocumentViewHolder& m_documentViewHolder;
            wxWindow* m_panel;

            EditorMap m_editors;
            SmartPropertyEditor* m_defaultEditor;
            SmartPropertyEditor* m_activeEditor;

            void activateEditor(SmartPropertyEditor* editor, const Model::PropertyKey& property);
            void deactivateEditor();
        public:
            SmartPropertyEditorManager(wxWindow* parent, DocumentViewHolder& documentViewHolder);
            ~SmartPropertyEditorManager();

            void selectEditor(const Model::PropertyKey& key);
            void updateEditor();

            inline Model::MapDocument& document() const {
                return m_documentViewHolder.document();
            }

            inline EditorView& view() const {
                return m_documentViewHolder.view();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__SmartPropertyEditor__) */
