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

#include "EntityPropertyGridTable.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/ModelUtils.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"
#include "View/ViewUtils.h"

namespace TrenchBroom {
    namespace View {
        EntityPropertyGridTable::PropertyRow::PropertyRow() :
        m_maxCount(0),
        m_count(0),
        m_multi(false) {}
        
        EntityPropertyGridTable::PropertyRow::PropertyRow(const String& key, const String& value, const String& tooltip, const size_t maxCount) :
        m_key(key),
        m_value(value),
        m_tooltip(tooltip),
        m_maxCount(maxCount),
        m_count(1),
        m_multi(false) {}
        
        const String& EntityPropertyGridTable::PropertyRow::key() const {
            return m_key;
        }
        
        const String& EntityPropertyGridTable::PropertyRow::value() const {
            return m_value;
        }
        
        const String& EntityPropertyGridTable::PropertyRow::tooltip() const {
            return m_multi ? EmptyString : m_tooltip;
        }

        void EntityPropertyGridTable::PropertyRow::compareValue(const String& i_value) {
            m_multi |= (m_value != i_value);
            ++m_count;
        }
        
        bool EntityPropertyGridTable::PropertyRow::multi() const {
            return m_multi;
        }
        
        bool EntityPropertyGridTable::PropertyRow::subset() const {
            return m_count < m_maxCount;
        }
        
        void EntityPropertyGridTable::PropertyRow::reset() {
            m_count = m_maxCount;
            m_multi = false;
        }

        EntityPropertyGridTable::DefaultRow::DefaultRow() {}
        
        EntityPropertyGridTable::DefaultRow::DefaultRow(const String& key, const String& value, const String& tooltip) :
        m_key(key),
        m_value(value),
        m_tooltip(tooltip) {}

        const String& EntityPropertyGridTable::DefaultRow::key() const {
            return m_key;
        }
        
        const String& EntityPropertyGridTable::DefaultRow::value() const {
            return m_value;
        }
        
        const String& EntityPropertyGridTable::DefaultRow::tooltip() const {
            return m_tooltip;
        }

        size_t EntityPropertyGridTable::RowManager::propertyCount() const {
            return m_propertyRows.size();
        }

        size_t EntityPropertyGridTable::RowManager::rowCount() const {
            return m_propertyRows.size() + m_defaultRows.size();
        }

        bool EntityPropertyGridTable::RowManager::isPropertyRow(const size_t rowIndex) const {
            assert(rowIndex < rowCount());
            return rowIndex < m_propertyRows.size();
        }
        
        bool EntityPropertyGridTable::RowManager::isDefaultRow(const size_t rowIndex) const {
            return !isPropertyRow(rowIndex);
        }

        size_t EntityPropertyGridTable::RowManager::indexOf(const String& key) const {
            PropertyRow::List::const_iterator propIt = findPropertyRow(m_propertyRows, key);
            if (propIt != m_propertyRows.end())
                return static_cast<size_t>(std::distance(m_propertyRows.begin(), propIt));
            
            DefaultRow::List::const_iterator defIt = findDefaultRow(m_defaultRows, key);
            if (defIt != m_defaultRows.end())
                return propertyCount() + static_cast<size_t>(std::distance(m_defaultRows.begin(), defIt));
            return rowCount();
        }

        const String& EntityPropertyGridTable::RowManager::key(const size_t rowIndex) const {
            if (isPropertyRow(rowIndex))
                return propertyRow(rowIndex).key();
            return defaultRow(rowIndex).key();
        }
        
        const String& EntityPropertyGridTable::RowManager::value(const size_t rowIndex) const {
            if (isPropertyRow(rowIndex)) {
                const PropertyRow& row = propertyRow(rowIndex);
                return row.multi() ? EmptyString : row.value();
            }
            return defaultRow(rowIndex).value();
        }
        
        const String& EntityPropertyGridTable::RowManager::tooltip(const size_t rowIndex) const {
            if (isPropertyRow(rowIndex))
                return propertyRow(rowIndex).tooltip();
            return defaultRow(rowIndex).tooltip();
        }

        bool EntityPropertyGridTable::RowManager::multi(size_t rowIndex) const {
            if (!isPropertyRow(rowIndex))
                return false;
            const PropertyRow& row = propertyRow(rowIndex);
            return row.multi();
        }

