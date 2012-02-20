/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "EntityDefinitionLayoutCell.h"
#import "EntityDefinition.h"
#import "GLString.h"

@implementation EntityDefinitionLayoutCell

- (id)initWithEntityDefinition:(EntityDefinition *)theEntityDefinition atPos:(NSPoint)thePos width:(float)theWidth name:(GLString *)theName {
    if ((self = [self init])) {
        entityDefinition = [theEntityDefinition retain];
        entityDefinitionBounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth);
        
        name = [theName retain];
        
        NSSize nameSize = [name size];
        nameBounds = NSMakeRect(thePos.x + (theWidth - nameSize.width) / 2, NSMaxY(entityDefinitionBounds), nameSize.width, nameSize.height);
        bounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth + nameSize.height);
    }
    
    return self;
}

- (void)dealloc {
    [entityDefinition release];
    [name release];
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

- (GLString *)name {
    return name;
}

- (NSRect)bounds {
    return bounds;
}

@end
