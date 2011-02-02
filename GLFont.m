//
//  GLFont.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLFont.h"


@implementation GLFont

- (id)initWithFont:(NSFont *)theFont {
    if (theFont == nil)
        [NSException raise:NSInvalidArgumentException format:@"font must not be nil"];
    
    if (self = [super init]) {
        float area = 0;
        // see "Calculating Text Height" in help
        
        NSMutableString* charString = [[NSMutableString alloc] init];
        for (int i = 0; i < 128; i++)
            [charString appendFormat:@"c", (char)i];
        
        NSTextStorage *textStorage = [[NSTextStorage alloc] initWithString:charString];
        [charString release];
        
        NSTextContainer *textContainer = [[NSTextContainer alloc] init];
        NSLayoutManager *layoutManager = [[NSLayoutManager alloc] init];
        
        [layoutManager addTextContainer:textContainer];
        [textStorage addLayoutManager:layoutManager];
        
        float width = 32;
        float height;
        do {
            width *= 2; // actually starting with 64
            [textContainer setContainerSize:NSMakeSize(width, FLT_MAX)];
            [layoutManager glyphRangeForTextContainer:textContainer]; // force layout
            height = [layoutManager usedRectForTextContainer:textContainer].size.height;
        } while (height > width);
        
        // (void)drawGlyphsForGlyphRange:(NSRange)glyphsToShow atPoint:(NSPoint)origin to render the text to an image
    }    
    
}

@end
