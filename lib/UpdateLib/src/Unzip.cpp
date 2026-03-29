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

#include "update/Unzip.h"

#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>

#include "update/FileUtils.h"
#include "update/Logging.h"

#include <miniz/miniz.h>

namespace upd
{

namespace
{

QString zipErrorString(mz_zip_archive& archive)
{
  const auto err = mz_zip_get_last_error(&archive);
  const auto* message = mz_zip_get_error_string(err);
  return QString::fromUtf8(message != nullptr ? message : "Unknown miniz error");
}

QString normalizePathForComparison(const QString& path)
{
  return QDir::fromNativeSeparators(QDir::cleanPath(path));
}

bool isPathInsideDestination(const QString& destinationRoot, const QString& candidatePath)
{
  const auto normalizedRoot = normalizePathForComparison(destinationRoot);
  const auto normalizedCandidate = normalizePathForComparison(candidatePath);
  const auto relativePath = QDir{normalizedRoot}.relativeFilePath(normalizedCandidate);
  return !QDir::isAbsolutePath(relativePath) && relativePath != ".."
         && !relativePath.startsWith("../");
}

#if defined(Q_OS_UNIX)
std::optional<QFileDevice::Permissions> zipEntryPermissions(
  const mz_zip_archive_file_stat& fileStat)
{
  const auto unixMode =
    static_cast<unsigned int>((fileStat.m_external_attr >> 16) & 0xFFFFu);
  if ((unixMode & 0777u) == 0u)
  {
    return std::nullopt;
  }

  auto permissions = QFileDevice::Permissions{};
  if ((unixMode & 0400u) != 0u)
  {
    permissions |= QFileDevice::ReadOwner;
  }
  if ((unixMode & 0200u) != 0u)
  {
    permissions |= QFileDevice::WriteOwner;
  }
  if ((unixMode & 0100u) != 0u)
  {
    permissions |= QFileDevice::ExeOwner;
  }
  if ((unixMode & 0040u) != 0u)
  {
    permissions |= QFileDevice::ReadGroup;
  }
  if ((unixMode & 0020u) != 0u)
  {
    permissions |= QFileDevice::WriteGroup;
  }
  if ((unixMode & 0010u) != 0u)
  {
    permissions |= QFileDevice::ExeGroup;
  }
  if ((unixMode & 0004u) != 0u)
  {
    permissions |= QFileDevice::ReadOther;
  }
  if ((unixMode & 0002u) != 0u)
  {
    permissions |= QFileDevice::WriteOther;
  }
  if ((unixMode & 0001u) != 0u)
  {
    permissions |= QFileDevice::ExeOther;
  }

  return permissions;
}

bool applyZipEntryPermissions(
  const QString& path,
  const mz_zip_archive_file_stat& fileStat,
  const std::optional<QString>& logFilePath)
{
  const auto permissions = zipEntryPermissions(fileStat);
  if (!permissions)
  {
    return true;
  }

  if (QFile::setPermissions(path, *permissions))
  {
    return true;
  }

  logToFile(
    logFilePath,
    QString{"Failed to unzip the archive: could not set permissions on '%1'"}.arg(path));
  return false;
}
#else
bool applyZipEntryPermissions(
  const QString&, const mz_zip_archive_file_stat&, const std::optional<QString>&)
{
  return true;
}
#endif

struct mz_zip_archive_file : public mz_zip_archive
{
  const std::optional<QString>& m_logFilePath;
  bool m_open = false;

  explicit mz_zip_archive_file(const std::optional<QString>& logFilePath)
    : m_logFilePath{logFilePath}
  {
    mz_zip_zero_struct(this);
  }

  bool open(const QString& zipPath)
  {
    close();

    const auto zipPathBytes = QFile::encodeName(zipPath);
    if (mz_zip_reader_init_file(this, zipPathBytes.constData(), 0) != MZ_TRUE)
    {
      m_open = false;
    }
    else
    {
      m_open = true;
    }

    return m_open;
  }

  void close()
  {
    if (m_open)
    {
      if (mz_zip_reader_end(this) != MZ_TRUE)
      {
        logToFile(
          m_logFilePath,
          QString{"Could not finalize zip reader (%1)"}.arg(zipErrorString(*this)));
      }
    }
  }

