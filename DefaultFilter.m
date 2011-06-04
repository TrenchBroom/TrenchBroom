//
//  SelectionFilter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 17.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultFilter.h"
#import "SelectionManager.h"
#import "Options.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

@implementation DefaultFilter

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager options:(Options *)theOptions {
    if (self = [self init]) {
        selectionManager = [theSelectionManager retain];
        options = [theOptions retain];
    }
    
    return self;
}

- (BOOL)brushPasses:(id <Brush>)brush {
    if (![options renderBrushes])
        return NO;
    
    if ([options isolationMode] == IM_NONE)
        return YES;
    
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
    if (![options renderEntities])
        return NO;
    
    if ([options isolationMode] == IM_NONE)
        return YES;
    
    return [selectionManager isEntitySelected:entity];
}

- (void)dealloc {
    [selectionManager release];
    [options release];
    [super dealloc];
}

@end
