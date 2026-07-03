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

#include "mdl/LoadUnrealPackage.h"

#include "fs/Reader.h"
#include "fs/ReaderException.h"

#include <fmt/format.h>

#include <array>

namespace tb::mdl
{
namespace
{

/**
 * NUL-terminated ANSI string, used for the name table when FileVersion < 64.
 * Not expected in Phase 1 (Unreal 61 / UT99 68 both have ArVer >= 64), but
 * ported from uepkg.py's `_read_names` for completeness.
 */
std::string readUnrealAnsiCString(fs::Reader& reader)
{
  auto result = std::string{};
  while (true)
  {
    const auto c = reader.readUnsignedChar<uint8_t>();
    if (c == 0)
    {
      break;
    }
    result.push_back(char(c));
  }
  return result;
}

std::string resolveFName(const std::vector<std::string>& names, const int32_t index)
{
  return index >= 0 && size_t(index) < names.size() ? names[size_t(index)]
                                                      : fmt::format("<name#{}>", index);
}

std::string readFName(const std::vector<std::string>& names, fs::Reader& reader)
{
  return resolveFName(names, readUnrealCompactIndex(reader));
}

} // namespace

std::string UnrealPackage::classNameOfExport(const UnrealObjectExport& e) const
{
  if (e.classIndex == 0)
  {
    return "Class";
  }
  if (e.classIndex < 0)
  {
    return imports[size_t(-e.classIndex - 1)].objectName;
  }
  return exports[size_t(e.classIndex - 1)].objectName;
}

std::optional<std::string> UnrealPackage::resolveObjectRef(const int32_t index) const
{
  if (index == 0)
  {
    return std::nullopt;
  }
  if (index < 0)
  {
    return imports[size_t(-index - 1)].objectName;
  }
  return exports[size_t(index - 1)].objectName;
}

const UnrealObjectExport* UnrealPackage::findExport(const std::string& objectName) const
{
  for (const auto& e : exports)
  {
    if (e.objectName == objectName)
    {
      return &e;
    }
  }
  return nullptr;
}

int32_t readUnrealCompactIndex(fs::Reader& reader)
{
  auto b = reader.readUnsignedChar<uint8_t>();
  const auto sign = (b & 0x80) != 0;
  auto r = int64_t(b & 0x3F);
  auto shift = 6;
  if (b & 0x40)
  {
    while (true)
    {
      b = reader.readUnsignedChar<uint8_t>();
      r |= int64_t(b & 0x7F) << shift;
      shift += 7;
      if (!(b & 0x80))
      {
        break;
      }
    }
  }
  return int32_t(sign ? -r : r);
}

std::string readUnrealString(fs::Reader& reader)
{
  const auto len = readUnrealCompactIndex(reader);
  if (len == 0)
  {
    return "";
  }
  if (len > 0)
  {
    // ANSI string: length includes the trailing NUL. Reader::readString()
    // already truncates at the first embedded NUL byte, matching the
    // behavior of uepkg.py's `raw.split(b'\x00', 1)[0]`.
    return reader.readString(size_t(len));
  }

  // Unicode string (negative length) -- not expected in UE1 .utx/.u/.unr
  // packages, but the bytes must still be consumed to keep the reader in
  // sync (uepkg.py: `raw = self.bytes(-ln * 2); raw.decode('utf-16-le')`).
  const auto charCount = size_t(-len);
  auto raw = std::vector<unsigned char>(charCount * 2);
  reader.read(raw.data(), raw.size());

  auto result = std::string{};
  result.reserve(charCount);
  for (size_t i = 0; i < charCount; ++i)
  {
    const auto lo = raw[i * 2];
    const auto hi = raw[i * 2 + 1];
    if (lo == 0 && hi == 0)
    {
      break;
    }
    result.push_back(char(lo));
  }
  return result;
}

std::vector<UnrealProperty> readUnrealPropertyList(
  const UnrealPackage& package, fs::Reader& reader)
{
  auto props = std::vector<UnrealProperty>{};
  while (true)
  {
    const auto name = readFName(package.names, reader);
    if (name == "None")
    {
      break;
    }

    const auto info = reader.readUnsignedChar<uint8_t>();
    const auto isArray = (info & 0x80) != 0;
    const auto ptype = UnrealPropertyType(info & 0x0F);

    if (ptype == UnrealPropertyType::Struct)
    {
      readUnrealCompactIndex(reader); // struct name, unused by current seams
    }

    const auto sizeClass = (info >> 4) & 0x07;
    auto dataSize = int32_t{0};
    switch (sizeClass)
    {
    case 0:
      dataSize = 1;
      break;
    case 1:
      dataSize = 2;
      break;
    case 2:
      dataSize = 4;
      break;
    case 3:
      dataSize = 12;
      break;
    case 4:
      dataSize = 16;
      break;
    case 5:
      dataSize = reader.readUnsignedChar<uint8_t>();
      break;
    case 6:
      dataSize = reader.readUnsignedInt<uint16_t>();
      break;
    case 7:
      dataSize = reader.readInt<int32_t>();
      break;
    }

    if (ptype != UnrealPropertyType::Bool && isArray)
    {
      // Variable-length array index. Exact port of PropReader._read_value's
      // encoding; current seams don't need per-element array indices, but
      // the bytes must still be consumed to keep the reader in sync.
      const auto b0 = reader.readUnsignedChar<uint8_t>();
      if (b0 >= 128)
      {
        reader.readUnsignedChar<uint8_t>();
        if (b0 & 0x40)
        {
          reader.readUnsignedChar<uint8_t>();
          reader.readUnsignedChar<uint8_t>();
        }
      }
    }

    auto prop = UnrealProperty{};
    prop.name = name;
    prop.type = ptype;

    switch (ptype)
    {
    case UnrealPropertyType::Bool:
      prop.boolValue = isArray;
      break;
    case UnrealPropertyType::Byte:
      reader.readUnsignedChar<uint8_t>();
      break;
    case UnrealPropertyType::Int:
      reader.readInt<int32_t>();
      break;
    case UnrealPropertyType::Float:
      reader.readFloat<float>();
      break;
    case UnrealPropertyType::Object:
    case UnrealPropertyType::Class:
      prop.objectRef = package.resolveObjectRef(readUnrealCompactIndex(reader));
      break;
    case UnrealPropertyType::Name:
      prop.stringValue = readFName(package.names, reader);
      break;
    case UnrealPropertyType::Str:
    case UnrealPropertyType::String:
      prop.stringValue = readUnrealString(reader);
      break;
    case UnrealPropertyType::Vector:
    case UnrealPropertyType::Rotator:
      reader.seekForward(12);
      break;
    default:
      // Struct, Array, Map, FixedArray: raw payload, not decoded by current
      // seams.
      reader.seekForward(size_t(dataSize));
      break;
    }

    props.push_back(std::move(prop));
  }
  return props;
}

Result<UnrealPackage> loadUnrealPackage(fs::Reader& reader)
{
  try
  {
    auto pkg = UnrealPackage{};

    const auto tag = reader.readUnsignedInt<uint32_t>();
    if (tag != UnrealPackageFileTag)
    {
      return Error{fmt::format(
        "Bad package tag {:#010x} (expected {:#010x})", tag, UnrealPackageFileTag)};
    }

    const auto version = reader.readUnsignedInt<uint32_t>();
    pkg.fileVersion = uint16_t(version & 0xFFFFu);
    pkg.licenseeVersion = uint16_t(version >> 16);
    pkg.packageFlags = reader.readUnsignedInt<uint32_t>();

    const auto nameCount = reader.readInt<int32_t>();
    const auto nameOffset = reader.readInt<int32_t>();
    const auto exportCount = reader.readInt<int32_t>();
    const auto exportOffset = reader.readInt<int32_t>();
    const auto importCount = reader.readInt<int32_t>();
    const auto importOffset = reader.readInt<int32_t>();

    if (pkg.fileVersion < 68)
    {
      // old heritage path, no GUID
      reader.readInt<int32_t>(); // heritageCount
      reader.readInt<int32_t>(); // heritageOffset
    }
    else
    {
      auto guid = std::array<unsigned char, 16>{};
      reader.read(guid.data(), guid.size());
      const auto generationCount = reader.readInt<int32_t>();
      for (int32_t i = 0; i < generationCount; ++i)
      {
        reader.readInt<int32_t>(); // export count
        reader.readInt<int32_t>(); // name count
      }
    }

    reader.seekFromBegin(size_t(nameOffset));
    pkg.names.reserve(size_t(nameCount));
    for (int32_t i = 0; i < nameCount; ++i)
    {
      auto name = pkg.fileVersion < 64 ? readUnrealAnsiCString(reader)
                                        : readUnrealString(reader);
      reader.readUnsignedInt<uint32_t>(); // object flags, discarded
      pkg.names.push_back(std::move(name));
    }

    reader.seekFromBegin(size_t(importOffset));
    pkg.imports.reserve(size_t(importCount));
    for (int32_t i = 0; i < importCount; ++i)
    {
      auto classPackage = readFName(pkg.names, reader);
      auto className = readFName(pkg.names, reader);
      const auto packageIndex = reader.readInt<int32_t>();
      auto objectName = readFName(pkg.names, reader);
      pkg.imports.push_back(UnrealObjectImport{
        std::move(classPackage), std::move(className), packageIndex, std::move(objectName)});
    }

    reader.seekFromBegin(size_t(exportOffset));
    pkg.exports.reserve(size_t(exportCount));
    for (int32_t i = 0; i < exportCount; ++i)
    {
      const auto classIndex = readUnrealCompactIndex(reader);
      const auto superIndex = readUnrealCompactIndex(reader);
      const auto packageIndex = reader.readInt<int32_t>();
      auto objectName = readFName(pkg.names, reader);
      const auto objectFlags = reader.readUnsignedInt<uint32_t>();
      const auto serialSize = readUnrealCompactIndex(reader);
      const auto serialOffset = serialSize != 0 ? readUnrealCompactIndex(reader) : 0;
      pkg.exports.push_back(UnrealObjectExport{
        classIndex,
        superIndex,
        packageIndex,
        std::move(objectName),
        objectFlags,
        serialSize,
        serialOffset});
    }

    return pkg;
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::mdl