        bool EntityPropertyGridTable::RowManager::subset(const size_t rowIndex) const {
            if (!isPropertyRow(rowIndex))
                return false;
            const PropertyRow& row = propertyRow(rowIndex);
            return row.subset();
        }

        const StringList EntityPropertyGridTable::RowManager::keys(const size_t rowIndex, const size_t count) const {
            assert(rowIndex + count <= propertyCount());
            
            StringList result(count);
            for (size_t i = 0; i < count; ++i)
                result[i] = m_propertyRows[rowIndex + i].key();
            return result;
        }

        void EntityPropertyGridTable::RowManager::updateRows(const Model::EntityList& entities) {
            PropertyRow::List newPropertyRows = collectPropertyRows(entities);
            DefaultRow::List newDefaultRows = collectDefaultRows(entities, newPropertyRows);
            
            std::swap(m_propertyRows, newPropertyRows);
            std::swap(m_defaultRows, newDefaultRows);
        }

        StringList EntityPropertyGridTable::RowManager::insertRows(const size_t rowIndex, const size_t count, const Model::EntityList& entities) {
            assert(rowIndex <= propertyCount());
            
            const StringList keyNames = newKeyNames(count, entities);
            assert(keyNames.size() == count);
            
            PropertyRow::List::iterator entryIt = m_propertyRows.begin();
            std::advance(entryIt, rowIndex);
            for (size_t i = 0; i < count; i++) {
                entryIt = m_propertyRows.insert(entryIt, PropertyRow(keyNames[i], "", "", entities.size()));
                entryIt->reset();
                std::advance(entryIt, 1);
            }
            
            return keyNames;
        }

        void EntityPropertyGridTable::RowManager::deleteRows(const size_t rowIndex, const size_t count) {
            assert(rowIndex + count <= propertyCount());
            
            PropertyRow::List::iterator first = m_propertyRows.begin();
            PropertyRow::List::iterator last = first;
            std::advance(first, rowIndex);
            std::advance(last, rowIndex + count);
            m_propertyRows.erase(first, last);
        }

        const EntityPropertyGridTable::PropertyRow& EntityPropertyGridTable::RowManager::propertyRow(const size_t rowIndex) const {
            assert(rowIndex < m_propertyRows.size());
            return m_propertyRows[rowIndex];
        }
        
        EntityPropertyGridTable::PropertyRow& EntityPropertyGridTable::RowManager::propertyRow(const size_t rowIndex) {
            assert(rowIndex < m_propertyRows.size());
            return m_propertyRows[rowIndex];
        }
        
        const EntityPropertyGridTable::DefaultRow& EntityPropertyGridTable::RowManager::defaultRow(const size_t rowIndex) const {
            assert(rowIndex >= m_propertyRows.size());
            assert(rowIndex < rowCount());
            return m_defaultRows[rowIndex - m_propertyRows.size()];
        }

        EntityPropertyGridTable::DefaultRow& EntityPropertyGridTable::RowManager::defaultRow(const size_t rowIndex) {
            assert(rowIndex >= m_propertyRows.size());
            assert(rowIndex < rowCount());
            return m_defaultRows[rowIndex - m_propertyRows.size()];
        }
        
        EntityPropertyGridTable::PropertyRow::List EntityPropertyGridTable::RowManager::collectPropertyRows(const Model::EntityList& entities) const {
            EntityPropertyGridTable::PropertyRow::List rows;
            
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity& entity = **entityIt;
                const Assets::EntityDefinition* entityDefinition = entity.definition();
                
                const Model::EntityProperty::List& properties = entity.properties();
                Model::EntityProperty::List::const_iterator propertyIt, propertyEnd;
                for (propertyIt = properties.begin(), propertyEnd = properties.end(); propertyIt != propertyEnd; ++propertyIt) {
                    const Model::EntityProperty& property = *propertyIt;
                    const Assets::PropertyDefinition* propertyDefinition = entityDefinition != NULL ? entityDefinition->propertyDefinition(property.key) : NULL;
                    
                    PropertyRow::List::iterator rowIt = findPropertyRow(rows, property.key);
                    if (rowIt != rows.end()) {
                        rowIt->compareValue(property.value);
                    } else {
                        const String& tooltip = propertyDefinition != NULL ? propertyDefinition->description() : EmptyString;
                        rows.push_back(PropertyRow(property.key, property.value, tooltip, entities.size()));
                    }
                }
            }

