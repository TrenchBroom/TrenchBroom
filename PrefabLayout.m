//
//  PrefabLayout.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PrefabLayout.h"
#import "PrefabManager.h"
#import "PrefabGroup.h"
#import "PrefabLayoutGroupRow.h"
#import "GLFont.h"

@implementation PrefabLayout

- (id)init {
    if (self = [super init]) {
        groupRows = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithPrefabManager:(PrefabManager *)thePrefabManager prefabsPerRow:(int)thePrefabsPerRow glFont:(GLFont *)theGLFont {
    if (thePrefabManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"prefab manager must not be nil"];
    
    if (self = [self init]) {
        prefabManager = [thePrefabManager retain];
        prefabsPerRow = thePrefabsPerRow;
        glFont = [theGLFont retain];
        outerMargin = 10;
        innerMargin = 5;
        groupMargin = 10;
    }
    
    return self;
}

- (void)validate {
    [groupRows removeAllObjects];
    
    NSEnumerator* groupEn = [[prefabManager prefabGroups] objectEnumerator];
    id <PrefabGroup> group;
    int y = outerMargin;
    
    while ((group = [groupEn nextObject])) {
        PrefabLayoutGroupRow* groupRow = [[PrefabLayoutGroupRow alloc] initWithPrefabGroup:group prefabsPerRow:prefabsPerRow glFont:glFont atPos:NSMakePoint(outerMargin, y) width:width - 2 * outerMargin innerMargin:innerMargin];
        [groupRows addObject:groupRow];
        y += NSHeight([groupRow bounds]) + groupMargin;
        [groupRow release];
    }
    
    height = y - groupMargin + outerMargin;
    valid = YES;
}

- (GLFont *)glFont {
    return glFont;
}

- (NSArray *)groupRows {
    if (!valid)
        [self validate];
    
    return groupRows;
}

- (float)height {
    if (!valid)
        [self validate];
    
    return height;
}

- (id <Prefab>)prefabAt:(NSPoint)pos {
    if (!valid)
        [self validate];
    
    NSEnumerator* groupRowEn = [groupRows objectEnumerator];
    PrefabLayoutGroupRow* groupRow;
    while ((groupRow = [groupRowEn nextObject]))
        if (NSPointInRect(pos, [groupRow bounds]))
            return [groupRow prefabAt:(NSPoint)pos];
    
    return nil;
}

- (void)setPrefabsPerRow:(int)thePrefabsPerRow {
    prefabsPerRow = thePrefabsPerRow;
    [self invalidate];
}

- (void)setWidth:(float)theWidth {
    width = theWidth;
    [self invalidate];
}

- (void)invalidate {
    valid = NO;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [groupRows release];
    [prefabManager release];
    [glFont release];
    [super dealloc];
}
@end
