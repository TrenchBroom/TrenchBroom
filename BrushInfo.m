//
//  BrushInfo.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
        for (int i = 0; i < 3; i++)
            flatColor[i] = [theBrush flatColor][i];
        
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
}

@end
