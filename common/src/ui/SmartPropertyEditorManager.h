/*
 Copyright (C) 2010 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QWidget>

#include "NotifierConnection.h"

#include <functional>
#include <string>
#include <vector>

class QStackedLayout;

namespace tb::mdl
{
class EntityNodeBase;
class Node;
} // namespace tb::mdl

namespace tb::ui
{
class MapDocument;
class Selection;
class SmartPropertyEditor;

using SmartPropertyEditorMatcher =
  std::function<bool(const std::string&, const std::vector<mdl::EntityNodeBase*>&)>;

class SmartPropertyEditorManager : public QWidget
{
private:
  std::weak_ptr<MapDocument> m_document;

  std::vector<std::tuple<SmartPropertyEditorMatcher, SmartPropertyEditor*>> m_editors;
  std::string m_propertyKey;
  QStackedLayout* m_stackedLayout = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit SmartPropertyEditorManager(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

  void switchEditor(
    const std::string& propertyKey, const std::vector<mdl::EntityNodeBase*>& nodes);
  bool isDefaultEditorActive() const;

private:
  SmartPropertyEditor* activeEditor() const;
  void createEditors();
  void registerEditor(SmartPropertyEditorMatcher matcher, SmartPropertyEditor* editor);

  void connectObservers();

  void selectionDidChange(const Selection& selection);
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);

  SmartPropertyEditor* selectEditor(
    const std::string& propertyKey, const std::vector<mdl::EntityNodeBase*>& nodes) const;
  SmartPropertyEditor* defaultEditor() const;

  void activateEditor(SmartPropertyEditor* editor, const std::string& propertyKey);
  void deactivateEditor();
  void updateEditor();
};

} // namespace tb::ui
