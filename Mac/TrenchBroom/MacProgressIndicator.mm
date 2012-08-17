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

#import "MacProgressIndicator.h"
#include "NSString+StdStringAdditions.h"

namespace TrenchBroom {
    namespace Controller {
        MacProgressIndicator::MacProgressIndicator(const std::string& text) : ProgressIndicator(100) {
            m_windowController = [[ProgressWindowController alloc] initWithWindowNibName:@"ProgressWindow"];
            [[m_windowController window] makeKeyAndOrderFront:NULL];
            
            NSTextField* label = [m_windowController label];
            NSProgressIndicator* indicator = [m_windowController progressIndicator];

            [label setStringValue:[NSString stringWithStdString:text]];
            
            [indicator setIndeterminate:NO];
            [indicator setUsesThreadedAnimation:YES];
            [indicator setMaxValue:100];

            [[NSRunLoop currentRunLoop] runMode:NSModalPanelRunLoopMode beforeDate:[NSDate date]];
        }

        MacProgressIndicator::~MacProgressIndicator() {
            [m_windowController close];
            [m_windowController release];
        }
        
        void MacProgressIndicator::doReset() {
            NSProgressIndicator* indicator = [m_windowController progressIndicator];
            [indicator setDoubleValue:0];
            [[NSRunLoop currentRunLoop] runMode:NSModalPanelRunLoopMode beforeDate:[NSDate date]];
        }

        void MacProgressIndicator::doUpdate() {
            NSProgressIndicator* indicator = [m_windowController progressIndicator];
            [indicator setDoubleValue:percent()];
            [[NSRunLoop currentRunLoop] runMode:NSModalPanelRunLoopMode beforeDate:[NSDate date]];
        }

        void MacProgressIndicator::setText(const std::string& text) {
            NSTextField* label = [m_windowController label];
            [label setStringValue:[NSString stringWithStdString:text]];
            [[NSRunLoop currentRunLoop] runMode:NSModalPanelRunLoopMode beforeDate:[NSDate date]];
        }
    }
}

@implementation ProgressWindowController

- (NSProgressIndicator *)progressIndicator {
    return progressIndicator;
}

- (NSTextField *)label {
    return label;
}

@end
