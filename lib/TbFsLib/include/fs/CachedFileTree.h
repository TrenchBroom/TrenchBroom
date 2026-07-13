/*
 Copyright (C) 2026 Kristian Duske

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

#include "fs/TraversalMode.h"

#include "kd/overload.h"
#include "kd/path_hash.h"
#include "kd/path_utils.h"

#include <filesystem>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace tb::fs
{

/** A cached, case-insensitively-searchable directory tree, shared by all FileSystem
 * implementations that build their whole directory structure once (on construction or
 * reload()) and answer queries from that cache instead of the backing storage. `Payload`
 * is whatever a leaf (file) entry needs to carry to answer an open request later - e.g.
 * a lazy loader for archive-backed file systems, or nothing at all
 * (`std::monostate`) for file systems that just reconstruct a real path and reopen it.
 */
template <typename Payload>
struct CachedFileEntry
{
  std::filesystem::path name;
  Payload payload;
};

template <typename Payload>
struct CachedDirectoryEntry;

template <typename Payload>
using CachedEntry = std::variant<CachedDirectoryEntry<Payload>, CachedFileEntry<Payload>>;

template <typename Payload>
struct CachedDirectoryEntry
{
  std::filesystem::path name;
  std::vector<CachedEntry<Payload>> entries;
  std::unordered_map<std::filesystem::path, size_t, kdl::path_hash> entryMapLC;
};

template <typename Payload>
const std::filesystem::path& getEntryName(const CachedEntry<Payload>& entry)
{
  return std::visit(
    [](const auto& x) -> const std::filesystem::path& { return x.name; }, entry);
}

template <typename Payload>
bool isDirectoryEntry(const CachedEntry<Payload>& entry)
{
  return std::visit(
    kdl::overload(
      [](const CachedDirectoryEntry<Payload>&) { return true; },
      [](const CachedFileEntry<Payload>&) { return false; }),
    entry);
}

template <typename Payload>
CachedEntry<Payload>& addCachedEntry(
  CachedDirectoryEntry<Payload>& parent,
  std::type_identity_t<CachedEntry<Payload>>&& entry)
{
  parent.entryMapLC[kdl::path_to_lower(getEntryName(entry))] = parent.entries.size();
  parent.entries.push_back(std::move(entry));
  return parent.entries.back();
}

template <typename DirEntry>
auto findChildEntry(DirEntry& directoryEntry, const std::filesystem::path& nameLC)
{
  using difftype =
    typename std::decay_t<decltype(directoryEntry.entries)>::difference_type;

  const auto entryIndexIt = directoryEntry.entryMapLC.find(nameLC);
  return entryIndexIt != directoryEntry.entryMapLC.end()
           ? std::next(directoryEntry.entries.begin(), difftype(entryIndexIt->second))
           : directoryEntry.entries.end();
}

template <typename Payload, typename F>
auto withCachedEntry(
  const std::filesystem::path& searchPathLC,
  const CachedEntry<Payload>& currentEntry,
  const std::filesystem::path& currentPath,
  const F& f,
  decltype(f(
    std::declval<const CachedEntry<Payload>&>(),
    std::declval<const std::filesystem::path&>())) defaultResult)
{
  if (searchPathLC.empty())
  {
    return f(currentEntry, currentPath);
  }

  return std::visit(
    kdl::overload(
      [&](const CachedDirectoryEntry<Payload>& directoryEntry) {
        const auto nameLC = kdl::path_front(searchPathLC);
        const auto entryIt = findChildEntry(directoryEntry, nameLC);

        return entryIt != directoryEntry.entries.end()
                 ? withCachedEntry(
                     kdl::path_pop_front(searchPathLC),
                     *entryIt,
                     currentPath / getEntryName(*entryIt),
                     f,
                     defaultResult)
                 : defaultResult;
      },
      [&](const CachedFileEntry<Payload>&) { return defaultResult; }),
    currentEntry);
}

template <typename Payload, typename F>
void withCachedEntry(
  const std::filesystem::path& searchPathLC,
  const CachedEntry<Payload>& currentEntry,
  const std::filesystem::path& currentPath,
  const F& f)
{
  if (searchPathLC.empty())
  {
    f(currentEntry, currentPath);
  }
  else
  {
    std::visit(
      kdl::overload(
        [&](const CachedDirectoryEntry<Payload>& directoryEntry) {
          const auto nameLC = kdl::path_front(searchPathLC);
          const auto entryIt = findChildEntry(directoryEntry, nameLC);

          if (entryIt != directoryEntry.entries.end())
          {
            withCachedEntry(
              kdl::path_pop_front(searchPathLC),
              *entryIt,
              currentPath / getEntryName(*entryIt),
              f);
          }
        },
        [&](const CachedFileEntry<Payload>&) {}),
      currentEntry);
  }
}

template <typename Payload>
const CachedEntry<Payload>* findCachedEntry(
  const std::filesystem::path& pathLC, const CachedEntry<Payload>& parent)
{
  return withCachedEntry(
    pathLC,
    parent,
    std::filesystem::path{},
    [](
      const CachedEntry<Payload>& entry, const std::filesystem::path&) { return &entry; },
    nullptr);
}

template <typename Payload>
CachedDirectoryEntry<Payload>& findOrCreateCachedDirectory(
  const std::filesystem::path& path, CachedDirectoryEntry<Payload>& parent)
{
  if (path.empty())
  {
    return parent;
  }

  auto name = kdl::path_front(path);
  auto nameLC = kdl::path_to_lower(name);
  auto entryIt = findChildEntry(parent, nameLC);
  if (entryIt != parent.entries.end())
  {
    return std::visit(
      kdl::overload(
        [&](CachedDirectoryEntry<Payload>& directoryEntry)
          -> CachedDirectoryEntry<Payload>& {
          return findOrCreateCachedDirectory(kdl::path_pop_front(path), directoryEntry);
        },
        [&](CachedFileEntry<Payload>&) -> CachedDirectoryEntry<Payload>& {
          *entryIt = CachedDirectoryEntry<Payload>{std::move(name), {}, {}};
          return findOrCreateCachedDirectory(
            kdl::path_pop_front(path), std::get<CachedDirectoryEntry<Payload>>(*entryIt));
        }),
      *entryIt);
  }
  else
  {
    return findOrCreateCachedDirectory(
      kdl::path_pop_front(path),
      std::get<CachedDirectoryEntry<Payload>>(
        addCachedEntry(parent, CachedDirectoryEntry<Payload>{std::move(name), {}, {}})));
  }
}

template <typename Payload>
void collectCachedEntries(
  const CachedEntry<Payload>& entry,
  const std::filesystem::path& entryPath,
  const size_t depth,
  const TraversalMode& traversalMode,
  std::vector<std::filesystem::path>& result)
{
  if (!traversalMode.depth || depth <= *traversalMode.depth)
  {
    std::visit(
      kdl::overload(
        [&](const CachedDirectoryEntry<Payload>& directoryEntry) {
          for (const auto& childEntry : directoryEntry.entries)
          {
            const auto childPath = entryPath / getEntryName(childEntry);
            result.push_back(childPath);
            collectCachedEntries(childEntry, childPath, depth + 1, traversalMode, result);
          }
        },
        [](const CachedFileEntry<Payload>&) {}),
      entry);
  }
}

} // namespace tb::fs
