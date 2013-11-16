/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
#include "Model/Object.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"

#include <cassert>

#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/variant.h>

namespace TrenchBroom {
    namespace View {
        class MapTreeViewDataModel : public wxDataViewModel {
        private:
            MapDocumentPtr m_document;
        public:
            MapTreeViewDataModel(MapDocumentPtr document) :
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
                static const wxVariant stringVariant(_(""));
                return stringVariant.GetType();
            }

            bool IsContainer(const wxDataViewItem& item) const {
                const Model::Object* object = reinterpret_cast<Model::Object*>(item.GetID());
                if (object == NULL)
                    return false;
                if (object->type() == Model::Object::OTEntity) {
                    const Model::Entity* entity = static_cast<const Model::Entity*>(object);
                    const Model::BrushList& brushes = entity->brushes();
                    return !brushes.empty();
                }
                return false;
            }
            
            struct AddObjectToItemArray {
                wxDataViewItemArray& items;
                
                AddObjectToItemArray(wxDataViewItemArray& i_items) :
                items(i_items) {}
                
                void operator()(Model::Object* object) const {
                    items.push_back(wxDataViewItem(object));
                }
            };
            
            unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const {
                const void* data = item.GetID();
                if (data == NULL) {
                    const Model::Map* map = m_document->map();
                    if (map != NULL) {
                        const Model::EntityList& entities = map->entities();
                        
                        AddObjectToItemArray addObjects(children);
                        Model::each(entities.begin(), entities.end(), addObjects, Model::MatchAll());
                    }
                } else {
                    const Model::Object* object = reinterpret_cast<const Model::Object*>(data);
                    if (object->type() == Model::Object::OTEntity) {
                        const Model::Entity* entity = static_cast<const Model::Entity*>(object);
                        const Model::BrushList& brushes = entity->brushes();
                        
                        AddObjectToItemArray addObjects(children);
                        Model::each(brushes.begin(), brushes.end(), addObjects, Model::MatchAll());
                    }
                }
                
                return children.size();
            }
            
            wxDataViewItem GetParent(const wxDataViewItem& item) const {
                const void* data = item.GetID();
                if (data == NULL)
                    return wxDataViewItem(NULL);
                
                const Model::Object* object = reinterpret_cast<const Model::Object*>(data);
                if (object->type() == Model::Object::OTEntity)
                    return wxDataViewItem(NULL);
                
                if (object->type() == Model::Object::OTBrush) {
                    const Model::Brush* brush = reinterpret_cast<const Model::Brush*>(object);
                    return wxDataViewItem(brush->parent());
                }
                
                // should not happen
                assert(false);
                return wxDataViewItem(NULL);
            }
            
            void GetValue(wxVariant& result, const wxDataViewItem& item, unsigned int col) const {
                assert(col == 0);
                const void* data = item.GetID();
                if (data == NULL) {
                    result = wxVariant("Map");
                } else {
                    const Model::Object* object = reinterpret_cast<const Model::Object*>(data);
                    if (object->type() == Model::Object::OTEntity) {
                        const Model::Entity* entity = static_cast<const Model::Entity*>(object);
                        result = wxVariant(wxString(entity->classname("missing classname")));
                    } else if (object->type() == Model::Object::OTBrush) {
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
            
            bool IsListModel() const {
                return false;
            }
            
            bool IsVirtualListModel() const {
                return false;
            }

            void bindObservers() {
                m_document->documentWasNewedNotifier.addObserver(this, &MapTreeViewDataModel::documentWasNewed);
                m_document->documentWasLoadedNotifier.addObserver(this, &MapTreeViewDataModel::documentWasLoaded);
            }
            
            void unbindObservers() {
                m_document->documentWasNewedNotifier.removeObserver(this, &MapTreeViewDataModel::documentWasNewed);
                m_document->documentWasLoadedNotifier.removeObserver(this, &MapTreeViewDataModel::documentWasLoaded);
            }
            
            void documentWasNewed() {
                Cleared();
            }
            
            void documentWasLoaded() {
                Cleared();
            }
        };
        
        MapTreeView::MapTreeView(wxWindow* parent, MapDocumentPtr document) :
        wxPanel(parent),
        m_tree(NULL) {
            m_tree = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER);
            m_tree->AssociateModel(new MapTreeViewDataModel(document));
            m_tree->AppendTextColumn("Caption", 0);
            m_tree->Bind(wxEVT_SIZE, &MapTreeView::OnTreeViewSize, this);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_tree, 1, wxEXPAND);
            SetSizerAndFit(sizer);
        }

        void MapTreeView::OnTreeViewSize(wxSizeEvent& event) {
            m_tree->GetColumn(0)->SetWidth(m_tree->GetClientSize().x);
        }
    }
}
