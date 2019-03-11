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

#ifndef ControlListBox_h
#define ControlListBox_h

#include <QWidget>

#include <vector>

class QLabel;
class QListWidget;

namespace TrenchBroom {
    namespace View {
        class ControlListBox : public QWidget {
            Q_OBJECT
        protected:
            class Item : public QWidget {
            public:
                Item(QWidget* parent);
                virtual ~Item() override;

                virtual void setSelectionColours(const wxColour& foreground, const wxColour& background);
                virtual void setDefaultColours(const wxColour& foreground, const wxColour& background);
            protected:
                void setColours(QWidget* window, const wxColour& foreground, const wxColour& background);
            };
        private:
            QListWidget* m_list;
        public:
            ControlListBox(QWidget* parent, bool restrictToClientWidth, const QString& emptyText = "");

            size_t GetItemCount() const;
            int GetSelection() const;
            
            void SetItemCount(size_t itemCount);
            void SetSelection(int index);
            void MakeVisible(size_t index);
            void MakeVisible(const Item* item);
            void MakeVisible(wxCoord y, wxCoord size);
            
            void SetItemMargin(const wxSize& margin);
            void SetShowLastDivider(bool showLastDivider);
            
            void SetEmptyText(const QString& emptyText);

        private:
            virtual Item* createItem(QWidget* parent, const wxSize& margins, size_t index) = 0;
        };
    }
}

#endif /* ControlListBox_h */
