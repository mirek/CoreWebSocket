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

#ifndef __CORE_WEB_SOCKET_WEB_SOCKET_TYPES__
#define __CORE_WEB_SOCKET_WEB_SOCKET_TYPES__ 1

#include <CoreFoundation/CoreFoundation.h>
#include "WebSocketFrame.h"

#define WebSocketLog(fmt, ...) printf(fmt, __VA_ARGS__)

#define kWebSocketHostAny      CFSTR("0.0.0.0")
#define kWebSocketHostLoopBack CFSTR("127.0.0.1")
#define kWebSocketPortAny      0

typedef struct WebSocket  WebSocket;
typedef        WebSocket *WebSocketRef;

typedef struct WebSocketClient  WebSocketClient;
typedef        WebSocketClient *WebSocketClientRef;

#pragma mark WebSocket Protocol

typedef enum WebSocketProtocol WebSocketProtocol;

enum WebSocketProtocol {
    kWebSocketProtocolUnknown           = -1,
    kWebSocketProtocolDraftIETF_HYBI_00 =  0,
    kWebSocketProtocolDraftIETF_HYBI_06 =  6,
    kWebSocketProtocol_RFC6455_13       = 13
};

#pragma mark WebSocket Callbacks

typedef void (*WebSocketDidAddClientCallback)     (WebSocketRef webSocket, WebSocketClientRef client);
typedef void (*WebSocketWillRemoveClientCallback) (WebSocketRef webSocket, WebSocketClientRef client);
typedef void (*WebSocketDidClientReadCallback)    (WebSocketRef webSocket, WebSocketClientRef client, CFStringRef value);

typedef struct WebSocketCallbacks WebSocketCallbacks;

struct WebSocketCallbacks {
    WebSocketDidAddClientCallback     didAddClientCallback;
    WebSocketWillRemoveClientCallback willRemoveClientCallback;
    WebSocketDidClientReadCallback    didClientReadCallback;
};

#pragma mark WebSocket Client

enum WebSocketClientState {
    kWebSocketClientInitialized,
    kWebSocketClientReadStreamOpened,
    kWebSocketClientWriteStreamOpened,
    kWebSocketClientHandShakeError,
    kWebSocketClientHandShakeRead,
    kWebSocketClientHandShakeSent,
    kWebSocketClientReady
};

struct WebSocketClient {
    CFAllocatorRef allocator;
    CFIndex retainCount;
    WebSocketRef webSocket;
    CFSocketNativeHandle handle;
    CFReadStreamRef read;
    CFWriteStreamRef write;
    
    CFMutableArrayRef writeQueue;
    
    CFHTTPMessageRef handShakeRequestHTTPMessage;
    WebSocketProtocol protocol;
    
    // Linked list of clients
    WebSocketClientRef previousClient;
    WebSocketClientRef nextClient;
    
    CFStreamClientContext context;
    
    Boolean didReadHandShake;
    Boolean didWriteHandShake;
    
    WebSocketFrameRef frame;
};

struct WebSocket {
    CFAllocatorRef allocator;
    CFIndex retainCount;
    void *userInfo;
    
    struct sockaddr_in addr;
    CFSocketRef socket;
    CFReadStreamRef read;
    CFWriteStreamRef write;
    
    CFIndex clientsUsedLength;
    CFIndex clientsLength;
    WebSocketClientRef *clients;
    
    WebSocketCallbacks callbacks;
};

#endif
