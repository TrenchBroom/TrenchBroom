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

#include "ActionManager.h"

#include "Preferences.h"
#include "Assets/EntityDefinition.h"
#include "Model/EntityAttributes.h"
#include "Model/Tag.h"
#include "View/CommandIds.h"
#include "View/Menu.h"
#include "View/wxKeyStrings.h"

#include <wx/menu.h>

namespace TrenchBroom {
    namespace View {
        ActionManager& ActionManager::instance() {
            static ActionManager instance;
            return instance;
        }

        wxMenu* ActionManager::findRecentDocumentsMenu(const wxMenuBar* menuBar) {
            const size_t fileMenuIndex = static_cast<size_t>(menuBar->FindMenu("File"));
            const wxMenu* fileMenu = menuBar->GetMenu(fileMenuIndex);
            if (fileMenu == nullptr) {
                return nullptr;
            }
            const wxMenuItem* recentDocumentsItem = fileMenu->FindItem(CommandIds::Menu::FileOpenRecent);
            if (recentDocumentsItem == nullptr) {
                return nullptr;
            }
            return recentDocumentsItem->GetSubMenu();
        }

        const ActionMenuItem* ActionManager::findMenuItem(const int id) const {
            return m_menuBar->findActionMenuItem(id);
        }

        void ActionManager::getShortcutEntries(const std::list<Model::SmartTag>& tags, const Assets::EntityDefinitionList& entityDefinitions, ShortcutEntryList& entries) {
            m_menuBar->getShortcutEntries(entries);

            for (auto& shortcut : m_viewShortcuts) {
                entries.push_back(shortcut.shortcutEntry());
            }

            getTagShortcutEntries(tags, entries);
            getEntityDefinitionShortcutEntries(entityDefinitions, entries);
        }

        void ActionManager::getTagShortcutEntries(const std::list<Model::SmartTag>& tags, ShortcutEntryList& entries) {
            // We want the shortcuts for visibility, enabling, and disabling visually grouped in the preferences.
            for (const auto& tag : tags) {
                entries.emplace_back(std::make_unique<ToggleTagVisibilityKeyboardShortcutEntry>(tag));
            }
            for (const auto& tag : tags) {
                if (tag.canEnable()) {
                    entries.emplace_back(std::make_unique<EnableTagKeyboardShortcutEntry>(tag));
                }
            }
            for (const auto& tag : tags) {
                if (tag.canDisable()) {
                    entries.emplace_back(std::make_unique<DisableTagKeyboardShortcutEntry>(tag));
                }
            }
        }

        class ActionManager::TagKeyboardShortcutEntry : public KeyboardShortcutEntry {
        protected:
            const Model::SmartTag& m_tag;
        public:
            explicit TagKeyboardShortcutEntry(const Model::SmartTag& tag) :
            m_tag(tag) {}
        public:
            static int toggleVisibleActionId(const Model::SmartTag& tag) {
                return CommandIds::Actions::LowestToggleTagCommandId + static_cast<int>(tag.index());
            }

            static int enableActionId(const Model::SmartTag& tag) {
                return CommandIds::Actions::LowestEnableTagCommandId + static_cast<int>(tag.index());
            }

            static int disableActionId(const Model::SmartTag& tag) {
                return CommandIds::Actions::LowestDisableTagCommandId + static_cast<int>(tag.index());
            }

            static IO::Path toggleVisiblePrefPath(const Model::SmartTag& tag) {
                return IO::Path("Filters/Tags") + IO::Path(tag.name()) + IO::Path("Toggle Visible");
            }

            static IO::Path enablePrefPath(const Model::SmartTag& tag) {
                return IO::Path("Tags") + IO::Path(tag.name()) + IO::Path("Enable");
            }

            static IO::Path disablePrefPath(const Model::SmartTag& tag) {
                return IO::Path("Tags") + IO::Path(tag.name()) + IO::Path("Disable");
            }
        private:
            bool doGetModifiable() const override {
                return true;
            }

            wxString doGetJsonString() const override {
                return "";
            }

            const Preference<KeyboardShortcut>& doGetPreference() const override {
                auto& prefs = PreferenceManager::instance();
                return prefs.dynamicPreference(path(), KeyboardShortcut());
            }

            Preference<KeyboardShortcut>& doGetPreference() override {
                auto& prefs = PreferenceManager::instance();
                return prefs.dynamicPreference(path(), KeyboardShortcut());
            }

            wxAcceleratorEntry doGetAcceleratorEntry(ActionView view) const override {
                return shortcut().acceleratorEntry(actionId());
            }

            virtual IO::Path path() const = 0;

            virtual int actionId() const = 0;

            deleteCopyAndMove(TagKeyboardShortcutEntry)
        };

