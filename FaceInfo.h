//
//  FaceInfo.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"

@class MutableFace;
@protocol Face;

@interface FaceInfo : NSObject {
@private
    NSNumber* faceId;
    TVector3i point1;
    TVector3i point2;
    TVector3i point3;
    int xOffset;
    int yOffset;
    float xScale;
    float yScale;
    float rotation;
    NSString* texture;
}

+ (id)faceInfoFor:(id <Face>)theFace;

- (id)initWithFace:(id <Face>)theFace;

- (void)updateFace:(MutableFace *)theFace;

@end
