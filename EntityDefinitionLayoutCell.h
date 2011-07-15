//
//  EntityDefinitionLayoutCell.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class EntityDefinition;

@interface EntityDefinitionLayoutCell : NSObject {
@private
    EntityDefinition* entityDefinition;
    NSRect entityDefinitionBounds;
    NSRect nameBounds;
    NSRect bounds;
}

- (id)initWithEntityDefinition:(EntityDefinition *)theEntityDefinition atPos:(NSPoint)thePos width:(float)theWidth nameSize:(NSSize)theNameSize;

- (EntityDefinition *)entityDefinition;
- (NSRect)entityDefinitionBounds;
- (NSRect)nameBounds;
- (NSRect)bounds;

@end
