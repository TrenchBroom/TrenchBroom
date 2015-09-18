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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Tool
#define TrenchBroom_Tool

#include "Notifier.h"

class wxBitmap;
class wxBookCtrlBase;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class Tool {
        private:
            bool m_active;

            wxBookCtrlBase* m_book;
            size_t m_pageIndex;
        public:
            Notifier1<Tool*> toolActivatedNotifier;
            Notifier1<Tool*> toolDeactivatedNotifier;
            Notifier1<Tool*> refreshViewsNotifier;
        protected:
            Tool(bool initiallyActive);
        public:
            virtual ~Tool();
            
            bool active() const;
            bool activate();
            bool deactivate();

            void refreshViews();
            
            void createPage(wxBookCtrlBase* book);
            void showPage();
            
            wxBitmap icon() const;
        private:
            virtual bool doActivate();
            virtual bool doDeactivate();

            virtual wxWindow* doCreatePage(wxWindow* parent);
            virtual String doGetIconName() const;
        };
    }
}

#endif /* defined(TrenchBroom_Tool) */
