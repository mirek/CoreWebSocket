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

#include <dispatch/dispatch.h>

#include "WebSocket.h"
#include "WebSocketFrame.h"

void
Callback (WebSocketRef self, WebSocketClientRef client, CFStringRef value) {
    if (value) {
        CFShow(value);
    }
}

int
main (int argc, const char *argv[]) {
    WebSocketRef webSocket = WebSocketCreateWithHostAndPort(NULL, kWebSocketHostAny, 60001, NULL);
    if (webSocket) {
        webSocket->callbacks.didClientReadCallback = Callback;
        CFRunLoopRun();
        WebSocketRelease(webSocket);
    }
    return 0;
}

