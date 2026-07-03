/*
 Copyright (C) 2025 Kristian Duske

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

#include "Result.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tb::fs
{
class Reader;
}

namespace tb::mdl
{

/**
 * UE1 (Unreal/UT99) package reader.
 *
 * This is a direct port of `uepkg.py` (the validated reference implementation).
 * It implements the UE1/UE2 path extracted from UModel's `UnPackage.cpp`:
 * `FPackageFileSummary`, the name/import/export tables, and the primitives
 * they are built from (`FCompactIndex`, `FString`).
 *
 * Validated FileVersions: 61 (Unreal, no GUID/heritage path) and 68 (UT99,
 * GUID/generations path). See uepkg.py for the field layouts this ports.
 */
inline constexpr uint32_t UnrealPackageFileTag = 0x9E2A83C1u;

struct UnrealObjectImport
{
  std::string classPackage;
  std::string className;
  int32_t packageIndex;
  std::string objectName;
};

struct UnrealObjectExport
{
  int32_t classIndex;
  int32_t superIndex;
  int32_t packageIndex;
  std::string objectName;
  uint32_t objectFlags;
  int32_t serialSize;
  int32_t serialOffset;
};

/**
 * UE1 tagged property types (EPropType), as read from FPropertyTag.
 */
enum class UnrealPropertyType : uint8_t
{
  Byte = 1,
  Int = 2,
  Bool = 3,
  Float = 4,
  Object = 5,
  Name = 6,
  String = 7,
  Class = 8,
  Array = 9,
  Struct = 10,
  Vector = 11,
  Rotator = 12,
  Str = 13,
  Map = 14,
  FixedArray = 15,
};

/**
 * A single entry from a UE1 tagged-property list (FPropertyTag).
 *
 * Only the fields needed by the current integration seams are decoded
 * (Bool, Object/Class references, and Name/Str/String values); all other
 * payloads are consumed from the reader but discarded. Extend this as
 * later seams (e.g. actor class defaults) need more property types.
 */
struct UnrealProperty
{
  std::string name;
  UnrealPropertyType type;
  bool boolValue = false;
  std::optional<std::string> objectRef;
  std::string stringValue;
};

class UnrealPackage
{
public:
  uint16_t fileVersion = 0;
  uint16_t licenseeVersion = 0;
  uint32_t packageFlags = 0;
  std::vector<std::string> names;
  std::vector<UnrealObjectImport> imports;
  std::vector<UnrealObjectExport> exports;

  /**
   * Resolves the class name of the given export. A `classIndex` of 0 means
   * the export is itself a UClass; negative indices reference the import
   * table, positive indices reference the export table (both 1-based).
   */
  std::string classNameOfExport(const UnrealObjectExport& e) const;

  /**
   * Resolves an Object/Class property reference (a signed FCompactIndex):
   * 0 is `None`, negative indices reference imports, positive indices
   * reference exports (both 1-based).
   */
  std::optional<std::string> resolveObjectRef(int32_t index) const;

  /**
   * Finds an export by object name. UE1 packages used by Phase 1 are
   * self-contained (a Texture's Palette lives in the same package), so this
   * is sufficient for resolving property references within one package.
   */
  const UnrealObjectExport* findExport(const std::string& objectName) const;
};

/**
 * Reads an FCompactIndex: a signed varint used throughout UE1 packages for
 * name/object indices and sizes. Exact port of UModel's
 * `FArchive::operator<<(FCompactIndex&)` (see `Reader.cindex()` in uepkg.py).
 */
int32_t readUnrealCompactIndex(fs::Reader& reader);

/**
 * Reads an FString: a compact-index length prefix followed by that many
 * bytes. A positive length is an ANSI string including its trailing NUL; a
 * negative length is a UTF-16 string (not expected in UE1 .utx/.u/.unr
 * packages, but consumed correctly to keep the reader in sync).
 */
std::string readUnrealString(fs::Reader& reader);

/**
 * Reads the UE1 tagged-property list that begins every object's serial
 * data (FPropertyTag, UObject::Serialize), stopping at the terminating
 * `None` name. `reader` must be positioned at the start of the object's
 * serial data.
 */
std::vector<UnrealProperty> readUnrealPropertyList(
  const UnrealPackage& package, fs::Reader& reader);

/**
 * Parses a UE1 package (.utx/.u/.unr) from `reader`, covering the entire
 * package file: FPackageFileSummary, name table, import table, and export
 * table.
 */
Result<UnrealPackage> loadUnrealPackage(fs::Reader& reader);

} // namespace tb::mdl
