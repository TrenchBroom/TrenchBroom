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

#include "MacScreenDC.h"

#include "wx/wxprec.h"

#include "wx/dcscreen.h"
#include "wx/osx/dcscreen.h"

#include "wx/osx/private.h"
#include "wx/graphics.h"

IMPLEMENT_ABSTRACT_CLASS(MacScreenDCImpl, wxWindowDCImpl)

MacScreenDCImpl::MacScreenDCImpl(wxDC *owner) :
wxWindowDCImpl(owner) {
    CGRect cgbounds ;
    cgbounds = CGDisplayBounds(CGMainDisplayID());
    m_width = (wxCoord)cgbounds.size.width;
    m_height = (wxCoord)cgbounds.size.height;

    NSRect overlayBounds = NSMakeRect(0, 0, m_width, m_height);
    NSWindow* overlayWindow = [[NSWindow alloc] initWithContentRect:overlayBounds styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
    [overlayWindow setIgnoresMouseEvents:YES];
    [overlayWindow setOpaque:NO];
    [overlayWindow setBackgroundColor:[NSColor clearColor]];
    [overlayWindow setLevel:NSFloatingWindowLevel];
    [overlayWindow orderFrontRegardless];
    m_overlayWindow = overlayWindow;
    
    SetGraphicsContext(wxGraphicsContext::CreateFromNative([[overlayWindow graphicsContext] graphicsPort]));
    m_ok = true ;
}

MacScreenDCImpl::~MacScreenDCImpl() {
    wxDELETE(m_graphicContext);
    NSWindow* overlayWindow = (NSWindow*)m_overlayWindow;
    [overlayWindow release];
}

void MacScreenDCImpl::Clear() {
    NSWindow* overlayWindow = (NSWindow*)m_overlayWindow;

    [NSGraphicsContext setCurrentContext:[overlayWindow graphicsContext]];
    [NSGraphicsContext saveGraphicsState];
    [[overlayWindow graphicsContext] setCompositingOperation:NSCompositeClear];
    
    NSBezierPath* path = [NSBezierPath bezierPathWithRect:[overlayWindow frame]];
    [path fill];
    
    [NSGraphicsContext restoreGraphicsState];
}

void MacScreenDCImpl::Flush() {
    NSWindow* overlayWindow = (NSWindow*)m_overlayWindow;
    [[overlayWindow graphicsContext] flushGraphics];
}

wxBitmap MacScreenDCImpl::DoGetAsBitmap(const wxRect *subrect) const {
    wxRect rect = subrect ? *subrect : wxRect(0, 0, m_width, m_height);
    wxBitmap bmp(rect.GetSize(), 32);
    return bmp;
}
