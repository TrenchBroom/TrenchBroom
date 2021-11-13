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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "NotifierConnection.h"

#include <memory>
#include <string>
#include <vector>

#include <QWidget>

class QStackedLayout;

namespace TrenchBroom {
namespace Model {
class EntityNodeBase;
class Node;
} // namespace Model

namespace View {
class MapDocument;
class Selection;
class SmartPropertyEditor;
class SmartPropertyEditorMatcher;

class SmartPropertyEditorManager : public QWidget {
private:
  using EditorPtr = SmartPropertyEditor*;
  using MatcherPtr = std::shared_ptr<SmartPropertyEditorMatcher>;
  using MatcherEditorPair = std::pair<MatcherPtr, EditorPtr>;
  using EditorList = std::vector<MatcherEditorPair>;

  std::weak_ptr<MapDocument> m_document;

  EditorList m_editors;
  std::string m_propertyKey;
  QStackedLayout* m_stackedLayout;

  NotifierConnection m_notifierConnection;

public:
  explicit SmartPropertyEditorManager(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

  void switchEditor(
    const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes);
  bool isDefaultEditorActive() const;

private:
  SmartPropertyEditor* activeEditor() const;
  void createEditors();

  void connectObservers();

  void selectionDidChange(const Selection& selection);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);

  EditorPtr selectEditor(
    const std::string& propertyKey, const std::vector<Model::EntityNodeBase*>& nodes) const;
  EditorPtr defaultEditor() const;

  void activateEditor(EditorPtr editor, const std::string& propertyKey);
  void deactivateEditor();
  void updateEditor();
};
} // namespace View
} // namespace TrenchBroom
