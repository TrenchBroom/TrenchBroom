//
//  MyDocument.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MapDocument.h"
#import "Map.h"
#import "Entity.h"
#import "Vector3i.h"
#import "MapWindowController.h"
#import "MapParser.h"

@implementation MapDocument

- (void)makeWindowControllers {
	MapWindowController* controller = [[MapWindowController alloc] initWithWindowNibName:@"MapDocument"];
	[self addWindowController:controller];
    [controller release];
}

- (id)initWithType:(NSString *)typeName error:(NSError **)outError {
    if (self = [super initWithType:typeName error:outError]) {
        map = [[Map alloc] init];
        Entity* worldspawn = [map createEntityWithProperty:@"classname" value:@"worldspawn"];
        [worldspawn createCuboidAt:[Vector3i vectorWithX:0 y:0 z:0] 
                              dimensions:[Vector3i vectorWithX:5 y:5 z:5] 
                                 texture:@""];
    }
    
    return self;
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    MapParser* parser = [[MapParser alloc] initWithData:data];
    map = [[parser parse] retain];
    [parser release];

    return YES;
}

- (Map *)map {
    return map;
}

- (void)dealloc {
    [map release];
	[super dealloc];
}

@end
