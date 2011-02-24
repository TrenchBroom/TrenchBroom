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
#import "Brush.h"
#import "Vector3i.h"
#import "MapWindowController.h"
#import "MapParser.h"
#import "BrushFactory.h"
#import "ProgressWindowController.h"
#import "Options.h"
#import "Picker.h"
#import "GLResources.h"
#import "WadLoader.h"
#import "TextureManager.h"

@implementation MapDocument

- (void)makeWindowControllers {
	MapWindowController* controller = [[MapWindowController alloc] initWithWindowNibName:@"MapDocument"];
	[self addWindowController:controller];
    [controller release];
}

- (void)postInit {
    picker = [[Picker alloc] initWithDocument:self];
    glResources = [[GLResources alloc] init];

    NSString* wads = [[map worldspawn] propertyForKey:@"wad"];
    if (wads != nil) {
        TextureManager* textureManager = [glResources textureManager];
        NSArray* wadPaths = [wads componentsSeparatedByString:@";"];
        for (int i = 0; i < [wadPaths count]; i++) {
            NSString* wadPath = [[wadPaths objectAtIndex:i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            NSFileManager* fileManager = [NSFileManager defaultManager];
            if ([fileManager fileExistsAtPath:wadPath]) {
                int slashIndex = [wadPath rangeOfString:@"/" options:NSBackwardsSearch].location;
                NSString* wadName = [wadPath substringFromIndex:slashIndex + 1];
                
                WadLoader* wadLoader = [[WadLoader alloc] init];
                Wad* wad = [wadLoader loadFromData:[NSData dataWithContentsOfMappedFile:wadPath] wadName:wadName];
                [wadLoader release];
                
                [textureManager loadTexturesFrom:wad];
            }
        }
    }
}

- (id)initWithType:(NSString *)typeName error:(NSError **)outError {
    if (self = [super initWithType:typeName error:outError]) {
        map = [[Map alloc] init];
        
        NSUndoManager* undoManager = [self undoManager];
        [map setUndoManager:undoManager];

        [undoManager disableUndoRegistration];
        Entity* worldspawn = [map createEntityWithProperty:@"classname" value:@"worldspawn"];
        

        /*
        BrushFactory* brushFactory = [BrushFactory sharedFactory];
        // Brush* brush = [brushFactory createCuboidFor:worldspawn atCenter:[Vector3i nullVector] dimensions:[Vector3i vectorWithX:64 y:64 z:64] texture:@""];
        Brush* brush = [worldspawn createBrush];
        
        
        [brush createFaceWithPoint1:[Vector3i vectorWithX:276 y:108 z:176] point2:[Vector3i vectorWithX:276 y:120 z:176] point3:[Vector3i vectorWithX:260 y:120 z:176] texture:@"mt_sr_v13"];
        [brush createFaceWithPoint1:[Vector3i vectorWithX:260 y:120 z:208] point2:[Vector3i vectorWithX:276 y:120 z:208] point3:[Vector3i vectorWithX:276 y:108 z:208] texture:@"mt_sr_v13"];
        [brush createFaceWithPoint1:[Vector3i vectorWithX:252 y:116 z:176] point2:[Vector3i vectorWithX:252 y:116 z:208] point3:[Vector3i vectorWithX:268 y:100 z:208] texture:@"mt_sr_v13"];
        [brush createFaceWithPoint1:[Vector3i vectorWithX:288 y:122 z:208] point2:[Vector3i vectorWithX:288 y:122 z:176] point3:[Vector3i vectorWithX:268 y:102 z:176] texture:@"mt_sr_v13"];
        [brush createFaceWithPoint1:[Vector3i vectorWithX:288 y:120 z:176] point2:[Vector3i vectorWithX:288 y:120 z:208] point3:[Vector3i vectorWithX:288 y:152 z:208] texture:@"mt_sr_v13"];
        [brush createFaceWithPoint1:[Vector3i vectorWithX:289 y:152 z:176] point2:[Vector3i vectorWithX:289 y:152 z:208] point3:[Vector3i vectorWithX:253 y:116 z:208] texture:@"mt_sr_v13"];

         ( 276 108 176 ) ( 276 120 176 ) ( 260 120 176 ) mt_sr_v13 -59 116 -90 1 1
         ( 260 120 208 ) ( 276 120 208 ) ( 276 108 208 ) mt_sr_v13 -59 116 -90 1 1
         ( 252 116 176 ) ( 252 116 208 ) ( 268 100 208 ) mt_sr_v13 20 -62 -180 1 1.000008
         ( 288 122 208 ) ( 288 122 176 ) ( 268 102 176 ) mt_sr_v13 -34 -62 0 1 -1.000008
         ( 288 120 176 ) ( 288 120 208 ) ( 288 152 208 ) mt_sr_v13 -59 -62 -180 1 1.000008
         ( 289 152 176 ) ( 289 152 208 ) ( 253 116 208 ) mt_sr_v13 -6 -62 0 1 -1.000008
         */
        
        [undoManager enableUndoRegistration];
        [self postInit];
    }
    
    return self;
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    ProgressWindowController* pwc = [[ProgressWindowController alloc] initWithWindowNibName:@"ProgressWindow"];
    [[pwc window] makeKeyAndOrderFront:self];
    [[pwc label] setStringValue:@"Loading map file..."];
    
    NSProgressIndicator* indicator = [pwc progressIndicator];
    [indicator setIndeterminate:NO];
    [indicator setUsesThreadedAnimation:YES];

    MapParser* parser = [[MapParser alloc] initWithData:data];
    map = [parser parseWithProgressIndicator:indicator];
    [map retain];
    [parser release];

    NSUndoManager* undoManager = [self undoManager];
    [map setUndoManager:undoManager];
    [pwc close];
    
    [self postInit];
    return YES;
}

- (Map *)map {
    return map;
}

- (Picker *)picker {
    return picker;
}

- (GLResources *)glResources {
    return glResources;
}

- (void)dealloc {
    [map release];
    [picker release];
    [glResources release];
	[super dealloc];
}

@end