            return rows;
        }
        
        EntityPropertyGridTable::DefaultRow::List EntityPropertyGridTable::RowManager::collectDefaultRows(const Model::EntityList& entities, const PropertyRow::List& propertyRows) const {
            DefaultRow::List defaultRows;
            const Assets::EntityDefinition* definition = selectEntityDefinition(entities);
            
            if (definition != NULL) {
                const Assets::PropertyDefinitionList& propertyDefs = definition->propertyDefinitions();
                Assets::PropertyDefinitionList::const_iterator propertyIt, propertyEnd;
                for (propertyIt = propertyDefs.begin(), propertyEnd = propertyDefs.end(); propertyIt != propertyEnd; ++propertyIt) {
                    const Assets::PropertyDefinitionPtr propertyDef = *propertyIt;
                    const String& name = propertyDef->name();
                    
                    if (findPropertyRow(propertyRows, name) != propertyRows.end())
                        continue;
                    if (findDefaultRow(defaultRows, name) != defaultRows.end())
                        continue;
                    
                    const String value = Assets::PropertyDefinition::defaultValue(*propertyDef);
                    const String& tooltip = propertyDef->description();
                    
                    defaultRows.push_back(DefaultRow(name, value, tooltip));
                }
            }
            
            return defaultRows;
        }
        
        EntityPropertyGridTable::PropertyRow::List::iterator EntityPropertyGridTable::RowManager::findPropertyRow(PropertyRow::List& rows, const String& key) {
            PropertyRow::List::iterator it, end;
            for (it = rows.begin(), end = rows.end(); it != end; ++it) {
                const PropertyRow& row = *it;
                if (row.key() == key)
                    return it;
            }
            return end;
        }

        EntityPropertyGridTable::PropertyRow::List::const_iterator EntityPropertyGridTable::RowManager::findPropertyRow(const PropertyRow::List& rows, const String& key) {
            PropertyRow::List::const_iterator it, end;
            for (it = rows.begin(), end = rows.end(); it != end; ++it) {
                const PropertyRow& row = *it;
                if (row.key() == key)
                    return it;
            }
            return end;
        }

        EntityPropertyGridTable::DefaultRow::List::iterator EntityPropertyGridTable::RowManager::findDefaultRow(DefaultRow::List& rows, const String& key) {
            DefaultRow::List::iterator it, end;
            for (it = rows.begin(), end = rows.end(); it != end; ++it) {
                const DefaultRow& row = *it;
                if (row.key() == key)
                    return it;
            }
            return end;
        }

        EntityPropertyGridTable::DefaultRow::List::const_iterator EntityPropertyGridTable::RowManager::findDefaultRow(const DefaultRow::List& rows, const String& key) {
            DefaultRow::List::const_iterator it, end;
            for (it = rows.begin(), end = rows.end(); it != end; ++it) {
                const DefaultRow& row = *it;
                if (row.key() == key)
                    return it;
            }
            return end;
        }
        
        StringList EntityPropertyGridTable::RowManager::newKeyNames(const size_t count, const Model::EntityList& entities) const {
            StringList result;
            result.reserve(count);
            
            size_t index = 1;
            for (size_t i = 0; i < count; ++i) {
                while (true) {
                    StringStream keyStream;
                    keyStream << "property " << index;
                    
                    bool indexIsFree = true;
                    Model::EntityList::const_iterator it, end;
                    for (it = entities.begin(), end = entities.end(); it != end && indexIsFree; ++it) {
                        const Model::Entity& entity = **it;
                        indexIsFree = !entity.hasProperty(keyStream.str());
                    }
                    
                    if (indexIsFree) {
                        result.push_back(keyStream.str());
                        break;
                    }
                    
                    ++index;
                }
            }
            return result;
        }

        EntityPropertyGridTable::EntityPropertyGridTable(MapDocumentWPtr document, ControllerWPtr controller) :
        m_document(document),
        m_controller(controller),
        m_ignoreUpdates(false),
        m_readonlyCellColor(wxColor(224, 224, 224)),
        m_specialCellColor(wxColor(128, 128, 128)) {}
        
