//
//  RenderBrush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "RenderBrush.h"
#import "RenderPolygon.h"
#import "Face.h"
#import "Plane.h"

@implementation RenderBrush

- (id)init {
    if (self = [super init]) {
        polygons = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithBrush:(Brush *)aBrush {
    if (self = [self init]) {
        [self setBrush:aBrush];
    }
    
    return self;
}

- (void)brushChanged:(NSNotification *)notification {

    [self setBrush:[notification object]];
}

- (Brush *)brush {
    
    return brush;
}

- (void)setBrush:(Brush *)aBrush {
    NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
    if (brush) {
        [notificationCenter removeObserver:self name:BrushFaceAdded object:brush];
        [notificationCenter removeObserver:self name:BrushFaceRemoved object:brush];
        [brush release];
    }
    
    brush = [aBrush retain];
    [self refreshPolygons];
    
    [notificationCenter addObserver:self selector:@selector(brushChanged:) name:BrushFaceAdded object:brush];
    [notificationCenter addObserver:self selector:@selector(brushChanged:) name:BrushFaceRemoved object:brush];
}

-(void)refreshPolygons {
    
    if (brush) {
        NSMutableArray* planes = [[NSMutableArray alloc] initWithCapacity:10];
        NSMutableArray* lines = [[NSMutableArray alloc] initWithCapacity:10];
        NSMutableDictionary* vertices = [[NSMutableDictionary alloc] initWithCapacity:10];

        NSEnumerator* faceEn = [[brush faces] objectEnumerator];
        Face* face = [faceEn nextObject];

        Plane* plane = [[Plane alloc] initWithPoint1:[face point1] point2:[face point2] point3:[face point3]];
        [planes addObject:plane];
        [plane release];
        
        while ((face = [faceEn nextObject])) {
            Plane *plane = [[Plane alloc] initWithPoint1:[face point1] point2:[face point2] point3:[face point3]];
            
            NSEnumerator* planeEn = [planes objectEnumerator];
            Plane* testPlane;
            
            while ((testPlane = [planeEn nextObject])) {
                Line* line = [plane intersectionWithPlane:testPlane];
                if (line) {
                    NSEnumerator* lineEn = [lines objectEnumerator];
                    Line* testLine;
                    
                    while ((testLine = [lineEn nextObject])) {
                        Vector3f* vertex = [line intersectionWithLine:testLine];
                        if (vertex) {
                            
                        }
                    }
                    
                    [lines addObject:line];
                    [line release];
                }
            }
            
            [planes addObject:plane];
            [plane release];
        }
        
        [vertices release];
        [planes release];
        [lines release];
    }
}
                    
- (void)dealloc {
    [brush release];
    [polygons release];

    [super dealloc];
}

@end
