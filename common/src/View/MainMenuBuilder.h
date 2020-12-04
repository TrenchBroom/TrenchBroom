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

#include "View/Actions.h"

#include <functional>

class QAction;
class QMenu;
class QMenuBar;

namespace TrenchBroom {
    namespace View {
        /**
         * Builds actions for a menu and saves them in the given ActionMap.
         */
        class MenuBuilderBase {
        protected:
            using ActionMap = std::map<const Action*, QAction*>;
            using TriggerFn = std::function<void(const Action&)>;
            ActionMap& m_actions;
            TriggerFn m_triggerFn;
        protected:
            MenuBuilderBase(ActionMap& actions, const TriggerFn& triggerFn);
        public:
            virtual ~MenuBuilderBase();

            /**
             * Updates the key sequence and tooltip of the given QAction to match the given Action.
             */
            static void updateActionKeySeqeunce(QAction* qAction, const Action* tAction);
        protected:
            QAction* findOrCreateQAction(const Action* tAction);
        };

        class MainMenuBuilder : public MenuVisitor, public MenuBuilderBase {
        private:
            QMenuBar& m_menuBar;

            QMenu* m_currentMenu;

            // special menu items and actions
        public:
            QMenu* recentDocumentsMenu;
            QAction* undoAction;
            QAction* redoAction;
            QAction* pasteAction;
            QAction* pasteAtOriginalPositionAction;
        public:
            MainMenuBuilder(QMenuBar& menuBar, ActionMap& actions, const TriggerFn& triggerFn);

            void visit(const Menu& menu) override;
            void visit(const MenuSeparatorItem& item) override;
            void visit(const MenuActionItem& item) override;
       };
    }
}



