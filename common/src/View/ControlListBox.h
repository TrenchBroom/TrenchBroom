/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef ControlListBox_h
#define ControlListBox_h

#include <wx/event.h>
#include <wx/scrolwin.h>

#include <vector>

wxDECLARE_EVENT(wxEVT_LISTBOX_RCLICK, wxCommandEvent);

class wxStaticText;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class ControlListBox : public wxScrolledWindow {
        protected:
            class Item : public wxWindow {
            public:
                Item(wxWindow* parent);
                virtual ~Item();
                
                bool AcceptsFocus() const;
                
                virtual void setSelectionColours(const wxColour& foreground, const wxColour& background);
                virtual void setDefaultColours(const wxColour& foreground, const wxColour& background);
            protected:
                void setColours(wxWindow* window, const wxColour& foreground, const wxColour& background);
            };
        private:
            typedef std::vector<Item*> ItemList;
            wxSize m_itemMargin;
            bool m_restrictToClientWidth;
            wxString m_emptyText;
            wxStaticText* m_emptyTextLabel;
            bool m_showLastDivider;
            bool m_valid;
            size_t m_newItemCount;
        protected:
            ItemList m_items;
            size_t m_selectionIndex;
        private:
            class Sizer;
        public:
            ControlListBox(wxWindow* parent, bool restrictToClientWidth, const wxString& emptyText = "");

            size_t GetItemCount() const;
            int GetSelection() const;
            
            void SetItemCount(size_t itemCount);
            void SetSelection(int index);
            void MakeVisible(size_t index);
            void MakeVisible(const Item* item);
            void MakeVisible(wxCoord y, wxCoord size);
            
            void SetItemMargin(const wxSize& margin);
            void SetShowLastDivider(bool showLastDivider);
            
            void SetEmptyText(const wxString& emptyText);
        private:
            void invalidate();
            void validate();
            void refresh(size_t itemCount);
            void bindEvents(wxWindow* window, size_t itemIndex);
            
            void OnIdle(wxIdleEvent& event);
            void OnSize(wxSizeEvent& event);
            void OnFocusChild(wxFocusEvent& event);
            void OnLeftClickChild(wxMouseEvent& event);
            void OnRightClickChild(wxMouseEvent& event);
            void OnDoubleClickChild(wxMouseEvent& event);
            void OnLeftClickVoid(wxMouseEvent& event);
            void sendEvent(wxMouseEvent& event);
            
            void setSelection(const wxEvent& event);
            void setSelection(size_t index);
        private:
            virtual Item* createItem(wxWindow* parent, const wxSize& margins, size_t index) = 0;
        };
    }
}

#endif /* ControlListBox_h */
