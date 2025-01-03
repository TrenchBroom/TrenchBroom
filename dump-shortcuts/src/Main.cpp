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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>

#include "KeyStrings.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "io/PathQt.h"
#include "ui/Actions.h"

#include <array>
#include <tuple>

namespace tb::ui
{
namespace
{
QString escapeString(const QString& str)
{
  return str == "'" ? "\\'" : str == "\\" ? "\\\\" : str;
}

void printKeys(QTextStream& out)
{
  const auto keyStrings = KeyStrings{};

  out << "const keys = {\n";
  for (const auto& [portable, native] : keyStrings)
  {
    out << "    '" << escapeString(portable) << "': '" << escapeString(native) << "',\n";
  }
  out << "};\n";
}

QString toString(const QStringList& path, const QString& suffix)
{
  auto result = QString{};
  result += "[";
  for (const auto& component : path)
  {
    result += "'" + component + "', ";
  }
  result += "'" + suffix + "'";
  result += "]";
  return result;
}

QString toString(const QKeySequence& keySequence)
{
  static const std::array<std::tuple<int, QString>, 4> Modifiers = {
    std::make_tuple(static_cast<int>(Qt::CTRL), QString::fromLatin1("Ctrl")),
    std::make_tuple(static_cast<int>(Qt::ALT), QString::fromLatin1("Alt")),
    std::make_tuple(static_cast<int>(Qt::SHIFT), QString::fromLatin1("Shift")),
    std::make_tuple(static_cast<int>(Qt::META), QString::fromLatin1("Meta")),
  };

  auto result = QString{};
  result += "{ ";

  if (keySequence.count() > 0)
  {
    const auto keyWithModifier = keySequence[0].toCombined();
    const auto key = keyWithModifier & ~(static_cast<int>(Qt::KeyboardModifierMask));

    const auto keyPortableText = QKeySequence{key}.toString(QKeySequence::PortableText);

    result += "key: '" + escapeString(keyPortableText) + "', ";
    result += "modifiers: [";
    for (const auto& [modifier, portableText] : Modifiers)
    {
      if ((keyWithModifier & modifier) != 0)
      {
        result += "'" + escapeString(portableText) + "', ";
      }
    }
    result += "]";
  }
  else
  {
    result += "key: '', modifiers: []";
  }
  result += " }";
  return result;
}

void printMenuShortcuts(QTextStream& out)
{
  out << "const menu = {\n";

  auto currentPath = QStringList{};
  const auto& actionManager = ActionManager::instance();
  actionManager.visitMainMenu(kdl::overload(
    [](const MenuSeparator&) {},
    [&](const MenuAction& actionItem) {
      out << "    '" << io::pathAsGenericQString(actionItem.action.preferencePath())
          << "': "
          << "{ path: " << toString(currentPath, actionItem.action.label())
          << ", shortcut: " << toString(actionItem.action.keySequence()) << " },\n";
    },
    [&](const auto& thisLambda, const Menu& menu) {
      currentPath.push_back(QString::fromStdString(menu.name));
      menu.visitEntries(thisLambda);
      currentPath.pop_back();
    }));

  out << "};\n";
}

void printActionShortcuts(QTextStream& out)
{
  out << "const actions = {\n";

  auto printPref = [&out](const auto& prefPath, const auto& keySequence) {
    out << "    '" << io::pathAsGenericQString(prefPath) << "': ";
    out << toString(keySequence) << ",\n";
  };

  const auto& actionManager = ActionManager::instance();
  actionManager.visitToolBar(kdl::overload(
    [](const MenuSeparator&) {},
    [&](const MenuAction& actionItem) {
      printPref(actionItem.action.preferencePath(), actionItem.action.keySequence());
    },
    [&](const auto& thisLambda, const Menu& menu) { menu.visitEntries(thisLambda); }));
  actionManager.visitMapViewActions([&](const auto& action) {
    printPref(action.preferencePath(), action.keySequence());
  });

  // some keys are just Preferences (e.g. WASD)
  for (auto* keyPref : Preferences::keyPreferences())
  {
    printPref(keyPref->path(), keyPref->defaultValue());
  }

  out << "};\n";
}

} // namespace
} // namespace tb::ui

extern void qt_set_sequence_auto_mnemonic(bool b);

int main(int argc, char* argv[])
{
  QSettings::setDefaultFormat(QSettings::IniFormat);

  // We can't use auto mnemonics in TrenchBroom. e.g. by default with Qt, Alt+D opens the
  // "Debug" menu, Alt+S activates the "Show default properties" checkbox in the entity
  // inspector. Flying with Alt held down and pressing WASD is a fundamental behaviour in
  // TB, so we can't have shortcuts randomly activating.
  qt_set_sequence_auto_mnemonic(false);

  auto out = QTextStream{stdout};

  tb::PreferenceManager::createInstance<tb::AppPreferenceManager>();

  // QKeySequence requires that an application instance is created!
  auto app = QApplication{argc, argv};
  app.setApplicationName("TrenchBroom");
  // Needs to be "" otherwise Qt adds this to the paths returned by QStandardPaths
  // which would cause preferences to move from where they were with wx
  app.setOrganizationName("");
  app.setOrganizationDomain("io.github.trenchbroom");

  tb::ui::printKeys(out);
  tb::ui::printMenuShortcuts(out);
  tb::ui::printActionShortcuts(out);

  tb::PreferenceManager::destroyInstance();

  out.flush();
  return out.status() == QTextStream::Ok ? 0 : 1;
}