        class ActionManager::ToggleTagVisibilityKeyboardShortcutEntry : public TagKeyboardShortcutEntry {
        public:
            using TagKeyboardShortcutEntry::TagKeyboardShortcutEntry;
        private:
            int doGetActionContext() const override {
                return ActionContext_Any;
            }

            wxString doGetActionDescription() const override {
                wxString result;
                result << "View Filter > Toggle " << m_tag.name() << " visible";
                return result;
            }

            IO::Path path() const override {
                return toggleVisiblePrefPath(m_tag);
            }

            int actionId() const override {
                return toggleVisibleActionId(m_tag);
            }

            deleteCopyAndMove(ToggleTagVisibilityKeyboardShortcutEntry)
        };

        class ActionManager::EnableTagKeyboardShortcutEntry : public TagKeyboardShortcutEntry {
        public:
            using TagKeyboardShortcutEntry::TagKeyboardShortcutEntry;
        private:
            int doGetActionContext() const override {
                return ActionContext_NodeSelection;
            }

            wxString doGetActionDescription() const override {
                wxString result;
                result << "Turn selection into " << m_tag.name();
                return result;
            }

            IO::Path path() const override {
                return enablePrefPath(m_tag);
            }

            int actionId() const override {
                return enableActionId(m_tag);
            }

            deleteCopyAndMove(EnableTagKeyboardShortcutEntry)
        };

        class ActionManager::DisableTagKeyboardShortcutEntry : public TagKeyboardShortcutEntry {
        public:
            using TagKeyboardShortcutEntry::TagKeyboardShortcutEntry;
        private:
            int doGetActionContext() const override {
                return ActionContext_NodeSelection;
            }

            wxString doGetActionDescription() const override {
                wxString result;
                result << "Turn selection into non-" << m_tag.name();
                return result;
            }

            IO::Path path() const override {
                return disablePrefPath(m_tag);
            }

            int actionId() const override {
                return disableActionId(m_tag);
            }

            deleteCopyAndMove(DisableTagKeyboardShortcutEntry)
        };

        void ActionManager::getEntityDefinitionShortcutEntries(Assets::EntityDefinitionList entityDefinitions, ActionManager::ShortcutEntryList& entries) {
            std::sort(std::begin(entityDefinitions), std::end(entityDefinitions), [](const auto* lhs, const auto* rhs) {
                return StringUtils::caseInsensitiveCompare(lhs->name(), rhs->name()) < 0;
            });

            for (size_t i = 0; i < entityDefinitions.size(); ++i) {
                const auto* definition = entityDefinitions[i];
                    entries.emplace_back(std::make_unique<ToggleEntityVisibilityKeyboardShortcutEntry>(definition->name(), i));
            }

            for (size_t i = 0; i < entityDefinitions.size(); ++i) {
                const auto* definition = entityDefinitions[i];
                if (definition->name() != Model::AttributeValues::WorldspawnClassname) {
                    entries.emplace_back(std::make_unique<CreateEntityKeyboardShortcutEntry>(definition->name(), i));
                }
            }
        }

        class ActionManager::EntityKeyboardShortcutEntry : public KeyboardShortcutEntry {
        protected:
            String m_classname;
            size_t m_index;
        public:
            explicit EntityKeyboardShortcutEntry(String classname, const size_t index) :
            m_classname(std::move(classname)),
            m_index(index) {}
        public:
            static int toggleVisibleActionId(const size_t index) {
                return CommandIds::Actions::LowestToggleEntityDefinitionCommandId + static_cast<int>(index);
            }

            static int createActionId(const size_t index) {
                return CommandIds::Actions::LowestCreateEntityCommandId + static_cast<int>(index);
            }

            static IO::Path toggleVisiblePrefPath(const String& classname) {
                return IO::Path("Filters/Entities") + IO::Path(classname) + IO::Path("Toggle Visible");
            }

            static IO::Path createPrefPath(const String& classname) {
                return IO::Path("Entities") + IO::Path(classname) + IO::Path("Create");
            }
        private:
            bool doGetModifiable() const override {
                return true;
            }

            wxString doGetJsonString() const override {
                return "";
            }

            const Preference<KeyboardShortcut>& doGetPreference() const override {
                auto& prefs = PreferenceManager::instance();
                return prefs.dynamicPreference(path(), KeyboardShortcut());
            }

            Preference<KeyboardShortcut>& doGetPreference() override {
                auto& prefs = PreferenceManager::instance();
                return prefs.dynamicPreference(path(), KeyboardShortcut());
            }

            wxAcceleratorEntry doGetAcceleratorEntry(ActionView view) const override {
                return shortcut().acceleratorEntry(actionId());
            }

            virtual IO::Path path() const = 0;

            virtual int actionId() const = 0;

            deleteCopyAndMove(EntityKeyboardShortcutEntry)
        };

