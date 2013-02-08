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

#ifndef __CORE_WEB_SOCKET_WEB_SOCKET_FRAME_H__
#define __CORE_WEB_SOCKET_WEB_SOCKET_FRAME_H__ 1

#include <CoreFoundation/CoreFoundation.h>

// 4 bits opcode field.
enum WebSocketFrameOpCode {
    kWebSocketFrameOpCodeContinuation =  0, // Denotes a continuation frame (or undefined op code)
    kWebSocketFrameOpCodeText         =  1, // Denotes a text frame
    kWebSocketFrameOpCodeBinary       =  2, // Denotes a binary frame
    kWebSocketFrameOpCodeClose        =  8, // Denotes a connection close
    kWebSocketFrameOpCodePing         =  9, // Denotes a ping
    kWebSocketFrameOpCodePong         = 10  // Denotes a pong
};

typedef enum WebSocketFrameOpCode WebSocketFrameOpCode;

const char *__WebSocketFrameOpCodeCString[15];

enum WebSocketFrameState {
    kWebSocketFrameStateNone,
    kWebSocketFrameState2BytesHeader,
    kWebSocketFrameState16BitLength,
    kWebSocketFrameState64BitLength,
    kWebSocketFrameStateMaskingKey,
    kWebSocketFrameStatePayload,
    kWebSocketFrameStateReady,
    kWebSocketFrameStateError
};

typedef enum WebSocketFrameState WebSocketFrameState;

struct WebSocketFrame {
    CFAllocatorRef allocator;
    CFIndex retainCount;
    
    WebSocketFrameState state;
    
    Boolean isFin;
    Boolean isRsv1;
    Boolean isRsv2;
    Boolean isRsv3;
    WebSocketFrameOpCode opCode;
    Boolean isMasked;
    UInt8 maskingKey[4];
    UInt8 payloadOffset;
    UInt64 payloadLength;

    // Frame data, including header and payload (ext and app data).
    CFMutableDataRef data;
};

typedef struct WebSocketFrame * WebSocketFrameRef;

/**
 * Create new frame object.
 */
WebSocketFrameRef
WebSocketFrameCreate (CFAllocatorRef allocator) CF_RETURNS_RETAINED;

/**
 * Create new websocket frame object with specified payload capacity.
 */
WebSocketFrameRef
WebSocketFrameCreateWithPayloadCapacity (CFAllocatorRef allocator, CFIndex payloadCapacity) CF_RETURNS_RETAINED;

/**
 * Deallocate websocket frame object. Direct use of this function is not recommended,
 * use WebSocketFrameRelease instead.
 */
void
WebSocketFrameDealloc (WebSocketFrameRef self);

/**
 * Increment retain count.
 */
WebSocketFrameRef
WebSocketFrameRetain (WebSocketFrameRef self);

/**
 * Release websocket frame object. If the retain count reaches zero, dealloc function will be
 * called and all reference to the object will be invalid.
 */
void
WebSocketFrameRelease (WebSocketFrameRef self);

/**
 * @return TRUE if the frame has FIN bit set, FALSE otherwise.
 */
Boolean
WebSocketFrameGetIsFin (WebSocketFrameRef self);

/**
 *
 */
WebSocketFrameOpCode
WebSocketFrameGetOpCode (WebSocketFrameRef self);

Boolean
WebSocketFrameGetIsMasked (WebSocketFrameRef self);

UInt64
WebSocketFrameGetPayloadLength (WebSocketFrameRef self);

WebSocketFrameState
WebSocketFrameGetState (WebSocketFrameRef self);

void
WebSocketFrameAppend (WebSocketFrameRef self, const UInt8 *bytes, CFIndex length);

void
WebSocketFrameReset (WebSocketFrameRef self);

void
WebSocketFrameParse (WebSocketFrameRef self);

void
WebSocketFrameGetPayload (WebSocketFrameRef self, CFRange range, UInt8 *buffer, Boolean unmask);

CFStringRef
WebSocketFrameCopyPayloadString (WebSocketFrameRef self, CFStringEncoding encoding);

#endif