  ~mz_zip_archive_file() { close(); }
};

} // namespace

bool unzip(
  const QString& zipPath,
  const QString& destFolderPath,
  const std::optional<QString>& logFilePath)
{
  if (!QFileInfo{destFolderPath}.exists() && !QDir{destFolderPath}.mkpath("."))
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip the archive: %1 could not be created"}.arg(
        destFolderPath));
    return false;
  }

  if (!QFileInfo{destFolderPath}.isDir())
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip the archive: %1 is not a folder"}.arg(destFolderPath));
    return false;
  }

  auto archive = mz_zip_archive_file{logFilePath};
  if (!archive.open(zipPath))
  {
    logToFile(
      logFilePath,
      QString{"Failed to unzip the archive: could not open zip file '%1' (%2)"}
        .arg(zipPath)
        .arg(zipErrorString(archive)));
    return false;
  }

  const auto destinationRoot = QDir::cleanPath(QDir{destFolderPath}.absolutePath());
  const auto fileCount = mz_zip_reader_get_num_files(&archive);
  for (mz_uint i = 0; i < fileCount; ++i)
  {
    auto fileStat = mz_zip_archive_file_stat{};
    if (mz_zip_reader_file_stat(&archive, i, &fileStat) != MZ_TRUE)
    {
      logToFile(
        logFilePath,
        QString{"Failed to unzip the archive: could not read zip entry metadata at index "
                "%1 (%2)"}
          .arg(i)
          .arg(zipErrorString(archive)));
      return false;
    }

    const auto nameLength = mz_zip_reader_get_filename(&archive, i, nullptr, 0);
    if (nameLength == 0)
    {
      logToFile(
        logFilePath,
        QString{
          "Failed to unzip the archive: could not read zip entry name at index %1 (%2)"}
          .arg(i)
          .arg(zipErrorString(archive)));
      return false;
    }

    auto entryNameBuffer = QByteArray{};
    entryNameBuffer.resize(static_cast<qsizetype>(nameLength));
    mz_zip_reader_get_filename(&archive, i, entryNameBuffer.data(), nameLength);

    const auto rawEntryPath = QString::fromUtf8(entryNameBuffer.constData());
    const auto isDirectoryEntry = mz_zip_reader_is_file_a_directory(&archive, i)
                                  || rawEntryPath.endsWith('/')
                                  || rawEntryPath.endsWith('\\');

    auto entryPath = rawEntryPath;
    entryPath.replace('\\', '/');
    entryPath = QDir::cleanPath(entryPath);

    const auto invalidRelativePath = QDir::isAbsolutePath(entryPath) || entryPath == ".."
                                     || entryPath.startsWith("../")
                                     || entryPath.isEmpty();
    if (invalidRelativePath)
    {
      logToFile(
        logFilePath,
        QString{"Failed to unzip the archive: invalid zip entry path '%1'"}.arg(
          entryPath));
      return false;
    }

    const auto destinationPath =
      QDir::cleanPath(QDir{destinationRoot}.filePath(entryPath));
    if (!isPathInsideDestination(destinationRoot, destinationPath))
    {
      logToFile(
        logFilePath,
        QString{"Failed to unzip the archive: zip entry path escapes destination '%1'"}
          .arg(entryPath));
      return false;
    }

    if (isDirectoryEntry)
    {
      if (!QDir{}.mkpath(destinationPath))
      {
        logToFile(
          logFilePath,
          QString{"Failed to unzip the archive: could not create directory '%1'"}.arg(
            destinationPath));
        return false;
      }
      if (!applyZipEntryPermissions(destinationPath, fileStat, logFilePath))
      {
        return false;
      }
      continue;
    }

    const auto destinationInfo = QFileInfo{destinationPath};
    if (!QDir{}.mkpath(destinationInfo.path()))
    {
      logToFile(
        logFilePath,
        QString{"Failed to unzip the archive: could not create directory '%1'"}.arg(
          destinationInfo.path()));
      return false;
    }

    const auto destinationPathBytes = QFile::encodeName(destinationPath);
    if (
      mz_zip_reader_extract_to_file(&archive, i, destinationPathBytes.constData(), 0)
      != MZ_TRUE)
    {
      logToFile(
        logFilePath,
        QString{"Failed to unzip the archive: could not extract '%1' (%2)"}
          .arg(entryPath)
          .arg(zipErrorString(archive)));
      return false;
    }

    if (!applyZipEntryPermissions(destinationPath, fileStat, logFilePath))
    {
      return false;
    }
  }

  return true;
}

} // namespace upd
