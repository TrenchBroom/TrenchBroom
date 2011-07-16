//
//  EntityDefinitionLayoutCell.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinitionLayoutCell.h"
#import "EntityDefinition.h"
#import "GLString.h"

@implementation EntityDefinitionLayoutCell

- (id)initWithEntityDefinition:(EntityDefinition *)theEntityDefinition atPos:(NSPoint)thePos width:(float)theWidth nameString:(GLString *)theNameString {
    if ((self = [self init])) {
        entityDefinition = [theEntityDefinition retain];
        entityDefinitionBounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth);
        
        nameString = [theNameString retain];
        
        NSSize nameSize = [nameString size];
        nameBounds = NSMakeRect(thePos.x + (theWidth - nameSize.width) / 2, NSMaxY(entityDefinitionBounds), nameSize.width, nameSize.height);
        bounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth + nameSize.height);
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

- (GLString *)nameString {
    return nameString;
}

- (NSRect)bounds {
    return bounds;
}

@end
