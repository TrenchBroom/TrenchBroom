//
//  Bsp.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"
#import "BspTexture.h"

typedef struct {
    TVector3f sAxis;
    TVector3f tAxis;
    float sOffset;
    float tOffset;
    BspTexture* texture;
} TTextureInfo;

typedef struct {
    int vertex0;
    int vertex1;
} TEdge;

typedef struct {
    int edgeIndex;
    int edgeCount;
    int textureInfoIndex;
} TFace;

@interface Bsp : NSObject {
@private
    NSString* name;
    NSMutableArray* models;
    NSMutableArray* textures;
}

- (id)initWithName:(NSString *)theName data:(NSData *)theData;

- (NSString *)name;
- (NSArray *)models;

@end
