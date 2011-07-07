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
    TVector3f* vertex0;
    TVector3f* vertex1;
} TEdge;

@interface Bsp : NSObject {
@private
    NSMutableArray* models;
}

- (id)initWithData:(NSData *)theData palette:(NSData *)thePalette;

@end