        class ActionManager::ToggleEntityVisibilityKeyboardShortcutEntry : public EntityKeyboardShortcutEntry {
        public:
            using EntityKeyboardShortcutEntry::EntityKeyboardShortcutEntry;
        private:
            int doGetActionContext() const override {
                return ActionContext_Any;
            }

            wxString doGetActionDescription() const override {
                wxString result;
                result << "View Filter > Toggle " << m_classname << " visible";
                return result;
            }

            IO::Path path() const override {
                return toggleVisiblePrefPath(m_classname);
            }

            int actionId() const override {
                return toggleVisibleActionId(m_index);
            }

            deleteCopyAndMove(ToggleEntityVisibilityKeyboardShortcutEntry)
        };

        class ActionManager::CreateEntityKeyboardShortcutEntry : public EntityKeyboardShortcutEntry {
        public:
            using EntityKeyboardShortcutEntry::EntityKeyboardShortcutEntry;
        private:
            int doGetActionContext() const override {
                return ActionContext_Any;
            }

            wxString doGetActionDescription() const override {
                wxString result;
                result << "Create " << m_classname;
                return result;
            }

            IO::Path path() const override {
                return createPrefPath(m_classname);
            }

            int actionId() const override {
                return createActionId(m_index);
            }

            deleteCopyAndMove(CreateEntityKeyboardShortcutEntry)
        };

        String ActionManager::getJSTable() {
            StringStream str;
            getKeysJSTable(str);
            getMenuJSTable(str);
            getActionJSTable(str);
            return str.str();
        }

        void ActionManager::getKeysJSTable(StringStream& str) {
            str << "var keys = { mac: {}, windows: {}, linux: {} };" << std::endl;
            wxKeyStringsWindows().appendJS("windows", str);
            wxKeyStringsMac().appendJS("mac", str);
            wxKeyStringsLinux().appendJS("linux", str);
            str << std::endl;
        }

        void ActionManager::getMenuJSTable(StringStream& str) {
            str << "var menu = {};" << std::endl;

            ShortcutEntryList entries;
            m_menuBar->getShortcutEntries(entries);

            for (const auto& entry : entries) {
                String preferencePath = entry->preferencePath().asString('/');
                if (StringUtils::caseSensitiveSuffix(preferencePath, "...")) {
                    // Remove "..." suffix because pandoc will transform this into unicode ellipses.
                    preferencePath = preferencePath.substr(0, preferencePath.length() - 3);
                }
                str << "menu[\"" << preferencePath << "\"] = " << entry->asJsonString() << ";"
                    << std::endl;;
            }
        }

        void printActionPreference(StringStream& str, const Preference<KeyboardShortcut>& pref);
        void printActionPreference(StringStream& str, const Preference<KeyboardShortcut>& pref) {
            str << "actions[\"" << pref.path().asString('/') << "\"] = " << pref.defaultValue().asJsonString() << ";" << std::endl;
        }

        void ActionManager::getActionJSTable(StringStream& str) {
            str << "var actions = {};" << std::endl;

            for (ViewShortcut& shortcut : m_viewShortcuts) {
                const auto entry = shortcut.shortcutEntry();
                str << "actions[\"" << entry->preferencePath().asString('/') << "\"] = " << entry->asJsonString() << ";" << std::endl;
            }

            printActionPreference(str, Preferences::CameraFlyForward);
            printActionPreference(str, Preferences::CameraFlyBackward);
            printActionPreference(str, Preferences::CameraFlyLeft);
            printActionPreference(str, Preferences::CameraFlyRight);
            printActionPreference(str, Preferences::CameraFlyUp);
            printActionPreference(str, Preferences::CameraFlyDown);
        }

        wxMenuBar* ActionManager::createMenuBar(const bool withShortcuts) const {
            return m_menuBar->createMenuBar(withShortcuts);
        }

        bool ActionManager::isMenuShortcutPreference(const IO::Path& path) const {
            return !path.isEmpty() && path.firstComponent().asString() == "Menu";
        }

        wxAcceleratorTable ActionManager::createViewAcceleratorTable(const ActionContext context, const ActionView view, const std::list<Model::SmartTag>& tags, const Assets::EntityDefinitionList& entityDefinitions) const {
            AcceleratorEntryList tableEntries;
            addViewActions(context, view, tableEntries);
#ifdef __WXGTK20__
            // This causes some shortcuts such as "2" to not work on Windows.
            // But it's necessary to enable one key menu shortcuts to work on GTK.
            addMenuActions(context, view, tableEntries);
#endif
            addTagActions(tags, tableEntries);
            addEntityDefinitionActions(entityDefinitions, tableEntries);
            return wxAcceleratorTable(static_cast<int>(tableEntries.size()), &tableEntries.front());
        }

        void ActionManager::addViewActions(ActionContext context, ActionView view, AcceleratorEntryList& accelerators) const {
            for (auto& shortcut : m_viewShortcuts) {
                if (shortcut.appliesToContext(context)) {
                    accelerators.push_back(shortcut.acceleratorEntry(view));
                }
            }
        }

