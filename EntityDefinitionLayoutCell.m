//
//  EntityDefinitionLayoutCell.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinitionLayoutCell.h"
#import "EntityDefinition.h"

@implementation EntityDefinitionLayoutCell

- (id)initWithEntityDefinition:(EntityDefinition *)theEntityDefinition atPos:(NSPoint)thePos width:(float)theWidth nameSize:(NSSize)theNameSize {
    if ((self = [self init])) {
        entityDefinition = [theEntityDefinition retain];
        entityDefinitionBounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth);
        
        nameBounds = NSMakeRect(thePos.x + (theWidth - theNameSize.width) / 2, NSMaxY(entityDefinitionBounds), theNameSize.width, theNameSize.height);
        bounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth + theNameSize.height);
    }
    
    return self;
}

- (void)dealloc {
    [entityDefinition release];
    [super dealloc];
}

- (EntityDefinition *)entityDefinition {
    return entityDefinition;
}

- (NSRect)entityDefinitionBounds {
    return entityDefinitionBounds;
}

- (NSRect)nameBounds {
    return nameBounds;
}

- (NSRect)bounds {
    return bounds;
}

@end
