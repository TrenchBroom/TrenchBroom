//
//  FaceInspectorController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 04.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "InspectorController.h"
#import "SelectionManager.h"
#import "TextureManager.h"
#import "GLFontManager.h"
#import "TextureView.h"
#import "SingleTextureView.h"
#import "TextureNameFilter.h"
#import "TextureUsageFilter.h"
#import "Map.h"
#import "Brush.h"
#import "Face.h"

static InspectorController* sharedInstance = nil;

@implementation InspectorController

+ (InspectorController *)sharedInspector {
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
    return @"Inspector";
}

- (void)updateTextureControls {
    NSMutableSet* selectedTextureNames = nil;
    NSMutableSet* selectedFaces = [[NSMutableSet alloc] initWithSet:[selectionManager selectedFaces]];
    NSSet* selectedBrushes = [selectionManager selectedBrushes];
    
    if ([selectedBrushes count] > 0) {
        NSEnumerator* brushEn = [selectedBrushes objectEnumerator];
        Brush* brush;
        while ((brush = [brushEn nextObject]))
            [selectedFaces addObjectsFromArray:[brush faces]];
    }
    
    if ([selectedFaces count] > 0) {
        selectedTextureNames = [[NSMutableSet alloc] init];

        [xOffsetField setEnabled:YES];
        [yOffsetField setEnabled:YES];
        [xScaleField setEnabled:YES];
        [yScaleField setEnabled:YES];
        [rotationField setEnabled:YES];
        
        NSEnumerator* faceEn = [selectedFaces objectEnumerator];
        Face* face = [faceEn nextObject];
        
        int xOffset = [face xOffset];
        int yOffset = [face yOffset];
        float xScale = [face xScale];
        float yScale = [face yScale];
        float rotation = [face rotation];
        NSString* textureName = [face texture];
        
        [selectedTextureNames addObject:textureName];
        
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
            [selectedTextureNames addObject:[face texture]];
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
            [singleTextureView setTexture:nil];
        } else {
            [textureNameField setStringValue:textureName];
            
            Texture* texture = [textureManager textureForName:textureName];
            [singleTextureView setTexture:texture];
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
        [singleTextureView setTexture:nil];
    }
    
    [textureView setSelectedTextureNames:selectedTextureNames];
    [selectedTextureNames release];
    [selectedFaces release];
}

- (void)faceFlagsChanged:(NSNotification *)notification {
    [self updateTextureControls];
}

- (void)selectionRemoved:(NSNotification *)notification {
    [self updateTextureControls];
    
    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face removeObserver:self];
}

- (void)selectionAdded:(NSNotification *)notification {
    [self updateTextureControls];

    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:SelectionFaces];
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face addObserver:self selector:@selector(faceFlagsChanged:) name:FaceFlagsChanged];
}

- (void)switchToContext:(NSOpenGLContext *)sharedContext selectionManager:(SelectionManager *)theSelectionManager textureManager:(TextureManager *)theTextureManager fontManager:(GLFontManager *)theFontManager map:(Map *)theMap {
    if (theSelectionManager == selectionManager && theTextureManager == textureManager)
        return;
    
    if (sharedContext != nil) {
        NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:[textureView pixelFormat] shareContext:sharedContext];
        [singleTextureView setOpenGLContext:context];
        [context release];
    }

    [textureManager removeObserver:self];
    [textureManager release];
    textureManager = [theTextureManager retain];
    [textureManager addObserver:self selector:@selector(textureManagerChanged:) name:TexturesAdded];
    [textureManager addObserver:self selector:@selector(textureManagerChanged:) name:TexturesRemoved];
    
    if (selectionManager != nil) {
        NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject]))
            [face removeObserver:self];

        [selectionManager removeObserver:self];
        [selectionManager release];
    }

    selectionManager = [theSelectionManager retain];
    [selectionManager addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded];
    [selectionManager addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved];

    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face addObserver:self selector:@selector(faceFlagsChanged:) name:FaceFlagsChanged];

    [map release];
    map = [theMap retain];
    
    [self updateTextureControls];
    [textureView switchToContext:sharedContext textureManager:textureManager fontManager:theFontManager map:map];
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

- (void)updateFilter {
    id<TextureFilter> filter = nil;
    NSString* pattern = [textureNameFilterField stringValue];
    
    if (pattern != nil && [pattern length] > 0)
        filter = [[TextureNameFilter alloc] initWithPattern:pattern];
    
    if ([textureUsageFilterSC selectedSegment] == 1) {
        id<TextureFilter> temp = [[TextureUsageFilter alloc] initWithTextureNames:[map textureNames] filter:filter];
        [filter release];
        filter = temp;
    }
    
    [textureView setTextureFilter:filter];
    [filter release];
}

- (IBAction)textureNameFilterTextChanged:(id)sender {
    [self updateFilter];
}

- (IBAction)textureUsageFilterChanged:(id)sender {
    [self updateFilter];
}

- (void)dealloc {
    [selectionManager removeObserver:self];
    [textureManager removeObserver:self];
    [selectionManager release];
    [textureManager release];
    [map release];
    [super dealloc];
}

@end
