//
//  RenderEntity.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RenderMap;
@class Entity;
@class VBOBuffer;

@interface RenderEntity : NSObject {
    RenderMap* renderMap;
    Entity* entity;
    NSMutableDictionary* renderBrushes;
    VBOBuffer* faceVBO;
}

- (id)initInMap:(RenderMap *)theRenderMap withEntity:(Entity *)theEntity faceVBO:(VBOBuffer *)theFaceVBO;

- (NSArray *)renderBrushes;

- (void)brushChanged;
@end
