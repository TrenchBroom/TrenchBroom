/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "ImageLoaderImpl.h"

#include "Exceptions.h"
#include "Macros.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        ImageLoaderImpl::InitFreeImage::InitFreeImage() {
            FreeImage_Initialise(true);
        }
        
        ImageLoaderImpl::InitFreeImage::~InitFreeImage() {
            FreeImage_DeInitialise();
        }
        
        ImageLoaderImpl::ImageLoaderImpl(const ImageLoader::Format format, const Path& path) :
        m_stream(NULL),
        m_bitmap(NULL),
        m_paletteInitialized(false),
        m_indicesInitialized(false),
        m_pixelsInitialized(false) {
            initialize();
            const FREE_IMAGE_FORMAT fifFormat = translateFormat(format);
            if (fifFormat == FIF_UNKNOWN)
                throw FileFormatException("Unknown image format");
            
            m_bitmap = FreeImage_Load(fifFormat, path.asString().c_str());
        }
        
        ImageLoaderImpl::ImageLoaderImpl(const ImageLoader::Format format, const char* begin, const char* end) :
        m_stream(NULL),
        m_bitmap(NULL),
        m_paletteInitialized(false),
        m_indicesInitialized(false),
        m_pixelsInitialized(false) {
            initialize();
            const FREE_IMAGE_FORMAT fifFormat = translateFormat(format);
            if (fifFormat == FIF_UNKNOWN)
                throw FileFormatException("Unknown image format");
            
            // this is supremely evil, but FreeImage guarantees that it will not modify wrapped memory
            BYTE* address = reinterpret_cast<BYTE*>(const_cast<char*>(begin));
            DWORD length = static_cast<DWORD>(end - begin);
            m_stream = FreeImage_OpenMemory(address, length);
            m_bitmap = FreeImage_LoadFromMemory(fifFormat, m_stream);
        }
        
        ImageLoaderImpl::~ImageLoaderImpl() {
            if (m_bitmap != NULL) {
                FreeImage_Unload(m_bitmap);
                m_bitmap = NULL;
            }
            if (m_stream != NULL) {
                FreeImage_CloseMemory(m_stream);
                m_stream = NULL;
            }
        }

        size_t ImageLoaderImpl::paletteSize() const {
            return static_cast<size_t>(FreeImage_GetColorsUsed(m_bitmap));
        }
        
        size_t ImageLoaderImpl::bitsPerPixel() const {
            return static_cast<size_t>(FreeImage_GetBPP(m_bitmap));
        }
        
        size_t ImageLoaderImpl::width() const {
            return static_cast<size_t>(FreeImage_GetWidth(m_bitmap));
        }
        
        size_t ImageLoaderImpl::height() const {
            return static_cast<size_t>(FreeImage_GetHeight(m_bitmap));
        }
        
        size_t ImageLoaderImpl::byteWidth() const {
            return static_cast<size_t>(FreeImage_GetLine(m_bitmap));
        }
        
        size_t ImageLoaderImpl::scanWidth() const {
            return static_cast<size_t>(FreeImage_GetPitch(m_bitmap));
        }
        
        bool ImageLoaderImpl::hasPalette() const {
            return FreeImage_GetPalette(m_bitmap) != NULL;
        }
        
        bool ImageLoaderImpl::hasIndices() const {
            return FreeImage_GetColorType(m_bitmap) == FIC_PALETTE;
        }
        
        bool ImageLoaderImpl::hasPixels() const {
            return static_cast<bool>(FreeImage_HasPixels(m_bitmap) == TRUE);
        }
        
        const Buffer<unsigned char>& ImageLoaderImpl::palette() const {
            assert(hasPalette());
            if (!m_paletteInitialized) {
                const RGBQUAD* pal = FreeImage_GetPalette(m_bitmap);
                if (pal != NULL) {
                    m_palette = Buffer<unsigned char>(paletteSize() * 3);
                    for (size_t i = 0; i < paletteSize(); ++i) {
                        m_palette[i * 3 + 0] = static_cast<unsigned char>(pal[i].rgbRed);
                        m_palette[i * 3 + 1] = static_cast<unsigned char>(pal[i].rgbGreen);
                        m_palette[i * 3 + 2] = static_cast<unsigned char>(pal[i].rgbBlue);
                    }
                    
                    m_paletteInitialized = true;
                }
            }
            
            return m_palette;
        }
        
        const Buffer<unsigned char>& ImageLoaderImpl::indices() const {
            assert(hasIndices());
            if (!m_indicesInitialized) {
                m_indices = Buffer<unsigned char>(width() * height());
                for (unsigned y = 0; y < height(); ++y) {
                    for (unsigned x = 0; x < width(); ++x) {
                        BYTE index = 0;
                        const bool success = (FreeImage_GetPixelIndex(m_bitmap, x, y, &index) == TRUE);
                        assert(success);
                        unused(success);
                        m_indices[(height() - y - 1) * width() + x] = static_cast<unsigned char>(index);
                    }
                }
                
                m_indicesInitialized = true;
            }
            
            return m_indices;
        }
        
        const Buffer<unsigned char>& ImageLoaderImpl::pixels(const ImageLoader::PixelFormat format) const {
            assert(hasPixels());
            if (!m_pixelsInitialized) {
                const size_t pSize = pixelSize(format);
                m_pixels = Buffer<unsigned char>(width() * height() * pSize);
                if (hasIndices())
                    initializeIndexedPixels(pSize);
                else
                    initializePixels(pSize);
                m_pixelsInitialized = true;
            }
            
            return m_pixels;
        }

        void ImageLoaderImpl::initialize() {
            static InitFreeImage initFreeImage;
        }

        void ImageLoaderImpl::initializeIndexedPixels(const size_t pSize) const {
            assert(pSize == 3);
            const RGBQUAD* pal = FreeImage_GetPalette(m_bitmap);
            assert(pal != NULL);
            
            for (unsigned y = 0; y < height(); ++y) {
                for (unsigned x = 0; x < width(); ++x) {
                    BYTE paletteIndex = 0;
                    const bool success = (FreeImage_GetPixelIndex(m_bitmap, x, y, &paletteIndex) == TRUE);
                    assert(success);
                    unused(success);

                    assert(paletteIndex < paletteSize());

                    const size_t pixelIndex = ((height() - y - 1) * width() + x) * pSize;
                    m_pixels[pixelIndex + 0] = static_cast<unsigned char>(pal[paletteIndex].rgbRed);
                    m_pixels[pixelIndex + 1] = static_cast<unsigned char>(pal[paletteIndex].rgbGreen);
                    m_pixels[pixelIndex + 2] = static_cast<unsigned char>(pal[paletteIndex].rgbBlue);
                }
            }
        }
        
        void ImageLoaderImpl::initializePixels(const size_t pSize) const {
            for (unsigned y = 0; y < height(); ++y) {
                for (unsigned x = 0; x < width(); ++x) {
                    RGBQUAD pixel;
                    const bool success = (FreeImage_GetPixelColor(m_bitmap, x, y, &pixel) == TRUE);
                    assert(success);
                    unused(success);

                    const size_t pixelIndex = ((height() - y - 1) * width() + x) * pSize;
                    m_pixels[pixelIndex + 0] = static_cast<unsigned char>(pixel.rgbRed);
                    m_pixels[pixelIndex + 1] = static_cast<unsigned char>(pixel.rgbGreen);
                    m_pixels[pixelIndex + 2] = static_cast<unsigned char>(pixel.rgbBlue);
                    if (pSize > 3)
                        m_pixels[pixelIndex + 3] = static_cast<unsigned char>(pixel.rgbReserved);
                }
            }
        }

        FREE_IMAGE_FORMAT ImageLoaderImpl::translateFormat(const ImageLoader::Format format) {
            switch (format) {
                case ImageLoader::PCX:
                    return FIF_PCX;
                switchDefault()
            }
        }

        size_t ImageLoaderImpl::pixelSize(const ImageLoader::PixelFormat format) {
            switch (format) {
                case ImageLoader::RGB:
                    return 3;
                case ImageLoader::RGBA:
                    return 4;
                switchDefault()
            }
        }
    }
}
