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

- (void) dealloc {
	[properties release];
	[brushes release];
	
	[super dealloc];
}

@end
