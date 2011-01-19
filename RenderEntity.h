//
//  RenderEntity.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RenderContext.h"

@class Entity;

@interface RenderEntity : NSObject {
    Entity* entity;
    NSMutableDictionary* renderBrushes;
}

- (id)initWithEntity:(Entity *)anEntity;

- (void)renderWithContext:(id <RenderContext>)renderContext;
@end
