//
//  Entitiy.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Entity.h"
#import "IdGenerator.h"
#import "Vector3i.h"
#import "Brush.h"

@implementation Entity

- (id)init {
    if (self = [super init]) {
        entityId = [[IdGenerator sharedGenerator] getId];
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithProperty:(NSString *)key value:(NSString *)value {
	if (self = [self init]) {
		[self setProperty:key value:value];
	}
	
	return self;
}

- (Brush *)createCuboidAt:(Vector3i *)position dimensions:(Vector3i *)dimensions texture:(NSString *)texture {
    Brush* brush = [[Brush alloc] initCuboidAt:position dimensions:dimensions texture:texture];
    [brushes addObject:brush];
    
    return [brush autorelease];
}

- (Brush *)createBrush {
    Brush* brush = [[Brush alloc] init];
    [brushes addObject:brush];
    
    return [brush autorelease];
}

- (NSNumber *)getId {
    return entityId;
}

- (NSArray *)brushes {
    return brushes;
}

- (void)setProperty:(NSString *)key value:(NSString *)value {
    NSString *oldValue = [self propertyForKey:key];
    BOOL exists = oldValue != nil;
    
    if (exists && [oldValue isEqualToString:value])
        return;
    
    [properties setObject:value forKey:key];
}

- (void)removeProperty:(NSString *)key {
    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    [properties removeObjectForKey:key];
}

- (NSString *)propertyForKey:(NSString *)key {
    return (NSString *)[properties objectForKey:key];
}

- (NSString *)classname {
    return [self propertyForKey:@"classname"];
}

- (NSDictionary *)properties {
    return properties;
}

- (BOOL)isWorldspawn {
    return [[self classname] isEqualToString:@"worldspawn"];
}

- (void) dealloc {
	[properties release];
	[brushes release];
	[super dealloc];
}

@end
