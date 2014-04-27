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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__SmartPropertyEditor__
#define __TrenchBroom__SmartPropertyEditor__

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/event.h>

class wxWindow;

namespace TrenchBroom {
    namespace View {
        class SmartPropertyEditor : public wxEvtHandler {
        private:
            View::MapDocumentWPtr m_document;
            View::ControllerWPtr m_controller;
            
            Model::PropertyKey m_key;
            Model::EntityList m_entities;
            bool m_active;
        public:
            SmartPropertyEditor(View::MapDocumentWPtr document, View::ControllerWPtr controller);
            virtual ~SmartPropertyEditor();
            
            bool usesKey(const Model::PropertyKey& key) const;
            
            wxWindow* activate(wxWindow* parent, const Model::PropertyKey& key);
            void update(const Model::EntityList& entities);
            void deactivate();
        protected:
            View::MapDocumentSPtr document() const;
            View::ControllerSPtr controller() const;
            const Model::PropertyKey& key() const;
            const Model::EntityList entities() const;
            void addOrUpdateProperty(const Model::PropertyValue& value);
        private:
            wxWindow* createVisual(wxWindow* parent);
            void destroyVisual();
            void updateVisual(const Model::EntityList& entities);
            
            virtual wxWindow* doCreateVisual(wxWindow* parent) = 0;
            virtual void doDestroyVisual() = 0;
            virtual void doUpdateVisual(const Model::EntityList& entities) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__SmartPropertyEditor__) */