        void ActionManager::addMenuActions(ActionContext context, ActionView view, AcceleratorEntryList& accelerators) const {
            ShortcutEntryList entries;
            m_menuBar->getShortcutEntries(entries);

            for (const auto& entry : entries) {
                if (entry->appliesToContext(context)) {
                    accelerators.push_back(entry->acceleratorEntry(view));
                }
            }
        }

        void ActionManager::addTagActions(const std::list<Model::SmartTag>& tags, ActionManager::AcceleratorEntryList& accelerators) const {
            for (const auto& tag : tags) {
                Preference<KeyboardShortcut> toggleVisiblePref(TagKeyboardShortcutEntry::toggleVisiblePrefPath(tag), KeyboardShortcut());
                const auto& toggleVisibleShortcut = pref(toggleVisiblePref);
                accelerators.push_back(toggleVisibleShortcut.acceleratorEntry(TagKeyboardShortcutEntry::toggleVisibleActionId(tag)));

                if (tag.canEnable()) {
                    Preference<KeyboardShortcut> enablePref(TagKeyboardShortcutEntry::enablePrefPath(tag), KeyboardShortcut());
                    const auto& enableShortcut = pref(enablePref);
                    accelerators.push_back(enableShortcut.acceleratorEntry(TagKeyboardShortcutEntry::enableActionId(tag)));
                }

                if (tag.canDisable()) {
                    Preference<KeyboardShortcut> disablePref(TagKeyboardShortcutEntry::disablePrefPath(tag), KeyboardShortcut());
                    const auto& disableShortcut = pref(disablePref);
                    accelerators.push_back(disableShortcut.acceleratorEntry(TagKeyboardShortcutEntry::disableActionId(tag)));
                }
            }
        }

        void ActionManager::addEntityDefinitionActions(const Assets::EntityDefinitionList& entityDefinitions, ActionManager::AcceleratorEntryList& accelerators) const {
            for (size_t i = 0; i < entityDefinitions.size(); ++i) {
                const auto* definition = entityDefinitions[i];
                Preference<KeyboardShortcut> preference(EntityKeyboardShortcutEntry::toggleVisiblePrefPath(definition->name()), KeyboardShortcut());
                const auto& shortcut = pref(preference);
                accelerators.push_back(shortcut.acceleratorEntry(EntityKeyboardShortcutEntry::toggleVisibleActionId(i)));
            }

            for (size_t i = 0; i < entityDefinitions.size(); ++i) {
                const auto* definition = entityDefinitions[i];
                if (definition->name() != Model::AttributeValues::WorldspawnClassname) {
                    Preference<KeyboardShortcut> preference(EntityKeyboardShortcutEntry::createPrefPath(definition->name()), KeyboardShortcut());
                    const auto& shortcut = pref(preference);
                    accelerators.push_back(shortcut.acceleratorEntry(EntityKeyboardShortcutEntry::createActionId(i)));
                }
            }
        }

        void ActionManager::resetShortcutsToDefaults() {
            m_menuBar->resetShortcuts();

            for (ViewShortcut& shortcut : m_viewShortcuts) {
                shortcut.resetShortcut();
            }
        }

        ActionManager::ActionManager() :
        m_menuBar(nullptr) {
            createMenuBar();
            createViewShortcuts();
        }

