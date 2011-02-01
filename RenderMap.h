//
//  RenderMap.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Map;
@class VBOBuffer;

@interface RenderMap : NSObject {
    Map* map;
    VBOBuffer* faceVBO;
    NSMutableDictionary* renderEntities;
}

- (id)initWithMap:(Map *)theMap faceVBO:(VBOBuffer *)theFaceVBO;

- (NSArray *)renderEntities;

@end