        int EntityPropertyGridTable::GetNumberRows() {
            return static_cast<int>(m_rows.rowCount());
        }
        
        int EntityPropertyGridTable::GetNumberPropertyRows() const {
            return static_cast<int>(m_rows.propertyCount());
        }
        
        int EntityPropertyGridTable::GetNumberCols() {
            return 2;
        }
        
        wxString EntityPropertyGridTable::GetValue(const int row, const int col) {
            assert(row >= 0 && row < GetRowsCount());
            assert(col >= 0 && col < GetColsCount());
            
            const size_t rowIndex = static_cast<size_t>(row);
            if (col == 0)
                return m_rows.key(rowIndex);
            return m_rows.value(rowIndex);
        }
        
        void EntityPropertyGridTable::SetValue(const int row, const int col, const wxString& value) {
            assert(row >= 0 && row < GetRowsCount());
            assert(col >= 0 && col < GetColsCount());
            
            MapDocumentSPtr document = lock(m_document);
            
            const size_t rowIndex = static_cast<size_t>(row);
            const Model::EntityList entities = document->allSelectedEntities();
            assert(!entities.empty());
            
            const SetBool ignoreUpdates(m_ignoreUpdates);
            if (col == 0)
                renameProperty(rowIndex, value.ToStdString(), entities);
            else
                updateProperty(rowIndex, value.ToStdString(), entities);
        }
        
        void EntityPropertyGridTable::Clear() {
            DeleteRows(0, static_cast<size_t>(GetRowsCount()));
        }
        
        bool EntityPropertyGridTable::InsertRows(const size_t pos, const size_t numRows) {
            assert(static_cast<int>(pos) <= GetRowsCount());
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            const Model::EntityList entities = document->allSelectedEntities();
            assert(!entities.empty());
            
            const StringList newKeys = m_rows.insertRows(pos, numRows, entities);

            const SetBool ignoreUpdates(m_ignoreUpdates);

            const UndoableCommandGroup commandGroup(controller);
            StringList::const_iterator it, end;
            for (it = newKeys.begin(), end = newKeys.end(); it != end; ++it) {
                const String& key = *it;
                controller->setEntityProperty(entities, key, "");
            }
            
            notifyRowsInserted(pos, numRows);
            
            return true;
        }
        
        bool EntityPropertyGridTable::AppendRows(const size_t numRows) {
            return InsertRows(m_rows.propertyCount(), numRows);
        }
        
        bool EntityPropertyGridTable::DeleteRows(const size_t pos, const size_t numRows) {
            // TODO: when deleting a property that has a default value in the property definition, re-add it to the list
            // of default properties...
            
            assert(pos + numRows <= m_rows.propertyCount());
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            const Model::EntityList entities = document->allSelectedEntities();
            assert(!entities.empty());
            
            const StringList keys = m_rows.keys(pos, numRows);
            assert(keys.size() == numRows);
            
            const SetBool ignoreUpdates(m_ignoreUpdates);
            const UndoableCommandGroup commandGroup(controller, numRows == 1 ? "Remove Property" : "Remove Properties");
            
            bool success = true;
            for (size_t i = 0; i < numRows && success; i++)
                success = controller->removeEntityProperty(entities, keys[i]);
            
            if (!success) {
                controller->rollbackGroup();
                return false;
            }

            m_rows.deleteRows(pos, numRows);
            notifyRowsDeleted(pos, numRows);
            return true;
        }
        
        wxString EntityPropertyGridTable::GetColLabelValue(const int col) {
            assert(col >= 0 && col < GetColsCount());
            if (col == 0)
                return "Key";
            return "Value";
        }
        
