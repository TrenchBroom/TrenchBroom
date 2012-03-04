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

#import "BoundsFeedbackFigure.h"
#import "GLUtils.h"

@interface BoundsFeedbackFigure (private)

- (void)doRender;

@end

@implementation BoundsFeedbackFigure (private)

- (void)doRender {
    glBegin(GL_LINES);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.min.z);
    
    glVertex3f(bounds.max.x, bounds.max.y, bounds.max.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.max.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.max.z);
    
    glVertex3f(bounds.min.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.min.z);
    
    glVertex3f(bounds.max.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.max.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.min.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.max.z);
    
    glVertex3f(bounds.min.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.max.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.min.y, bounds.max.z);
    glVertex3f(bounds.min.x, bounds.max.y, bounds.max.z);
    glEnd();
}

@end

@implementation BoundsFeedbackFigure

- (void)setBounds:(TBoundingBox *)theBounds {
    bounds = *theBounds;
}

- (void)render:(id <Filter>)theFilter {
    glDisable(GL_DEPTH_TEST);
    glColor4f(0, 1, 0, 0.5f);
    [self doRender];
    
    glEnable(GL_DEPTH_TEST);
    glColor4f(0, 1, 0, 1);
    [self doRender];
}

@end