        void ActionManager::createMenuBar() {
            m_menuBar = std::make_unique<MenuBar>();

            Menu* fileMenu = m_menuBar->addMenu("File");
            fileMenu->addUnmodifiableActionItem(wxID_NEW, "New", KeyboardShortcut('N', WXK_CONTROL));
            fileMenu->addSeparator();
            fileMenu->addUnmodifiableActionItem(wxID_OPEN, "Open...", KeyboardShortcut('O', WXK_CONTROL));
            fileMenu->addMenu(CommandIds::Menu::FileOpenRecent, "Open Recent");
            fileMenu->addSeparator();
            fileMenu->addUnmodifiableActionItem(wxID_SAVE, "Save", KeyboardShortcut('S', WXK_CONTROL));
            fileMenu->addUnmodifiableActionItem(wxID_SAVEAS, "Save as...", KeyboardShortcut('S', WXK_SHIFT, WXK_CONTROL));

            Menu* exportMenu = fileMenu->addMenu("Export");
            exportMenu->addModifiableActionItem(CommandIds::Menu::FileExportObj, "Wavefront OBJ...");

            fileMenu->addSeparator();
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileLoadPointFile, "Load Point File...");
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileReloadPointFile, "Reload Point File");
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileUnloadPointFile, "Unload Point File");
            fileMenu->addSeparator();
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileLoadPortalFile, "Load Portal File...");
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileReloadPortalFile, "Reload Portal File");
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileUnloadPortalFile, "Unload Portal File");
            fileMenu->addSeparator();
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileReloadTextureCollections, "Reload Texture Collections", KeyboardShortcut(WXK_F5));
            fileMenu->addModifiableActionItem(CommandIds::Menu::FileReloadEntityDefinitions, "Reload Entity Definitions", KeyboardShortcut(WXK_F6));
            fileMenu->addSeparator();
            fileMenu->addUnmodifiableActionItem(wxID_CLOSE, "Close", KeyboardShortcut('W', WXK_CONTROL));

            Menu* editMenu = m_menuBar->addMenu("Edit");
            editMenu->addUnmodifiableActionItem(wxID_UNDO, "Undo", KeyboardShortcut('Z', WXK_CONTROL));
            editMenu->addUnmodifiableActionItem(wxID_REDO, "Redo", KeyboardShortcut('Z', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();
            editMenu->addModifiableActionItem(CommandIds::Menu::EditRepeat, "Repeat", KeyboardShortcut('R', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditClearRepeat, "Clear Repeatable Commands", KeyboardShortcut('R', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();
            editMenu->addUnmodifiableActionItem(wxID_CUT, "Cut", KeyboardShortcut('X', WXK_CONTROL));
            editMenu->addUnmodifiableActionItem(wxID_COPY, "Copy", KeyboardShortcut('C', WXK_CONTROL));
            editMenu->addUnmodifiableActionItem(wxID_PASTE, "Paste", KeyboardShortcut('V', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditPasteAtOriginalPosition, "Paste at Original Position", KeyboardShortcut('V', WXK_CONTROL, WXK_ALT));
            editMenu->addModifiableActionItem(wxID_DUPLICATE, "Duplicate", KeyboardShortcut('D', WXK_CONTROL));
#ifdef __APPLE__
            editMenu->addModifiableActionItem(wxID_DELETE, "Delete", KeyboardShortcut(WXK_BACK));
#else
            editMenu->addModifiableActionItem(wxID_DELETE, "Delete", KeyboardShortcut(WXK_DELETE));
#endif

            editMenu->addSeparator();
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectAll, "Select All", KeyboardShortcut('A', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectSiblings, "Select Siblings", KeyboardShortcut('B', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectTouching, "Select Touching", KeyboardShortcut('T', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectInside, "Select Inside", KeyboardShortcut('E', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectTall, "Select Tall", KeyboardShortcut('E', WXK_CONTROL, WXK_SHIFT));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectByFilePosition, "Select by Line Number");
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSelectNone, "Select None", KeyboardShortcut('A', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();

            editMenu->addModifiableActionItem(CommandIds::Menu::EditGroupSelection, "Group", KeyboardShortcut('G', WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditUngroupSelection, "Ungroup", KeyboardShortcut('G', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();

            Menu* toolMenu = editMenu->addMenu("Tools");
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleCreateComplexBrushTool, "Brush Tool", KeyboardShortcut('B'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleClipTool, "Clip Tool", KeyboardShortcut('C'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleRotateObjectsTool, "Rotate Tool", KeyboardShortcut('R'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleScaleObjectsTool, "Scale Tool", KeyboardShortcut('T'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleShearObjectsTool, "Shear Tool", KeyboardShortcut('G'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleVertexTool, "Vertex Tool", KeyboardShortcut('V'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleEdgeTool, "Edge Tool", KeyboardShortcut('E'));
            toolMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleFaceTool, "Face Tool", KeyboardShortcut('F'));

            Menu* csgMenu = editMenu->addMenu("CSG");
            csgMenu->addModifiableActionItem(CommandIds::Menu::EditCsgConvexMerge, "Convex Merge", KeyboardShortcut('J', WXK_CONTROL));
            csgMenu->addModifiableActionItem(CommandIds::Menu::EditCsgSubtract, "Subtract", KeyboardShortcut('K', WXK_CONTROL));
            csgMenu->addModifiableActionItem(CommandIds::Menu::EditCsgHollow, "Hollow", KeyboardShortcut('K', WXK_CONTROL, WXK_ALT));
            csgMenu->addModifiableActionItem(CommandIds::Menu::EditCsgIntersect, "Intersect", KeyboardShortcut('L', WXK_CONTROL));

            editMenu->addSeparator();
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSnapVerticesToInteger, "Snap Vertices to Integer", KeyboardShortcut('V', WXK_SHIFT, WXK_CONTROL));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditSnapVerticesToGrid, "Snap Vertices to Grid", KeyboardShortcut('V', WXK_SHIFT, WXK_CONTROL, WXK_ALT));
            editMenu->addSeparator();
            editMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleTextureLock, "Texture Lock");
            editMenu->addModifiableCheckItem(CommandIds::Menu::EditToggleUVLock, "UV Lock", KeyboardShortcut('U'));
            editMenu->addModifiableActionItem(CommandIds::Menu::EditReplaceTexture, "Replace Texture...");

            Menu* viewMenu = m_menuBar->addMenu("View");
            Menu* gridMenu = viewMenu->addMenu("Grid");
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleShowGrid, "Show Grid", KeyboardShortcut('0'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleSnapToGrid, "Snap to Grid", KeyboardShortcut('0', WXK_ALT));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewIncGridSize, "Increase Grid Size", KeyboardShortcut('+'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewDecGridSize, "Decrease Grid Size", KeyboardShortcut('-'));
            gridMenu->addSeparator();
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize0Point125, "Set Grid Size 0.125");
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize0Point25, "Set Grid Size 0.25");
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize0Point5, "Set Grid Size 0.5");
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize1, "Set Grid Size 1", KeyboardShortcut('1'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize2, "Set Grid Size 2", KeyboardShortcut('2'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize4, "Set Grid Size 4", KeyboardShortcut('3'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize8, "Set Grid Size 8", KeyboardShortcut('4'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize16, "Set Grid Size 16", KeyboardShortcut('5'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize32, "Set Grid Size 32", KeyboardShortcut('6'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize64, "Set Grid Size 64", KeyboardShortcut('7'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize128, "Set Grid Size 128", KeyboardShortcut('8'));
            gridMenu->addModifiableCheckItem(CommandIds::Menu::ViewSetGridSize256, "Set Grid Size 256", KeyboardShortcut('9'));

            Menu* cameraMenu = viewMenu->addMenu("Camera");
            cameraMenu->addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToNextPoint, "Move to Next Point", KeyboardShortcut('.'));
            cameraMenu->addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPreviousPoint, "Move to Previous Point", KeyboardShortcut(','));
            cameraMenu->addModifiableActionItem(CommandIds::Menu::ViewFocusCameraOnSelection, "Focus on Selection", KeyboardShortcut('U', WXK_CONTROL));
            cameraMenu->addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPosition, "Move Camera to...");
            cameraMenu->addSeparator();

            viewMenu->addSeparator();
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewIsolateSelection, "Isolate", KeyboardShortcut('I', WXK_CONTROL));
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewHideSelection, "Hide", KeyboardShortcut('I', WXK_CONTROL, WXK_ALT));
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewUnhideAll, "Show All", KeyboardShortcut('I', WXK_CONTROL, WXK_SHIFT));

            viewMenu->addSeparator();
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewSwitchToMapInspector, "Switch to Map Inspector", KeyboardShortcut('1', WXK_CONTROL));
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewSwitchToEntityInspector, "Switch to Entity Inspector", KeyboardShortcut('2', WXK_CONTROL));
            viewMenu->addModifiableActionItem(CommandIds::Menu::ViewSwitchToFaceInspector, "Switch to Face Inspector", KeyboardShortcut('3', WXK_CONTROL));
            viewMenu->addSeparator();
            viewMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleInfoPanel, "Toggle Info Panel", KeyboardShortcut('4', WXK_CONTROL));
            viewMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleInspector, "Toggle Inspector", KeyboardShortcut('5', WXK_CONTROL));
            viewMenu->addSeparator();
            viewMenu->addModifiableCheckItem(CommandIds::Menu::ViewToggleMaximizeCurrentView, "Maximize Current View", KeyboardShortcut(WXK_SPACE, WXK_CONTROL));

            Menu* runMenu = m_menuBar->addMenu("Run");
            runMenu->addModifiableActionItem(CommandIds::Menu::RunCompile, "Compile...");
            runMenu->addModifiableActionItem(CommandIds::Menu::RunLaunch, "Launch...");

#ifndef NDEBUG
            Menu* debugMenu = m_menuBar->addMenu("Debug");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugPrintVertices, "Print Vertices");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugCreateBrush, "Create Brush...");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugCreateCube, "Create Cube...");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugClipWithFace, "Clip Brush...");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugCopyJSShortcuts, "Copy Javascript Shortcut Map");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugCrash, "Crash...");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugThrowExceptionDuringCommand, "Throw Exception During Command");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugCrashReportDialog, "Show Crash Report Dialog");
            debugMenu->addUnmodifiableActionItem(CommandIds::Menu::DebugSetWindowSize, "Set Window Size...");
