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

#pragma once

#include "Notifier.h"

class QWidget;
class QStackedLayout;

namespace TrenchBroom {
    namespace View {
        class Tool {
        private:
            bool m_active;

            QStackedLayout* m_book;
            int m_pageIndex;
        public:
            Notifier<Tool*> toolActivatedNotifier;
            Notifier<Tool*> toolDeactivatedNotifier;
            Notifier<Tool*> refreshViewsNotifier;
            Notifier<Tool*> toolHandleSelectionChangedNotifier;
        protected:
            explicit Tool(bool initiallyActive);
        public:
            virtual ~Tool();

            bool active() const;
            bool activate();
            bool deactivate();

            void refreshViews();
            void notifyToolHandleSelectionChanged();

            void createPage(QStackedLayout* book);
            void showPage();
        private:
            virtual bool doActivate();
            virtual bool doDeactivate();

            virtual QWidget* doCreatePage(QWidget* parent);
        };
    }
}

#endif /* defined(TrenchBroom_Tool) */
