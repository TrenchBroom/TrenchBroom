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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ZipFileSystem.h"

// FIXME: port to something non-wx
#if 0
#include "CollectionUtils.h"
#include "IO/CharArrayReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"

#include <cassert>
#include <cstring>
#include <memory>

#include <wx/log.h>
#include <wx/mstream.h>
#include <wx/zipstrm.h>

namespace TrenchBroom {
    namespace IO {
        ZipFileSystem::ZipCompressedFile::ZipCompressedFile(std::shared_ptr<wxZipInputStream> stream, std::unique_ptr<wxZipEntry> entry) :
        m_stream(std::move(stream)),
        m_entry(std::move(entry)) {}

        MappedFile::Ptr ZipFileSystem::ZipCompressedFile::doOpen() const {
            const auto path = Path(m_entry->GetName().ToStdString());

            if (!m_stream->OpenEntry(*m_entry)) {
                throw FileSystemException("Could not open zip entry at " + path.asString());
            }

            if (!m_stream->CanRead()) {
                throw FileSystemException("Could not read zip entry at " + path.asString());
            }

            const auto uncompressedSize = static_cast<size_t>(m_entry->GetSize());
            auto data = std::make_unique<char[]>(uncompressedSize);
            auto* begin = data.get();

            m_stream->Read(begin, uncompressedSize);
            m_stream->CloseEntry();

            return std::make_shared<MappedFileBuffer>(path, std::move(data), uncompressedSize);
        }

        ZipFileSystem::ZipFileSystem(const Path& path, MappedFile::Ptr file) :
        ZipFileSystem(nullptr, path, std::move(file)) {}

        ZipFileSystem::ZipFileSystem(std::shared_ptr<FileSystem> next, const Path& path, MappedFile::Ptr file) :
        ImageFileSystem(std::move(next), path, std::move(file)) {
            initialize();
        }

        void ZipFileSystem::doReadDirectory() {
            // wxZipInputStream uses wxLogError which will pop up a dialog!
            wxLogNull disableLogging;

            auto stream = std::make_shared<wxZipInputStream>(new wxMemoryInputStream(m_file->begin(), m_file->size()));
            for (int i = 0; i < stream->GetTotalEntries(); ++i) {
                auto entry = std::unique_ptr<wxZipEntry>(stream->GetNextEntry());
                if (!entry->IsDir()) {
                    const auto path = Path(entry->GetName().ToStdString());
                    m_root.addFile(path, std::make_unique<ZipCompressedFile>(stream, std::move(entry)));
                }
            }

            if (stream->GetLastError() != wxSTREAM_NO_ERROR) {
                throw FileSystemException("Error while reading compressed file");
            }
        }
    }
}
#endif