#endif

            Menu* helpMenu = m_menuBar->addMenu("Help");
#ifdef _WIN32
            helpMenu->addUnmodifiableActionItem(wxID_HELP, "TrenchBroom Manual", KeyboardShortcut(WXK_F1));
#else
            helpMenu->addUnmodifiableActionItem(wxID_HELP, "TrenchBroom Manual");
#endif

#ifdef __APPLE__
            // these won't show up in the app menu if we don't add them here
            fileMenu->addUnmodifiableActionItem(wxID_ABOUT, "About TrenchBroom");
            fileMenu->addUnmodifiableActionItem(wxID_PREFERENCES, "Preferences...", KeyboardShortcut(',', WXK_CONTROL));
            fileMenu->addUnmodifiableActionItem(wxID_EXIT, "Exit");
#else
            viewMenu->addSeparator();
            viewMenu->addUnmodifiableActionItem(wxID_PREFERENCES, "Preferences...");

            helpMenu->addSeparator();
            helpMenu->addUnmodifiableActionItem(wxID_ABOUT, "About TrenchBroom");
#endif
        }

        void ActionManager::createViewShortcuts() {
            createViewShortcut(KeyboardShortcut(WXK_RETURN), ActionContext_CreateComplexBrushTool,
                               Action(View::CommandIds::Actions::Nothing, "", false),
                               Action(View::CommandIds::Actions::PerformCreateBrush, "Create brush", true));

            createViewShortcut(KeyboardShortcut(WXK_RETURN, WXK_CONTROL), ActionContext_ClipTool,
                               Action(View::CommandIds::Actions::ToggleClipSide, "Toggle clip side", true));
            createViewShortcut(KeyboardShortcut(WXK_RETURN), ActionContext_ClipTool,
                               Action(View::CommandIds::Actions::PerformClip, "Perform clip", true));

            createViewShortcut(KeyboardShortcut(WXK_UP), ActionContext_AnyVertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesUp, "Move vertices up", true),
                               Action(View::CommandIds::Actions::MoveVerticesForward, "Move vertices forward", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN), ActionContext_AnyVertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesDown, "Move vertices down", true),
                               Action(View::CommandIds::Actions::MoveVerticesBackward, "Move vertices backward", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT), ActionContext_AnyVertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesLeft, "Move vertices left", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT), ActionContext_AnyVertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesRight, "Move vertices right", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP), ActionContext_AnyVertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesBackward, "Move vertices backward", true),
                               Action(View::CommandIds::Actions::MoveVerticesUp, "Move vertices up", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN), ActionContext_AnyVertexTool,
                               Action(View::CommandIds::Actions::MoveVerticesForward, "Move vertices forward", true),
                               Action(View::CommandIds::Actions::MoveVerticesDown, "Move vertices down", true));

            createViewShortcut(KeyboardShortcut(WXK_UP), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterUp, "Move rotation center up", true),
                               Action(View::CommandIds::Actions::MoveRotationCenterForward, "Move rotation center forward", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterDown, "Move rotation center down", true),
                               Action(View::CommandIds::Actions::MoveRotationCenterBackward, "Move rotation center backward", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterLeft, "Move rotation center left", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterRight, "Move rotation center right", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterBackward, "Move rotation center backward", true),
                               Action(View::CommandIds::Actions::MoveRotationCenterUp, "Move rotation center up", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN), ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::MoveRotationCenterForward, "Move rotation center forward", true),
                               Action(View::CommandIds::Actions::MoveRotationCenterDown, "Move rotation center down", true));

            createViewShortcut(KeyboardShortcut(WXK_UP), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsUp, "Move objects up", true),
                               Action(View::CommandIds::Actions::MoveObjectsForward, "Move objects forward", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsDown, "Move objects down", true),
                               Action(View::CommandIds::Actions::MoveObjectsBackward, "Move objects backward", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsLeft, "Move objects left", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsRight, "Move objects right", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsBackward, "Move objects backward", true),
                               Action(View::CommandIds::Actions::MoveObjectsUp, "Move objects up", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MoveObjectsForward, "Move objects forward", true),
                               Action(View::CommandIds::Actions::MoveObjectsDown, "Move objects down", true));

            createViewShortcut(KeyboardShortcut(WXK_UP, WXK_ALT), ActionContext_NodeSelection | ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::RollObjectsCW, "Roll objects clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN, WXK_ALT), ActionContext_NodeSelection | ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::RollObjectsCCW, "Roll objects counter-clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT, WXK_ALT), ActionContext_NodeSelection | ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::YawObjectsCW, "Yaw objects clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT, WXK_ALT), ActionContext_NodeSelection | ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::YawObjectsCCW, "Yaw objects counter-clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP, WXK_ALT), ActionContext_NodeSelection | ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::PitchObjectsCW, "Pitch objects clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN, WXK_ALT), ActionContext_NodeSelection | ActionContext_RotateTool,
                               Action(View::CommandIds::Actions::PitchObjectsCCW, "Pitch objects counter-clockwise", true));

            createViewShortcut(KeyboardShortcut('F', WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::FlipObjectsHorizontally, "Flip objects horizontally", true));
            createViewShortcut(KeyboardShortcut('F', WXK_CONTROL, WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::FlipObjectsVertically, "Flip objects vertically", true));

            createViewShortcut(KeyboardShortcut(WXK_UP, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsUp, "Duplicate and move objects up", true),
                               Action(View::CommandIds::Actions::DuplicateObjectsForward, "Duplicate and move objects forward", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsDown, "Duplicate and move objects down", true),
                               Action(View::CommandIds::Actions::DuplicateObjectsBackward, "Duplicate and move objects backward", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsLeft, "Duplicate and move objects left", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsRight, "Duplicate and move objects right", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsBackward, "Duplicate and move objects backward", true),
                               Action(View::CommandIds::Actions::DuplicateObjectsUp, "Duplicate and move objects up", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN, WXK_CONTROL), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::DuplicateObjectsForward, "Duplicate and move objects forward", true),
                               Action(View::CommandIds::Actions::DuplicateObjectsDown, "Duplicate and move objects down", true));

            createViewShortcut(KeyboardShortcut(WXK_UP), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesUp, "Move textures up", true));
            createViewShortcut(KeyboardShortcut(WXK_UP, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesUp, "Move textures up (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_UP, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesUp, "Move textures up (coarse)", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesDown, "Move textures down", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesDown, "Move textures down (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_DOWN, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesDown, "Move textures down (coarse)", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesLeft, "Move textures left", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesLeft, "Move textures left (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_LEFT, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesLeft, "Move textures left (coarse)", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesRight, "Move textures right", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesRight, "Move textures right (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_RIGHT, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::MoveTexturesRight, "Move textures right (coarse)", true));

            createViewShortcut(KeyboardShortcut(WXK_PAGEUP), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCW, "Rotate textures clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCW, "Rotate textures clockwise (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEUP, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCW, "Rotate textures clockwise (coarse)", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCCW, "Rotate textures counter-clockwise", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN, WXK_CONTROL), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCCW, "Rotate textures counter-clockwise (fine)", true));
            createViewShortcut(KeyboardShortcut(WXK_PAGEDOWN, WXK_SHIFT), ActionContext_FaceSelection, Action(),
                               Action(View::CommandIds::Actions::RotateTexturesCCW, "Rotate textures counter-clockwise (coarse)", true));

            createViewShortcut(KeyboardShortcut(WXK_SPACE), ActionContext_Any,
                               Action(View::CommandIds::Actions::CycleMapViews, "Cycle map view", true));

            createViewShortcut(KeyboardShortcut(WXK_ESCAPE, WXK_SHIFT), ActionContext_Any,
                               Action("No effect"),
                               Action(View::CommandIds::Actions::ResetZoom, "Reset camera zoom", true));

            createViewShortcut(KeyboardShortcut(WXK_ESCAPE), ActionContext_Any,
                               Action(View::CommandIds::Actions::Cancel, "Cancel", true));
            createViewShortcut(KeyboardShortcut(WXK_ESCAPE, WXK_CONTROL), ActionContext_Any,
                               Action(View::CommandIds::Actions::DeactivateTool, "Deactivate current tool", true));

            createViewShortcut(KeyboardShortcut('S', WXK_ALT), ActionContext_NodeSelection,
                               Action(View::CommandIds::Actions::MakeStructural, "Make structural", true));

            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::ToggleShowEntityClassnames, "View Filter > Toggle show entity classnames", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::ToggleShowGroupBounds, "View Filter > Toggle show group bounds", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::ToggleShowBrushEntityBounds, "View Filter > Toggle show brush entity bounds", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::ToggleShowPointEntityBounds, "View Filter > Toggle show point entity bounds", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::ToggleShowPointEntities, "View Filter > Toggle show point entities", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::ToggleShowPointEntityModels, "View Filter > Toggle show point entity models", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::ToggleShowBrushes, "View Filter > Toggle show brushes", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeShowTextures, "View Filter > Show textures", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeHideTextures, "View Filter > Hide textures", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeHideFaces, "View Filter > Hide faces", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeShadeFaces, "View Filter > Shade faces", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeUseFog, "View Filter > Use fog", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeShowEdges, "View Filter > Show edges", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeShowAllEntityLinks, "View Filter > Show all entity links", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeShowTransitiveEntityLinks, "View Filter > Show transitively selected entity links", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeShowDirectEntityLinks, "View Filter > Show directly selected entity links", true));
            createViewShortcut(KeyboardShortcut(), ActionContext_Any,
                               Action(CommandIds::Actions::RenderModeHideEntityLinks, "View Filter > Hide entity links", true));
        }

        void ActionManager::createViewShortcut(const KeyboardShortcut& shortcut, const int context, const Action& action2D, const Action& action3D) {
            m_viewShortcuts.push_back(ViewShortcut(shortcut, context, action2D, action3D));
        }

        void ActionManager::createViewShortcut(const KeyboardShortcut& shortcut, const int context, const Action& action) {
            m_viewShortcuts.push_back(ViewShortcut(shortcut, context, action));
        }
    }
}
