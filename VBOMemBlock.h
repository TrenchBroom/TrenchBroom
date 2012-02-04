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

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    BS_FREE, // memory block is free
    BS_USED_INVALID, // memory block is in use, but invalid
    BS_USED_VALID // memory block is in use and valid
} EVBOMemBlockState;

@class VBOBuffer;

@interface VBOMemBlock : NSObject {
    @private
    int address;
    int capacity;
    EVBOMemBlockState state;
    VBOMemBlock* next;
    VBOMemBlock* previous;
    VBOBuffer* vboBuffer;
}

- (id)initBlockIn:(VBOBuffer *)theVboBuffer at:(int)theAddress capacity:(int)theCapacity;

- (int)address;
- (void)setAddress:(int)theAddress;
- (int)capacity;
- (EVBOMemBlockState)state;
- (VBOBuffer *)vbo;
                                                                       
- (void)setCapacity:(int)aSize;
- (void)setState:(EVBOMemBlockState)theState;

- (void)activate;
- (void)deactivate;

- (VBOMemBlock *)previous;
- (VBOMemBlock *)next;

- (void)setPrevious:(VBOMemBlock *)memBlock;
- (void)setNext:(VBOMemBlock *)memBlock;

- (void)insertBetweenPrevious:(VBOMemBlock *)previousBlock next:(VBOMemBlock *)nextBlock;
- (void)remove;

- (void)free;
@end
