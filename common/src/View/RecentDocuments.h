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

#ifndef TrenchBroom_RecentDocuments
#define TrenchBroom_RecentDocuments

#include "CollectionUtils.h"
#include "Notifier.h"
#include "IO/Path.h"

#include <vector>

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/event.h>
#include <wx/menu.h>

namespace TrenchBroom {
    namespace View {
        template <class EventHandler>
        class RecentDocuments {
        private:
            typedef void (EventHandler::*Function)(wxCommandEvent&);
            typedef std::vector<wxMenu*> MenuList;
            MenuList m_menus;
            EventHandler* m_handler;
            Function m_function;
            int m_baseId;
            size_t m_maxSize;
            IO::Path::List m_recentDocuments;
        public:
            Notifier0 didChangeNotifier;
        public:
            RecentDocuments(const int baseId, const size_t maxSize) :
            m_handler(NULL),
            m_function(NULL),
            m_baseId(baseId),
            m_maxSize(maxSize) {
                assert(m_maxSize > 0);
                loadFromConfig();
            }
            
            const IO::Path::List& recentDocuments() const {
                return m_recentDocuments;
            }

            void addMenu(wxMenu* menu) {
                assert(menu != NULL);
                clearMenu(menu);
                createMenuItems(menu);
                m_menus.push_back(menu);
            }
            
            void removeMenu(wxMenu* menu) {
                assert(menu != NULL);
                clearMenu(menu);
                VectorUtils::erase(m_menus, menu);
            }
            
            void setHandler(EventHandler* handler, Function function) {
                if (m_handler != NULL && m_function != NULL)
                    clearBindings();
                
                m_handler = handler;
                m_function = function;
                
                if (m_handler != NULL && m_function != NULL)
                    createBindings();
            }
            
            void updatePath(const IO::Path& path) {
                insertPath(path);
                updateMenus();
                updateBindings();
                saveToConfig();
                didChangeNotifier();
            }
            
            void removePath(const IO::Path& path) {
                const size_t oldSize = m_recentDocuments.size();
                
                const IO::Path canonPath = path.makeCanonical();
                VectorUtils::erase(m_recentDocuments, canonPath);
                
                if (oldSize > m_recentDocuments.size()) {
                    updateMenus();
                    updateBindings();
                    saveToConfig();
                    didChangeNotifier();
                }
            }
        private:
            void loadFromConfig() {
                m_recentDocuments.clear();
                wxConfigBase* conf = wxConfig::Get();
                for (size_t i = 0; i < m_maxSize; ++i) {
                    const wxString confName = wxString("RecentDocuments/") << i;
                    wxString value;
                    if (conf->Read(confName, &value))
                        m_recentDocuments.push_back(IO::Path(value.ToStdString()));
                    else
                        break;
                }
            }
            
            void saveToConfig() {
                wxConfigBase* conf = wxConfig::Get();
                conf->DeleteGroup("RecentDocuments");
                for (size_t i = 0; i < m_recentDocuments.size(); ++i) {
                    const wxString confName = wxString("RecentDocuments/") << i;
                    const wxString value = m_recentDocuments[i].asString();
                    conf->Write(confName, value);
                }
            }
            
            void updateBindings() {
                if (m_handler != NULL && m_function != NULL) {
                    clearBindings();
                    createBindings();
                }
            }
            
            void createBindings() {
                for (size_t i = 0; i < m_recentDocuments.size(); ++i) {
                    wxVariant* data = new wxVariant(wxString(m_recentDocuments[i].asString()));
                    const int windowId = m_baseId + static_cast<int>(i);
                    m_handler->Bind(wxEVT_MENU, m_function, m_handler, windowId, windowId, data);
                }
            }
            
            void clearBindings() {
                for (int i = 0; i < static_cast<int>(m_maxSize); ++i)
                    m_handler->Unbind(wxEVT_MENU, m_function, m_handler, m_baseId + i);
            }
            
            void insertPath(const IO::Path& path) {
                const IO::Path canonPath = path.makeCanonical();
                IO::Path::List::iterator it = std::find(m_recentDocuments.begin(), m_recentDocuments.end(), canonPath);
                if (it != m_recentDocuments.end())
                    m_recentDocuments.erase(it);
                m_recentDocuments.insert(m_recentDocuments.begin(), canonPath);
                if (m_recentDocuments.size() > m_maxSize)
                    m_recentDocuments.pop_back();
            }
            
            void updateMenus() {
                MenuList::iterator it, end;
                for (it = m_menus.begin(), end = m_menus.end(); it != end; ++it) {
                    wxMenu* menu = *it;
                    clearMenu(menu);
                    createMenuItems(menu);
                }
            }
            
            void clearMenu(wxMenu* menu) {
                while (menu->GetMenuItemCount() > 0) {
                    wxMenuItem* item = menu->FindItemByPosition(0);
                    menu->Delete(item);
                }
            }
            
            void createMenuItems(wxMenu* menu) {
                for (size_t i = 0; i < m_recentDocuments.size(); ++i) {
                    const IO::Path& path = m_recentDocuments[i];
                    const int windowId = m_baseId + static_cast<int>(i);
                    menu->Append(windowId, path.lastComponent().asString());
                }
            }
        };
    }
}

#endif /* defined(TrenchBroom_RecentDocuments) */
