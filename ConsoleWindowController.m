/*
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

#import "ConsoleWindowController.h"

@implementation ConsoleWindowController

- (id)initWithWindowNibName:(NSString *)windowNibName {
    if ((self = [super initWithWindowNibName:windowNibName])) {
        NSFont* regularFont = [NSFont fontWithName:@"Menlo" size:10];
        regularAttrs = [[NSDictionary alloc] initWithObjectsAndKeys:regularFont, NSFontAttributeName, nil];
        
        NSFontManager* fontManager = [NSFontManager sharedFontManager];
        NSFont* boldFont = [fontManager convertFont:regularFont toHaveTrait:NSBoldFontMask];
        boldAttrs = [[NSDictionary alloc] initWithObjectsAndKeys:boldFont, NSFontAttributeName, nil];
    }
    
    return self;
}

- (void)dealloc {
    [regularAttrs release];
    [boldAttrs release];
    [super dealloc];
}

- (void)log:(NSString *)theString bold:(BOOL)isBold {
    NSAssert(theString != nil, @"string must not be nil");
    
    NSAttributedString* attributedString;
    if (isBold)
        attributedString = [[NSAttributedString alloc] initWithString:theString attributes:boldAttrs];
    else
        attributedString = [[NSAttributedString alloc] initWithString:theString attributes:regularAttrs];
    
    [[consoleTextView textStorage] appendAttributedString:attributedString];
    [consoleTextView scrollToEndOfDocument:self];
    [attributedString release];
    
    if (![[self window] isVisible])
        [[self window] orderFront:self];
}

- (void)log:(NSString *)theString {
    [self log:theString bold:NO];
}

- (void)logBold:(NSString *)theString {
    [self log:theString bold:YES];
}

- (IBAction)clearConsole:(id)sender {
    NSAttributedString* emptyString = [[NSAttributedString alloc] initWithString:@""];
    [[consoleTextView textStorage] setAttributedString:emptyString];
    [emptyString release];
}

@end
