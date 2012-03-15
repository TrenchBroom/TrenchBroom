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

#ifndef TrenchBroom_WadIO_h
#define TrenchBroom_WadIO_h

#import "TrenchBroom.h"

unsigned int const WAD_NUM_ENTRIES_ADDRESS = 4;
unsigned int const WAD_DIR_OFFSET_ADDRESS = 8;
unsigned int const WAD_DIR_ENTRY_NAME_LENGTH = 16;
unsigned int const WAD_PAL_LENGTH = 256;
unsigned int const WAD_TEX_WIDTH_OFFSET = 16;

typedef enum {
    WT_MIP,
    WT_PAL
} EWadEntryType;

typedef struct {
    long address;
    long length;
    long size;
    char type;
    char compression;
    short dummy;
    char name[WAD_DIR_ENTRY_NAME_LENGTH]; 
} TWadEntry;

typedef struct TWadDirectoryTag {
    TWadEntry* entries;
    unsigned int entryCount;
} TWadDirectory;

typedef struct {
    char r;
    char g;
    char b;
} TRGB;

typedef struct {
    unsigned int width;
    unsigned int height;
    char* mip0;
    char* mip1;
    char* mip2;
    char* mip3;
} TMipEntry;

typedef struct {
    TRGB palette[WAD_PAL_LENGTH];
} TPaletteEntry;

FILE* initWadDirectory(TWadDirectory* wadDirectory, const char* path);
void freeWadDirectory(TWadDirectory* wadDirectory);
void loadMipEntry(TMipEntry* mipEntry, const TWadEntry* wadEntry, FILE* file);
void loadPaletteEntry(TPaletteEntry* paletteEntry, const TWadEntry* wadEntry, FILE* file);

#endif
