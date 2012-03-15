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

#include <stdlib.h>
#include <stdio.h>
#import "WadIO.h"

FILE* initWadDirectory(TWadDirectory* wadDirectory, const char* path) {
    FILE* file = fopen(path, "r");
    if (file != NULL) {
        fseek(file, WAD_NUM_ENTRIES_ADDRESS, SEEK_SET);
        wadDirectory->entryCount = fscanf(file, "%ui");
        wadDirectory->entries = malloc(wadDirectory->entryCount * sizeof(TWadEntry));
        
        fseek(file, WAD_DIR_OFFSET_ADDRESS, SEEK_SET);
        unsigned int directoryAddress = fscanf(file, "%ui");
        fseek(file, directoryAddress, SEEK_SET);
        fread(wadDirectory->entries, sizeof(TWadEntry), wadDirectory->entryCount, file);
    }
        
    return file;
}

void freeWadDirectory(TWadDirectory* wadDirectory) {
    free(wadDirectory->entries);
    wadDirectory->entries = NULL;
    wadDirectory->entryCount = 0;
}

void loadMipEntry(TMipEntry* mipEntry, const TWadEntry* wadEntry, FILE* file) {
    fseek(file, wadEntry->address, SEEK_SET);
    fseek(file, WAD_TEX_WIDTH_OFFSET, SEEK_CUR);
    mipEntry->width = fscanf(file, "%ui");
    mipEntry->height = fscanf(file, "%ui");
    
    unsigned int mip0Addr = fscanf(file, "%ui");
    unsigned int mip1Addr = fscanf(file, "%ui");
    unsigned int mip2Addr = fscanf(file, "%ui");
    unsigned int mip3Addr = fscanf(file, "%ui");
    
    unsigned int mip0Size = mipEntry->width * mipEntry->height;
    unsigned int mip1Size = mip0Size / 4;
    unsigned int mip2Size = mip1Size / 4;
    unsigned int mip3Size = mip2Size / 4;

    mipEntry->mip0 = malloc(mip0Size);
    mipEntry->mip1 = malloc(mip1Size);
    mipEntry->mip2 = malloc(mip2Size);
    mipEntry->mip3 = malloc(mip3Size);
    
    fseek(file, wadEntry->address + mip0Addr, SEEK_SET);
    fread(mipEntry->mip0, sizeof(char), mip0Size, file);

    fseek(file, wadEntry->address + mip1Addr, SEEK_SET);
    fread(mipEntry->mip1, sizeof(char), mip1Size, file);
    
    fseek(file, wadEntry->address + mip2Addr, SEEK_SET);
    fread(mipEntry->mip2, sizeof(char), mip2Size, file);

    fseek(file, wadEntry->address + mip3Addr, SEEK_SET);
    fread(mipEntry->mip3, sizeof(char), mip3Size, file);
}

void loadPaletteEntry(TPaletteEntry* paletteEntry, const TWadEntry* wadEntry, FILE* file) {
    fseek(file, wadEntry->address, SEEK_SET);
    fread(paletteEntry->palette, sizeof(TRGB), WAD_PAL_LENGTH, file);
}
