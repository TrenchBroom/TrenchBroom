/*
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

@class GLStringData;
@class VBOBuffer;
@class VBOMemBlock;
@class IntData;

@interface GLString : NSObject {
    @private
    VBOMemBlock* memBlock;
    BOOL hasTriangleSet;
    int triangleSetIndex;
    int triangleSetCount;
    BOOL hasTriangleStrips;
    IntData* triangleStripIndices;
    IntData* triangleStripCounts;
    BOOL hasTriangleFans;
    IntData* triangleFanIndices;
    IntData* triangleFanCounts;
    NSSize size;
}

- (id)initWithVbo:(VBOBuffer *)theVbo data:(GLStringData *)theData size:(NSSize)theSize;

- (NSSize)size;

- (void)renderBackground;
- (void)render;
@end