        wxGridCellAttr* EntityPropertyGridTable::GetAttr(const int row, const int col, const wxGridCellAttr::wxAttrKind kind) {
            if (row < 0 || row >= GetRowsCount() ||
                col < 0 || col >= GetColsCount())
                return NULL;
            
            const size_t rowIndex = static_cast<size_t>(row);
            wxGridCellAttr* attr = wxGridTableBase::GetAttr(row, col, kind);
            if (attr == NULL)
                attr = new wxGridCellAttr();
            
            if (col == 0) {
                if (m_rows.isDefaultRow(rowIndex)) {
                    attr->SetFont(GetView()->GetFont().MakeItalic());
                    attr->SetReadOnly();
                } else {
                    attr->SetFont(GetView()->GetFont());
                    
                    const String& key = m_rows.key(rowIndex);
                    const bool subset = m_rows.subset(rowIndex);
                    const bool readonly = !Model::isPropertyKeyMutable(key) || !Model::isPropertyValueMutable(key);
                    if (readonly) {
                        attr->SetReadOnly(true);
                        attr->SetBackgroundColour(m_readonlyCellColor);
                    } else if (subset) {
                        attr->SetTextColour(m_specialCellColor);
                    }
                }
            } else if (col == 1) {
                if (m_rows.isDefaultRow(rowIndex)) {
                    attr->SetFont(GetView()->GetFont().MakeItalic());
                } else {
                    attr->SetFont(GetView()->GetFont());

                    const String& key = m_rows.key(rowIndex);
                    const bool multi = m_rows.multi(rowIndex);
                    const bool readonly = !Model::isPropertyValueMutable(key);
                    if (readonly) {
                        attr->SetReadOnly(true);
                        attr->SetBackgroundColour(m_readonlyCellColor);
                    }
                    if (multi)
                        attr->SetTextColour(m_specialCellColor);
                }
            }
            return attr;
        }
        
        void EntityPropertyGridTable::update() {
            if (m_ignoreUpdates)
                return;
            
            MapDocumentSPtr document = lock(m_document);
            const size_t oldRowCount = m_rows.rowCount();
            m_rows.updateRows(document->allSelectedEntities());
            const size_t newRowCount = m_rows.rowCount();
            
            if (oldRowCount < newRowCount)
                notifyRowsAppended(newRowCount - oldRowCount);
            else if (oldRowCount > newRowCount)
                notifyRowsDeleted(oldRowCount - 1, oldRowCount - newRowCount);
            notifyRowsUpdated(0, newRowCount);
        }

        String EntityPropertyGridTable::tooltip(const wxGridCellCoords cellCoords) const {
            if (cellCoords.GetRow() < 0 || cellCoords.GetRow() >= GetRowsCount())
                return "";
            
            const size_t rowIndex = static_cast<size_t>(cellCoords.GetRow());
            return m_rows.tooltip(rowIndex);
        }
        
        Model::PropertyKey EntityPropertyGridTable::propertyKey(const int row) const {
            if (row < 0 || row >= static_cast<int>(m_rows.rowCount()))
                return "";
            return m_rows.key(static_cast<size_t>(row));
        }
        
        int EntityPropertyGridTable::rowForKey(const Model::PropertyKey& key) const {
            const size_t index = m_rows.indexOf(key);
            if (index >= m_rows.rowCount())
                return -1;
            return static_cast<int>(index);
        }

        void EntityPropertyGridTable::renameProperty(size_t rowIndex, const String& newKey, const Model::EntityList& entities) {
            assert(rowIndex < m_rows.propertyCount());
            
            ControllerSPtr controller = lock(m_controller);
            const String& oldKey = m_rows.key(rowIndex);
            if (controller->renameEntityProperty(entities, oldKey, newKey)) {
                m_rows.updateRows(entities);
                notifyRowsUpdated(0, m_rows.rowCount());
            }
        }
        
        void EntityPropertyGridTable::updateProperty(size_t rowIndex, const String& newValue, const Model::EntityList& entities) {
            assert(rowIndex < m_rows.rowCount());

            ControllerSPtr controller = lock(m_controller);
            const String& key = m_rows.key(rowIndex);
            if (controller->setEntityProperty(entities, key, newValue)) {
                m_rows.updateRows(entities);
                notifyRowsUpdated(0, m_rows.rowCount());
            }
        }
        
        void EntityPropertyGridTable::notifyRowsUpdated(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityPropertyGridTable::notifyRowsInserted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityPropertyGridTable::notifyRowsAppended(size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
        
        void EntityPropertyGridTable::notifyRowsDeleted(size_t pos, size_t numRows) {
            if (GetView() != NULL) {
                wxGridTableMessage message(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                           static_cast<int>(pos),
                                           static_cast<int>(numRows));
                GetView()->ProcessTableMessage(message);
            }
        }
    }
}
