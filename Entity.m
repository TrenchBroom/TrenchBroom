//
//  Entitiy.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Entity.h"

@implementation Entity

- (id)initWithKey:(NSString *)key value:(NSString *)value {
	if (self = [super init]) {
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableSet alloc] init];
		
		[properties setObject:value forKey:key];
	}
	
	return self;
}

- (Brush *)createCuboidAt:(Vector3i *)position with:(Vector3i *)dimensions {
    Brush* brush = [[Brush alloc] initCuboidAt:position with:dimensions];
    [brushes addObject:brush];
    
    return brush;
}

- (void)addBrush:(Brush *)brush {
    if (brush)
        [brushes addObject:brush];
}

- (void)removeBrush:(Brush *)brush {
    if (brush)
        [brushes removeObject:brush];
}



- (void) dealloc {
	[properties release];
	[brushes release];
	
	[super dealloc];
}

@end
