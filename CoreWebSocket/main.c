//
// The MIT License
//
// Copyright (c) 2011 - 2013, Mirek Rusin <mirek [at] me [dot] com>
// http://github.com/mirek/CoreWebSocket
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#include <CoreGraphics/CGGeometry.h>
#include <dispatch/dispatch.h>

#include "WebSocket.h"
#include "WebSocketFrame.h"

#define FPS 60

CGPoint box1 = { 0, 0 };
CGPoint box2 = { 0, 0 };

void
Callback (WebSocketRef self, WebSocketClientRef client, CFStringRef value) {
    if (value) {
        char *buffer = malloc(1024);
        if (CFStringGetCString(value, buffer, 1024, kCFStringEncodingUTF8)) {
            sscanf(buffer, "%lf %lf", &box1.x, &box1.y);
        }
        free(buffer);
    }
}

void
TimerCallback (CFRunLoopTimerRef timer, void *info) {
    WebSocketRef self = (WebSocketRef) info;

    box2.x += (box1.x - box2.x) * 0.1;
    box2.y += (box1.y - box2.y) * 0.1;

    for (CFIndex i = 0; i < WebSocketGetClientCount(self); ++i) {
        WebSocketClientRef client = WebSocketGetClientAtIndex(self, i);
        WebSocketClientWriteWithFormat(client, CFSTR("[%lf, %lf]"), box2.x, box2.y);
    }
}

int
main (int argc, const char *argv[]) {
    WebSocketRef webSocket = WebSocketCreateWithHostAndPort(NULL, kWebSocketHostAny, 6001, NULL);
    if (webSocket) {

        printf("Running on 0.0.0.0:6001... Open public/test.html file.");

        webSocket->callbacks.didClientReadCallback = Callback;

        // Send some data periodically to the web page.
        CFRunLoopTimerContext context = { 0, webSocket, NULL, NULL, NULL };
        CFRunLoopTimerRef timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent(), 1.0 / FPS, 0, 0, TimerCallback, &context);
        CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopCommonModes);

        CFRunLoopRun();

        WebSocketRelease(webSocket);
    }
    return 0;
}

