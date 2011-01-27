//
//  RenderMap.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RenderContext.h"

@class Map;
@class RenderEntity;
@class Entity;
@class VBOBuffer;

@interface RenderMap : NSObject {
    Map* map;
    VBOBuffer* vboBuffer;
    NSMutableDictionary* renderEntities;
}

- (id)initWithMap:(Map *)theMap vboBuffer:(VBOBuffer *)theVboBuffer;

- (void)renderWithContext:(id <RenderContext>)renderContext;

@end
