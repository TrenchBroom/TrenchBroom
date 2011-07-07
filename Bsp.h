//
//  Bsp.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"
#import "Texture.h"

typedef struct {
    TVector3f sAxis;
    TVector3f tAxis;
    float sOffset;
    float tOffset;
    Texture* texture;
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
}

- (id)initWithName:(NSString *)theName data:(NSData *)theData palette:(NSData *)thePalette;

- (NSString *)name;
- (NSArray *)models;

@end
