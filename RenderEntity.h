//
//  RenderEntity.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Entity;

@interface RenderEntity : NSObject {
    Entity* entity;
    NSMutableDictionary* renderBrushes;
}

- (id)initWithEntity:(Entity *)anEntity;

- (void)brushAdded:(NSNotification *)notification;
- (void)brushRemoved:(NSNotification *)notification;

- (void)propertyAdded:(NSNotification *)notification;
- (void)propertyRemoved:(NSNotification *)notification;
- (void)propertyChanged:(NSNotification *)notification;
@end
