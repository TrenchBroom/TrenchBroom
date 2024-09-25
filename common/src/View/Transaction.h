/*
 Copyright (C) 2024 Kristian Duske

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

#include <memory>
#include <string>

namespace TrenchBroom::View
{
class MapDocument;

class Transaction
{
public:
  enum class State
  {
    Running,
    Committed,
    Cancelled,
  };

private:
  MapDocument& m_document;
  std::string m_name;
  State m_state;

public:
  explicit Transaction(std::weak_ptr<MapDocument> document, std::string name = "");
  explicit Transaction(std::shared_ptr<MapDocument> document, std::string name = "");
  explicit Transaction(MapDocument& document, std::string name = "");
  ~Transaction();

  State state() const;

  void finish(bool commit);
  bool commit();
  void rollback();
  void cancel();

private:
  void begin();
};

} // namespace TrenchBroom::View
