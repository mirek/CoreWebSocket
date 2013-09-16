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

#ifndef __CORE_WEB_SOCKET_WEB_SOCKET__
#define __CORE_WEB_SOCKET_WEB_SOCKET__ 1

#include "CoreWebSocket/CoreWebSocket.h"

#define __WebSocketMaxHeaderKeyLength 4096

#pragma mark Lifecycle

WebSocketRef WebSocketCreateWithHostAndPort (CFAllocatorRef allocator, CFStringRef host, UInt16 port, void *userInfo);
WebSocketRef WebSocketCreate                (CFAllocatorRef allocator, void *userInfo);

void         WebSocketDealloc            (WebSocketRef self);
void         WebSocketRetain             (WebSocketRef self);
void         WebSocketRelease            (WebSocketRef self);

CFStringRef  WebSocketCopyHostString (WebSocketRef self) CF_RETURNS_RETAINED;
UInt16       WebSocketGetPort        (WebSocketRef self);

void         WebSocketConnectWithHostAndPort(WebSocketRef webSocket, CFStringRef hostOrAddress, UInt32 port);

void         WebSocketWriteWithString               (WebSocketRef webSocket, CFStringRef value);
CFIndex      WebSocketWriteWithStringAndClientIndex (WebSocketRef webSocket, CFStringRef value, CFIndex index);

WebSocketClientRef
WebSocketGetClientAtIndex (WebSocketRef self, CFIndex index);

CFIndex
WebSocketGetClientCount (WebSocketRef self);

#pragma mark Callbacks

void         WebSocketSetClientReadCallback (WebSocketRef self, WebSocketDidClientReadCallback callback);

#pragma mark Internal, client management

CFIndex __WebSocketAppendClient (WebSocketRef webSocket, WebSocketClientRef client);
CFIndex __WebSocketRemoveClient (WebSocketRef webSocket, WebSocketClientRef client);

#pragma mark Internal, socket callback

void __WebSocketAcceptCallBack(CFSocketRef socket, CFSocketCallBackType type, CFDataRef address, const void *data, void *info);

#endif

