/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "Wad.h"

#include "IO/IOUtils.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        namespace WadLayout {
            static const unsigned int NumEntriesAddress     = 4;
            static const unsigned int DirOffsetAddress      = 8;
            static const unsigned int DirEntryTypeOffset    = 4;
            static const unsigned int DirEntryNameOffset    = 3;
            static const unsigned int DirEntryNameLength    = 16;
            static const unsigned int PalLength             = 256;
            static const unsigned int TexWidthOffset        = 16;
            static const unsigned int MaxTextureSize        = 512;
        }

        Mip* Wad::loadMip(const WadEntry& entry, unsigned int mipCount) const throw (IOException) {
            if (m_stream.eof())
                throw IOException::unexpectedEof();
            if (entry.type() != WadEntryType::WEMip)
                throw IOException("Entry %s is not a mip", entry.name().c_str());
            
            m_stream.seekg(entry.address(), std::ios::beg);
            m_stream.seekg(WadLayout::TexWidthOffset, std::ios::cur);
            unsigned int width = IO::readUnsignedInt<int32_t>(m_stream);
            unsigned int height = IO::readUnsignedInt<int32_t>(m_stream);
            unsigned int mip0Size = width * height;
            unsigned int mip0Offset = IO::readUnsignedInt<int32_t>(m_stream);
            
            if (width == 0 || height == 0 ||
                width > WadLayout::MaxTextureSize || height > WadLayout::MaxTextureSize)
                throw IOException("Invalid mip dimensions (%ix%i)", width, height);
            if (mip0Offset + mip0Size > entry.length())
                throw IOException("Mip data beyond wad entry");
            
            unsigned char* mip0 = NULL;
            if (mipCount > 0) {
                mip0 = new unsigned char[mip0Size];
                m_stream.seekg(entry.address() + mip0Offset, std::ios::beg);
                m_stream.read(reinterpret_cast<char *>(mip0), static_cast<std::streamsize>(mip0Size));
            }
            
            return new Mip(entry.name(), static_cast<unsigned int>(width), static_cast<unsigned int>(height), mip0);
        }

        Wad::Wad(const String& path) throw (IOException) :
        m_stream(path.c_str(), std::ios::binary | std::ios::in) {
            if (!m_stream.is_open())
                throw IOException::openError();
            
            if (!m_stream.good())
                throw IOException::badStream(m_stream);
            
            m_stream.seekg(0, std::ios::end);
            m_length = static_cast<size_t>(m_stream.tellg());
            m_stream.seekg(0, std::ios::beg);
            
			m_stream.exceptions(std::ios::failbit | std::ios::badbit);
            
            m_stream.seekg(WadLayout::NumEntriesAddress, std::ios::beg);
            unsigned int entryCount = IO::readUnsignedInt<int32_t>(m_stream);
            m_stream.seekg(WadLayout::DirOffsetAddress, std::ios::beg);
            unsigned int directoryAddr = IO::readUnsignedInt<int32_t>(m_stream);

            if (directoryAddr > m_length)
                throw IOException("Wad directory beyond end of file");

            char entryType;
            char entryName[WadLayout::DirEntryNameLength];
            m_stream.seekg(directoryAddr, std::ios::beg);
            
            for (unsigned int i = 0; i < entryCount; i++) {
                if (m_stream.eof())
                    throw IOException::unexpectedEof();
                
                unsigned int entryAddress = IO::readUnsignedInt<int32_t>(m_stream);
                unsigned int entryLength = IO::readUnsignedInt<int32_t>(m_stream);
                
                if (entryAddress + entryLength > m_length)
                    throw IOException("Wad entry beyond end of file");
                
                m_stream.seekg(WadLayout::DirEntryTypeOffset, std::ios::cur);
                m_stream.read(&entryType, 1);
                m_stream.seekg(WadLayout::DirEntryNameOffset, std::ios::cur);
                m_stream.read(reinterpret_cast<char *>(entryName), WadLayout::DirEntryNameLength);
                
                // might leak if there are duplicate entries
                m_entries[entryName] = WadEntry(entryAddress, entryLength, entryType, entryName);
            }
            
			m_stream.clear();
        }
        
        Mip* Wad::loadMip(const String& name, unsigned int mipCount) const throw (IOException) {
            EntryMap::const_iterator it = m_entries.find(name);
            if (it == m_entries.end())
                throw IOException("Wad entry %s not found", name.c_str());
            return loadMip(it->second, mipCount);
        }

        Mip::List Wad::loadMips(unsigned int mipCount) const throw (IOException) {
            Mip::List mips;
            EntryMap::const_iterator it, end;
            for (it = m_entries.begin(), end = m_entries.end(); it != end; ++it) {
                const WadEntry& entry = it->second;
                if (entry.type() == WadEntryType::WEMip) {
                    Mip* mip = loadMip(entry, mipCount);
                    if (mip != NULL)
                        mips.push_back(mip);
                }
            }
            
            return mips;
        }
    }
}
