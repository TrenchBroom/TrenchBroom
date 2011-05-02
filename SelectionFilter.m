//
//  SelectionFilter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 17.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "SelectionFilter.h"
#import "SelectionManager.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

@implementation SelectionFilter

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager {
    if (self = [self init]) {
        selectionManager = [theSelectionManager retain];
    }
    
    return self;
}

- (BOOL)brushPasses:(id <Brush>)brush {
    if ([selectionManager mode] == SM_FACES) {
        NSEnumerator* faceEn = [[brush faces] objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject]))
            if ([selectionManager isFaceSelected:face])
                return YES;
        
        return NO;
    }
    
    if ([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_BRUSHES_ENTITIES) {
        return [selectionManager isBrushSelected:brush];
    }
    
    return NO;
}

- (BOOL)entityPasses:(id <Entity>)entity {
    return [selectionManager isEntitySelected:entity];
}

- (void)dealloc {
    [selectionManager release];
    [super dealloc];
}

@end
