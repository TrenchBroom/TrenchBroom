//
//  FaceInspectorController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 04.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FaceInspectorController.h"
#import "SelectionManager.h"
#import "TextureManager.h"
#import "SingleTextureView.h"
#import "Face.h"

static FaceInspectorController* sharedInstance = nil;

@implementation FaceInspectorController

+ (FaceInspectorController *)sharedInspector {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (NSString *)windowNibName {
    return @"FaceInspector";
}

- (void)selectionChanged:(NSNotification *)notification {
    NSLog(@"selection manager: %@", selectionManager);
    if (selectionManager != nil) {
        NSLog(@"selection manager: %i", [selectionManager mode]);
    }
    
    if ([selectionManager mode] == SM_FACES && [[selectionManager selectedFaces] count] > 0) {
        [xOffsetField setEnabled:YES];
        [yOffsetField setEnabled:YES];
        [xScaleField setEnabled:YES];
        [yScaleField setEnabled:YES];
        [rotationField setEnabled:YES];
        
        NSSet* faces = [selectionManager selectedFaces];
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face = [faceEn nextObject];
        
        int xOffset = [face xOffset];
        int yOffset = [face yOffset];
        float xScale = [face xScale];
        float yScale = [face yScale];
        float rotation = [face rotation];
        NSString* textureName = [face texture];
        
        BOOL xOffsetMultiple = NO;
        BOOL yOffsetMultiple = NO;
        BOOL xScaleMultiple = NO;
        BOOL yScaleMultiple = NO;
        BOOL rotationMultiple = NO;
        BOOL textureMultiple = NO;
        
        while ((face = [faceEn nextObject])) {
            xOffsetMultiple  |= xOffset  != [face xOffset];
            yOffsetMultiple  |= yOffset  != [face yOffset];
            xScaleMultiple   |= xScale   != [face xScale];
            yScaleMultiple   |= yScale   != [face yScale];
            rotationMultiple |= rotation != [face rotation];
            textureMultiple  |= ![textureName isEqualToString:[face texture]];
        }
        
        if (xOffsetMultiple) {
            [[xOffsetField cell] setPlaceholderString:@"multiple"];
            [xOffsetField setStringValue:@""];
        } else {
            [xOffsetField setIntValue:xOffset];
        }
        
        if (yOffsetMultiple) {
            [[yOffsetField cell] setPlaceholderString:@"multiple"];
            [yOffsetField setStringValue:@""];
        } else {
            [yOffsetField setIntValue:yOffset];
        }
        
        if (xScaleMultiple) {
            [[xScaleField cell] setPlaceholderString:@"multiple"];
            [xScaleField setStringValue:@""];
        } else {
            [xScaleField setFloatValue:xScale];
        }
        
        if (yScaleMultiple) {
            [[yScaleField cell] setPlaceholderString:@"multiple"];
            [yScaleField setStringValue:@""];
        } else {
            [yScaleField setFloatValue:yScale];
        }
        
        if (rotationMultiple) {
            [[rotationField cell] setPlaceholderString:@"multiple"];
            [rotationField setStringValue:@""];
        } else {
            [rotationField setFloatValue:rotation];
        }
        
        if (textureMultiple) {
            [[textureNameField cell] setPlaceholderString:@"multiple"];
            [textureNameField setStringValue:@""];
            [textureView setTexture:nil];
        } else {
            [textureNameField setStringValue:textureName];
            
            Texture* texture = [textureManager textureForName:textureName];
            [textureView setTexture:texture];
        }
    } else {
        [xOffsetField setEnabled:NO];
        [yOffsetField setEnabled:NO];
        [xScaleField setEnabled:NO];
        [yScaleField setEnabled:NO];
        [rotationField setEnabled:NO];
        
        [[xOffsetField cell] setPlaceholderString:@"n/a"];
        [[yOffsetField cell] setPlaceholderString:@"n/a"];
        [[xScaleField cell] setPlaceholderString:@"n/a"];
        [[yScaleField cell] setPlaceholderString:@"n/a"];
        [[rotationField cell] setPlaceholderString:@"n/a"];
        [[textureNameField cell] setPlaceholderString:@"n/a"];
        
        [xOffsetField setStringValue:@""];
        [yOffsetField setStringValue:@""];
        [xScaleField setStringValue:@""];
        [yScaleField setStringValue:@""];
        [rotationField setStringValue:@""];
        [textureNameField setStringValue:@""];
        [textureView setTexture:nil];
    }
}

- (void)switchToContext:(NSOpenGLContext *)sharedContext selectionManager:(SelectionManager *)theSelectionManager textureManager:(TextureManager *)theTextureManager {
    if (theSelectionManager == selectionManager && theTextureManager == textureManager)
        return;
    
    if (sharedContext != nil) {
        NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:[textureView pixelFormat] shareContext:sharedContext];
        [textureView setOpenGLContext:context];
        [context release];
    }

    if (textureManager != nil) {
        [textureManager removeObserver:self];
        [textureManager release];
        textureManager = nil;
    }
    
    if (selectionManager != nil) {
        [selectionManager removeObserver:self];
        [selectionManager release];
        selectionManager = nil;
    }
    
    if (theTextureManager != nil) {
        textureManager = [theTextureManager retain];
        [textureManager addObserver:self selector:@selector(textureManagerChanged:) name:TexturesAdded];
        [textureManager addObserver:self selector:@selector(textureManagerChanged:) name:TexturesRemoved];
    }

    if (theSelectionManager != nil) {
        selectionManager = [theSelectionManager retain];
        [selectionManager addObserver:self selector:@selector(selectionChanged:) name:SelectionAdded];
        [selectionManager addObserver:self selector:@selector(selectionChanged:) name:SelectionRemoved];
        [self selectionChanged:nil];
    }
}

- (void)textureManagerChanged:(NSNotification *)notification {
}

- (IBAction)xOffsetTextChanged:(id)sender {
    int xOffset = [xOffsetField intValue];
    NSSet* faces = [selectionManager selectedFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setXOffset:xOffset];
}

- (IBAction)yOffsetTextChanged:(id)sender {
    int yOffset = [yOffsetField intValue];
    NSSet* faces = [selectionManager selectedFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setYOffset:yOffset];
}

- (IBAction)xScaleTextChanged:(id)sender {
    float xScale = [xScaleField floatValue];
    NSSet* faces = [selectionManager selectedFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setXScale:xScale];
}

- (IBAction)yScaleTextChanged:(id)sender {
    float yScale = [yScaleField floatValue];
    NSSet* faces = [selectionManager selectedFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setYScale:yScale];
}

- (IBAction)rotationTextChanged:(id)sender {
    float rotation = [rotationField floatValue];
    NSSet* faces = [selectionManager selectedFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setRotation:rotation];
}

- (void)dealloc {
    [selectionManager removeObserver:self];
    [textureManager removeObserver:self];
    [selectionManager release];
    [textureManager release];
    [super dealloc];
}

@end
