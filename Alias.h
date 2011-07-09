//
//  EntityModel.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class AliasFrame;
@class AliasSkin;

typedef struct {
    BOOL onseam;
    int s;
    int t;
} TSkinVertex;

typedef struct {
    BOOL front;
    int vertices[3];
} TSkinTriangle;

typedef struct {
    TVector3f position;
    TVector2f texCoords;
    TVector3f norm;
} TFrameVertex;

typedef struct {
    int x,y,z,i;
} TPackedFrameVertex;

typedef struct {
    TFrameVertex vertices[3];
} TFrameTriangle;

@interface Alias : NSObject {
    NSString* name;
    NSMutableArray* frames;
    NSMutableArray* skins;
}

- (id)initWithName:(NSString *)theName data:(NSData *)theData;

- (NSString *)name;
- (AliasFrame *)firstFrame;

- (AliasSkin *)firstSkin;
- (AliasSkin *)skinWithIndex:(int)theSkinIndex;

@end
