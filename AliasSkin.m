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

#import "AliasSkin.h"


@implementation AliasSkin
- (id)initSingleSkin:(NSData *)thePicture width:(int)theWidth height:(int)theHeight {
    NSAssert(thePicture != nil, @"picture must not be nil");
    
    if ((self = [self init])) {
        pictures = [[NSArray alloc] initWithObjects:thePicture, nil];
        times = NULL;
        width = theWidth;
        height = theHeight;
    }
    
    return self;
}

- (id)initMultiSkin:(NSArray *)thePictures times:(float *)theTimes width:(int)theWidth height:(int)theHeight {
    NSAssert(thePictures != nil, @"picture array must not be nil");
    NSAssert(theTimes != NULL, @"time array must not be NULL");

    if ((self = [self init])) {
        pictures = [[NSArray alloc] initWithArray:thePictures];
        times = malloc([pictures count] * sizeof(float));
        memcpy(times, theTimes, [pictures count] * sizeof(float));
        width = theWidth;
        height = theHeight;
    }
    
    return self;
}

- (void)dealloc {
    if (times != NULL)
        free(times);
    [pictures release];
    [super dealloc];
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

- (NSData *)pictureAtIndex:(int)theIndex {
    NSAssert(theIndex >= 0 && theIndex < [self pictureCount], @"index out of bounds");
    return [pictures objectAtIndex:theIndex];
}

- (float)timeAtIndex:(int)theIndex {
    NSAssert([self pictureCount] > 1, @"only multi skins can have times");
    NSAssert(theIndex >= 0 && theIndex < [self pictureCount], @"index out of bounds");
    return times[theIndex];
}

- (int)pictureCount {
    return [pictures count];
}

@end
