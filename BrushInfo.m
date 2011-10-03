/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "BrushInfo.h"
#import "MutableBrush.h"
#import "MutableFace.h"
#import "Brush.h"
#import "Face.h"
#import "FaceInfo.h"

@implementation BrushInfo

+ (id)brushInfoFor:(id <Brush>)theBrush {
    return [[[BrushInfo alloc] initWithBrush:theBrush] autorelease];
}

- (id)initWithBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    if ((self = [self init])) {
        brushId = [[theBrush brushId] retain];
        
        NSArray* faces = [theBrush faces];
        faceInfos = [[NSMutableArray alloc] initWithCapacity:[faces count]];

        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            FaceInfo* faceInfo = [[FaceInfo alloc] initWithFace:face];
            [faceInfos addObject:faceInfo];
            [faceInfo release];
        }
    }
    
    return self;
}

- (void)updateBrush:(MutableBrush *)theBrush {
    NSAssert([brushId isEqualToNumber:[theBrush brushId]], @"brush id must be equal");
    
    NSArray* faces = [theBrush faces];
    NSAssert([faces count] == [faceInfos count], @"face info count must be same as face count");
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    NSEnumerator* faceInfoEn = [faceInfos objectEnumerator];
    id <Face> face;
    FaceInfo* faceInfo;
    while ((face = [faceEn nextObject]) && (faceInfo = [faceInfoEn nextObject]))
        [faceInfo updateFace:(MutableFace *)face];

    [theBrush invalidateVertexData];
}

@end
