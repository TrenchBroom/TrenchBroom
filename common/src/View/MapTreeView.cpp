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

#include "MapTreeView.h"

#include "Notifier.h"
#include "Model/ModelTypes.h"
#include "Model/ModelUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/ModelUtils.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/ViewUtils.h"

#include <wx/dataview.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/variant.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        struct AddObjectToItemArray {
            wxDataViewItemArray& items;

            AddObjectToItemArray(wxDataViewItemArray& i_items) :
            items(i_items) {}

            void operator()(Model::Object* object) const {
                items.push_back(wxDataViewItem(reinterpret_cast<void*>(object)));
            }
        };

        class MapTreeViewDataModel : public wxDataViewModel {
        private:
            MapDocumentWPtr m_document;
        public:
            MapTreeViewDataModel(MapDocumentWPtr document) :
            m_document(document) {
                bindObservers();
            }

            ~MapTreeViewDataModel() {
                unbindObservers();
            }

            unsigned int GetColumnCount() const {
                return 1;
            }

            wxString GetColumnType(unsigned int col) const {
                assert(col == 0);
                return "string";
            }

            bool IsContainer(const wxDataViewItem& item) const {
                if (!item.IsOk())
                    return true;
                
                const void* data = item.GetID();
                assert(data != NULL);
                const Model::Object* object = reinterpret_cast<const Model::Object*>(data);
                
                if (object->type() == Model::Object::Type_Entity) {
#if defined __linux__
                    return true;
#else
                    const Model::Entity* entity = static_cast<const Model::Entity*>(object);
                    const Model::BrushList& brushes = entity->brushes();
                    return !brushes.empty();
#endif
                }
                return false;
            }

            unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const {
                if (expired(m_document))
                    return 0;

                MapDocumentSPtr document = lock(m_document);
                if (!item.IsOk()) {
                    const Model::Map* map = document->map();
                    if (map != NULL) {
                        const Model::EntityList& entities = map->entities();

                        AddObjectToItemArray addObjects(children);
                        Model::each(entities.begin(), entities.end(), addObjects, Model::MatchAll());
                    }
                } else {
                    const void* data = item.GetID();
                    assert(data != NULL);
                    
                    const Model::Object* object = reinterpret_cast<const Model::Object*>(data);
                    if (object->type() == Model::Object::Type_Entity) {
                        const Model::Entity* entity = static_cast<const Model::Entity*>(object);
                        const Model::BrushList& brushes = entity->brushes();

                        AddObjectToItemArray addObjects(children);
                        Model::each(brushes.begin(), brushes.end(), addObjects, Model::MatchAll());
                    }
                }

                return static_cast<unsigned int>(children.size());
            }

            wxDataViewItem GetParent(const wxDataViewItem& item) const {
                if (!item.IsOk())
                    return wxDataViewItem(NULL);

                const void* data = item.GetID();
                assert(data != NULL);
                
                const Model::Object* object = reinterpret_cast<const Model::Object*>(data);
                if (object->type() == Model::Object::Type_Entity)
                    return wxDataViewItem(NULL);

                if (object->type() == Model::Object::Type_Brush) {
                    const Model::Brush* brush = reinterpret_cast<const Model::Brush*>(object);
                    return wxDataViewItem(reinterpret_cast<void*>(brush->parent()));
                }

                // should not happen
                assert(false);
                return wxDataViewItem(NULL);
            }

            void GetValue(wxVariant& result, const wxDataViewItem& item, unsigned int col) const {
                assert(col == 0);
                if (!item.IsOk()) {
                    result = wxVariant("Map");
                } else {
                    const void* data = item.GetID();
                    assert(data != NULL);
                    const Model::Object* object = reinterpret_cast<const Model::Object*>(data);
                    if (object->type() == Model::Object::Type_Entity) {
                        const Model::Entity* entity = static_cast<const Model::Entity*>(object);
                        result = wxVariant(wxString(entity->classname("missing classname")));
                    } else if (object->type() == Model::Object::Type_Brush) {
                        const Model::Brush* brush = static_cast<const Model::Brush*>(object);
                        wxString label;
                        label << brush->faces().size() << "-sided brush";
                        result = wxVariant(label);
                    }
                }
            }

            bool SetValue(const wxVariant& value, const wxDataViewItem& item, unsigned int col) {
                assert(col == 0);
                return false;
            }

            void bindObservers() {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasClearedNotifier.addObserver(this, &MapTreeViewDataModel::documentWasCleared);
                document->documentWasNewedNotifier.addObserver(this, &MapTreeViewDataModel::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.addObserver(this, &MapTreeViewDataModel::documentWasNewedOrLoaded);
                document->objectsWereAddedNotifier.addObserver(this, &MapTreeViewDataModel::objectsWereAdded);
                document->objectsWereRemovedNotifier.addObserver(this, &MapTreeViewDataModel::objectsWereRemoved);
                document->objectsDidChangeNotifier.addObserver(this, &MapTreeViewDataModel::objectsDidChange);
            }

            void unbindObservers() {
                if (!expired(m_document)) {
                    MapDocumentSPtr document = lock(m_document);
                    document->documentWasClearedNotifier.removeObserver(this, &MapTreeViewDataModel::documentWasCleared);
                    document->documentWasNewedNotifier.removeObserver(this, &MapTreeViewDataModel::documentWasNewedOrLoaded);
                    document->documentWasLoadedNotifier.removeObserver(this, &MapTreeViewDataModel::documentWasNewedOrLoaded);
                    document->objectsWereAddedNotifier.removeObserver(this, &MapTreeViewDataModel::objectsWereAdded);
                    document->objectsWereRemovedNotifier.removeObserver(this, &MapTreeViewDataModel::objectsWereRemoved);
                    document->objectsDidChangeNotifier.removeObserver(this, &MapTreeViewDataModel::objectsDidChange);
                }
            }

            void documentWasCleared() {
                Cleared();
            }
            
            void documentWasNewedOrLoaded() {
                addAllObjects();
            }
            
            void addAllObjects() {
                if (!expired(m_document)) {
                    MapDocumentSPtr document = lock(m_document);
                    const Model::Map* map = document->map();
                    if (map != NULL) {
                        const Model::EntityList& entities = map->entities();
                        
                        wxDataViewItemArray children;
                        AddObjectToItemArray addObjects(children);
                        Model::each(entities.begin(), entities.end(), addObjects, Model::MatchAll());
                        ItemsAdded(wxDataViewItem(NULL), children);
                        
                        Model::EntityList::const_iterator it, end;
                        for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                            Model::Entity* entity = *it;
                            const Model::BrushList& brushes = entity->brushes();
                            if (!brushes.empty()) {
                                children.Clear();
                                Model::each(brushes.begin(), brushes.end(), addObjects, Model::MatchAll());
                                ItemsAdded(wxDataViewItem(reinterpret_cast<void*>(entity)), children);
                            }
                        }
                    }
                }
            }

            void objectsWereAdded(const Model::ObjectList& objects) {
                const Model::ObjectChildrenMap map = Model::makeObjectChildrenMap(objects);
                
                Model::ObjectChildrenMap::const_iterator it, end;
                for (it = map.begin(), end = map.end(); it != end; ++it) {
                    Model::Object* parent = it->first;
                    const Model::ObjectList& children = it->second;

                    wxDataViewItem parentItem(parent);
                    wxDataViewItemArray childItems;
                    AddObjectToItemArray addObjects(childItems);
                    Model::each(children.begin(), children.end(), addObjects, Model::MatchAll());
                    ItemsAdded(parentItem, childItems);
                }
            }

            void objectsWereRemoved(const Model::ObjectParentList& objects) {
                const Model::ObjectChildrenMap map = Model::makeObjectChildrenMap(objects);
                
                Model::ObjectChildrenMap::const_iterator it, end;
                for (it = map.begin(), end = map.end(); it != end; ++it) {
                    Model::Object* parent = it->first;
                    const Model::ObjectList& children = it->second;

                    wxDataViewItem parentItem(parent);
                    wxDataViewItemArray childItems;
                    AddObjectToItemArray addObjects(childItems);
                    Model::each(children.begin(), children.end(), addObjects, Model::MatchAll());
                    ItemsDeleted(parentItem, childItems);
                }
            }

            void objectsDidChange(const Model::ObjectList& objects) {
                wxDataViewItemArray items;
                AddObjectToItemArray addObjects(items);
                Model::each(objects.begin(), objects.end(), addObjects, Model::MatchAll());
                ItemsChanged(items);
            }
        };

        MapTreeView::MapTreeView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_tree(NULL),
        m_ignoreTreeSelection(false),
        m_ignoreDocumentSelection(false) {
            m_tree = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER | wxDV_MULTIPLE | wxBORDER_NONE);
            m_tree->AssociateModel(new MapTreeViewDataModel(m_document));
            m_tree->AppendTextColumn("Caption", 0)->SetWidth(200);
            m_tree->Expand(wxDataViewItem(NULL));

            m_tree->Bind(wxEVT_SIZE, &MapTreeView::OnTreeViewSize, this);
            m_tree->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &MapTreeView::OnTreeViewSelectionChanged, this);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_tree, 1, wxEXPAND);
            SetSizerAndFit(sizer);

            bindObservers();
        }

        MapTreeView::~MapTreeView() {
            unbindObservers();
        }

        void MapTreeView::OnTreeViewSize(wxSizeEvent& event) {
            const int scrollbarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
            const int newWidth = std::max(1, m_tree->GetClientSize().x - scrollbarWidth);
            m_tree->GetColumn(0)->SetWidth(newWidth);
            event.Skip();
        }

        void MapTreeView::OnTreeViewSelectionChanged(wxDataViewEvent& event) {
            if (m_ignoreTreeSelection)
                return;

            ControllerSPtr controller = lock(m_controller);

            SetBool disableDocumentSelection(m_ignoreDocumentSelection);

            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);

            Model::ObjectList selectObjects;
            selectObjects.reserve(selections.size());

            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                Model::Object* object = reinterpret_cast<Model::Object*>(item.GetID());
                selectObjects.push_back(object);
            }

            controller->deselectAllAndSelectObjects(selectObjects);
            // TODO: make the selected objects visible in the 3D view
        }

        void MapTreeView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &MapTreeView::selectionDidChange);
        }

        void MapTreeView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &MapTreeView::selectionDidChange);
            }
        }

        void MapTreeView::selectionDidChange(const Model::SelectionResult& result) {
            if (m_ignoreDocumentSelection)
                return;

            SetBool disableTreeSelection(m_ignoreTreeSelection);

            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& selectedObjects = document->selectedObjects();

            wxDataViewItemArray selections;
            AddObjectToItemArray addObjects(selections);
            Model::each(selectedObjects.begin(), selectedObjects.end(), addObjects, Model::MatchAll());
            m_tree->UnselectAll();
            m_tree->SetSelections(selections);

            if (!selections.empty())
                m_tree->EnsureVisible(selections.front());
        }
    }
}
