/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "TextRenderer.h"
#import "GLFontManager.h"
#import "GLString.h"
#import "TextAnchor.h"
#import "GLUtils.h"
#import "Camera.h"

@implementation TextRenderer

- (id)initWithFontManager:(GLFontManager *)theFontManager camera:(Camera *)theCamera fadeDistance:(float)theFadeDistance {
    NSAssert(theFontManager != nil, @"font manager must not be nil");
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theFadeDistance > 0, @"fade distance must be greater than 0");
    
    if ((self = [self init])) {
        strings = [[NSMutableDictionary alloc] init];
        anchors = [[NSMutableDictionary alloc] init];
        fontManager = [theFontManager retain];
        camera = [theCamera retain];
        fadeDistance = theFadeDistance;
    }
    
    return self;
}

- (void)dealloc {
    [fontManager release];
    [strings release];
    [anchors release];
    [super dealloc];
}

- (void)addString:(NSString *)theString forKey:(id <NSCopying>)theKey withFont:(NSFont *)theFont withAnchor:(id <TextAnchor>)theAnchor {
    NSAssert(theString != nil, @"string must not be nil");
    NSAssert(theKey != nil, @"key must not be nil");
    NSAssert(theFont != nil, @"font must not be nil");
    NSAssert(theAnchor != nil, @"anchor must not be nil");
    
    GLString* glString = [fontManager createGLString:theString font:theFont];
    [strings setObject:glString forKey:theKey];
    [anchors setObject:theAnchor forKey:theKey];
}

- (void)removeStringForKey:(id <NSCopying>)theKey {
    NSAssert(theKey != nil, @"key must not be nil");
    
    [strings removeObjectForKey:theKey];
    [anchors removeObjectForKey:theKey];
}

- (void)addString:(GLString *)theString forKey:(id <NSCopying>)theKey withAnchor:(id <TextAnchor>)theAnchor {
    NSAssert(theString != nil, @"string must not be nil");
    NSAssert(theKey != nil, @"key must not be nil");
    NSAssert(theAnchor != nil, @"anchor must not be nil");
    
    [strings setObject:theString forKey:theKey];
    [anchors setObject:theAnchor forKey:theKey];
}

- (void)moveStringWithKey:(id <NSCopying>)theKey toTextRenderer:(TextRenderer *)theTextRenderer {
    NSAssert(theKey != nil, @"key must not be nil");
    NSAssert(theTextRenderer != nil, @"text renderer must not be nil");

    GLString* string = [strings objectForKey:theKey];
    id <TextAnchor> anchor = [anchors objectForKey:theKey];
    
    [theTextRenderer addString:string forKey:theKey withAnchor:anchor];
    [self removeStringForKey:theKey];
}


- (void)renderColor:(const TVector4f *)theColor {
    glPolygonMode(GL_FRONT, GL_FILL);

    TVector3f position;
    float cutOff = (fadeDistance + 100) * (fadeDistance + 100);

    for (id <NSCopying> key in strings) {
        GLString* glString = [strings objectForKey:key];
        id <TextAnchor> anchor = [anchors objectForKey:key];
        
        [anchor position:&position];
        float dist2 = [camera squaredDistanceTo:&position];
        if (dist2 <= cutOff) {
            float dist = sqrt(dist2);
            float factor = dist / 300;
            
            NSSize size = [glString size];

            glPushMatrix();
            glTranslatef(position.x, position.y, position.z);
            [camera setBillboardMatrix];
            glScalef(factor, factor, 0);
            glTranslatef(-size.width / 2, 0, 0);

            float alphaFactor = 1 - fmaxf((dist - fadeDistance), 0) / 100;
            
            glColor4f(0, 0, 0, 0.6f * alphaFactor);
            [glString renderBackground:NSMakeSize(2, 1)];
            
            glSetEdgeOffset(0.5f);
            glColor4f(theColor->x, theColor->y, theColor->z, theColor->w * alphaFactor);
            [glString render];
            glResetEdgeOffset();
            
            glPopMatrix();
        }
    }
    
}

- (void)removeAllStrings {
    [strings removeAllObjects];
    [anchors removeAllObjects];
}

@end
