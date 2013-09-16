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

#include <CoreFoundation/CoreFoundation.h>
#include <string.h>

#include "WebSocketFrame.h"

#define if_self if (self)
#define if_self_and(x) if ((self) && (x))

#pragma mark Private declarations

UInt8   __WebSocketFrameUInt8                  (WebSocketFrameRef self, CFIndex offset);
UInt16  __WebSocketFrameUInt16                 (WebSocketFrameRef self, CFIndex offset);
UInt32  __WebSocketFrameUInt32                 (WebSocketFrameRef self, CFIndex offset);
UInt64  __WebSocketFrameUInt64                 (WebSocketFrameRef self, CFIndex offset);
CFIndex __WebSocketFrameGetSizeOfPayloadLength (WebSocketFrameRef self);

#define kWebSocketFrameDefaultReadStreamBufferLength (4 * 1024)

WebSocketFrameRef
WebSocketFrameCreate (CFAllocatorRef allocator) {
    WebSocketFrameRef self = CFAllocatorAllocate(allocator, sizeof(struct WebSocketFrame), 0);
    memset(self, 0, sizeof(struct WebSocketFrame));
    if_self {
        self->retainCount = 1;
        self->allocator = allocator ? CFRetain(allocator) : NULL;
        self->data = CFDataCreateMutable(allocator, 0);
    }
    return self;
}

WebSocketFrameRef
WebSocketFrameCreateWithPayloadData (CFAllocatorRef allocator, WebSocketFrameOpCode opCode, Boolean isMasked, UInt8 *maskingKey, CFDataRef payload) {
    WebSocketFrameRef self = WebSocketFrameCreate(allocator);
    if_self {
        UInt8 header[2 + 8];
        memset(header, 0, sizeof(header));

        Boolean isFin = TRUE;
        Boolean rsv1 = FALSE;
        Boolean rsv2 = FALSE;
        Boolean rsv3 = FALSE;

        header[0] |= ((isFin ? 1 : 0) << 7);
        header[0] |= ((rsv1  ? 1 : 0) << 6);
        header[0] |= ((rsv2  ? 1 : 0) << 5);
        header[0] |= ((rsv3  ? 1 : 0) << 4);
        header[0] |= (opCode & 0xf);

        header[1] |= ((isMasked ? 1 : 0) << 7);

        CFIndex length = payload != NULL ? CFDataGetLength(payload) : 0;
        if (length <= 125) {
            header[1] |= (UInt8) length;
            WebSocketFrameAppend(self, header, 2);
        } else {
            if (length < 65535) {
                header[1] |= 126;
                *((UInt16 *) &header[2]) = OSSwapBigToHostInt16(length);
                WebSocketFrameAppend(self, header, 2 + 2);
            } else {
                header[1] |= 127;
                *((UInt64 *) &header[2]) = OSSwapBigToHostInt64(length);
                WebSocketFrameAppend(self, header, 2 + 8);
            }
        }

        if (isMasked) {
            UInt8 autoMaskingKey[4] = { 0, 0, 0, 0 };
            if (maskingKey == NULL) {
                * (u_int32_t *) autoMaskingKey = arc4random();
                maskingKey = autoMaskingKey;
            }
            WebSocketFrameAppend(self, maskingKey, 4);
        }

        if (length > 0) {
            WebSocketFrameAppend(self, CFDataGetBytePtr(payload), length);
        }

        WebSocketFrameParse(self);
    }
    return self;
}

WebSocketFrameRef
WebSocketFrameCreateWithPayloadString (CFAllocatorRef allocator, Boolean isMasked, UInt8 *maskingKey, CFStringRef payload) {
    WebSocketFrameRef self = NULL;
    CFDataRef data = CFStringCreateExternalRepresentation(allocator, payload, kCFStringEncodingUTF8, 0);
    if (data != NULL) {
        self = WebSocketFrameCreateWithPayloadData(allocator, kWebSocketFrameOpCodeText, isMasked, maskingKey, data);
        CFRelease(data);
    }
    return self;
}

void
WebSocketFrameDealloc (WebSocketFrameRef self) {
    if_self {
        CFAllocatorRef allocator = self->allocator;
        if (self->data) {
            CFRelease(self->data), self->data = NULL;
        }
        CFAllocatorDeallocate(allocator, self), self = NULL;
        if (allocator != NULL) {
            CFRelease(allocator), allocator = NULL;
        }
    }
}

WebSocketFrameRef
WebSocketFrameRetain (WebSocketFrameRef self) {
    if_self {
        ++self->retainCount;
    }
    return self;
}

void
WebSocketFrameRelease (WebSocketFrameRef self) {
    if_self {
        if (--self->retainCount == 0) {
            WebSocketFrameDealloc(self);
        }
    }
}

Boolean
WebSocketFrameGetIsFin (WebSocketFrameRef self) {
    Boolean result = FALSE;
    if_self_and (self->state == kWebSocketFrameStateReady) {
        result = self->isFin;
    }
    return result;
}

WebSocketFrameOpCode
WebSocketFrameGetOpCode (WebSocketFrameRef self) {
    WebSocketFrameOpCode result = 0;
    if_self_and (self->state == kWebSocketFrameStateReady) {
        result = self->opCode;
    }
    return result;
}

Boolean
WebSocketFrameGetIsMasked (WebSocketFrameRef self) {
    Boolean result = FALSE;
    if_self_and (self->state == kWebSocketFrameStateReady) {
        result = self->isMasked;
    }
    return result;
}

void
WebSocketFrameAppend (WebSocketFrameRef self, const UInt8 *bytes, CFIndex length) {
    if_self {
        CFDataAppendBytes(self->data, bytes, length);
    }
}

UInt64
WebSocketFrameGetPayloadLength (WebSocketFrameRef self) {
    UInt64 result = 0;
    if_self_and (self->state == kWebSocketFrameStateReady) {
        result = self->payloadLength;
    }
    return result;
}

WebSocketFrameState
WebSocketFrameGetState (WebSocketFrameRef self) {
    WebSocketFrameState result = kWebSocketFrameStateNone;
    if_self {
        result = self->state;
    }
    return result;
}

void
WebSocketFrameReset (WebSocketFrameRef self) {
    if_self {
        self->isFin = FALSE;
        self->isRsv1 = FALSE;
        self->isRsv2 = FALSE;
        self->isRsv3 = FALSE;
        self->isMasked = FALSE;
        self->maskingKey[0] = 0;
        self->maskingKey[1] = 0;
        self->maskingKey[2] = 0;
        self->maskingKey[3] = 0;
        self->opCode = 0;
        self->state = kWebSocketFrameStateNone;
        self->payloadOffset = 0;
        self->payloadLength = 0;
        CFDataSetLength(self->data, 0);
    }
}

#pragma mark Private definitions

const char *
__WebSocketFrameOpCodeCString[15] = {
    "kWebSocketFrameOpCodeContinuation", // 0
    "kWebSocketFrameOpCodeText",         // 1
    "kWebSocketFrameOpCodeBinary",       // 2
    "kWebSocketFrameOpCode3",            // 3
    "kWebSocketFrameOpCode4",            // 4
    "kWebSocketFrameOpCode5",            // 5
    "kWebSocketFrameOpCode6",            // 6
    "kWebSocketFrameOpCode7",            // 7
    "kWebSocketFrameOpCodeClose",        // 8
    "kWebSocketFrameOpCodePing",         // 9
    "kWebSocketFrameOpCodePong",         // 10
    "kWebSocketFrameOpCode11",           // 11
    "kWebSocketFrameOpCode12",           // 12
    "kWebSocketFrameOpCode13",           // 13
    "kWebSocketFrameOpCode14"            // 14
};

UInt8
__WebSocketFrameUInt8 (WebSocketFrameRef self, CFIndex offset) {
    UInt8 result = 0;
    if_self_and (offset + sizeof(result) - 1 < CFDataGetLength(self->data)) {
        CFDataGetBytes(self->data, CFRangeMake(offset, sizeof(result)), (UInt8 *) &result);
    }
    return result;
}

UInt16
__WebSocketFrameUInt16 (WebSocketFrameRef self, CFIndex offset) {
    UInt16 result = 0;
    if_self_and (offset + sizeof(result) - 1 < CFDataGetLength(self->data)) {
        CFDataGetBytes(self->data, CFRangeMake(offset, sizeof(result)), (UInt8 *) &result);
        result = OSSwapBigToHostInt16(result);
    }
    return result;
}

UInt32
__WebSocketFrameUInt32 (WebSocketFrameRef self, CFIndex offset) {
    UInt32 result = 0;
    if_self_and (offset + sizeof(result) - 1 < CFDataGetLength(self->data)) {
        CFDataGetBytes(self->data, CFRangeMake(offset, sizeof(result)), (UInt8 *) &result);
        result = OSSwapBigToHostInt32(result);
    }
    return result;
}

UInt64
__WebSocketFrameUInt64 (WebSocketFrameRef self, CFIndex offset) {
    UInt64 result = 0;
    if_self_and (offset + sizeof(result) - 1 < CFDataGetLength(self->data)) {
        CFDataGetBytes(self->data, CFRangeMake(offset, sizeof(result)), (UInt8 *) &result);
        result = OSSwapBigToHostInt64(result);
    }
    return result;
}

WebSocketFrameState
WebSocketFrameParse (WebSocketFrameRef self) {
    WebSocketFrameState result = kWebSocketFrameStateNone;
    if_self {
        
        self->isFin = FALSE;
        self->isRsv1 = FALSE;
        self->isRsv2 = FALSE;
        self->isRsv3 = FALSE;
        self->isMasked = FALSE;
        self->maskingKey[0] = 0;
        self->maskingKey[1] = 0;
        self->maskingKey[2] = 0;
        self->maskingKey[3] = 0;
        self->opCode = 0;
        self->state = kWebSocketFrameState2BytesHeader;
        self->payloadOffset = 0;
        self->payloadLength = 0;
        
        CFIndex n = CFDataGetLength(self->data);
        Boolean loop = TRUE;
        CFIndex i = 0;
        while (loop) {
            switch (self->state) {
                case kWebSocketFrameState2BytesHeader:
                    if (i + 2 <= n) {
                        UInt8 byte1 = __WebSocketFrameUInt8(self, i);
                        UInt8 byte2 = __WebSocketFrameUInt8(self, i + 1);
                        
                        self->isFin  = byte1 & (1 << 7) ? TRUE : FALSE;
                        self->isRsv1 = byte1 & (1 << 6) ? TRUE : FALSE;
                        self->isRsv2 = byte1 & (1 << 5) ? TRUE : FALSE;
                        self->isRsv3 = byte1 & (1 << 4) ? TRUE : FALSE;
                        self->opCode = (WebSocketFrameOpCode) (byte1 & 0xf);
                        
                        self->isMasked = byte2 & (1 << 7) ? TRUE : FALSE;
                        self->payloadLength = (byte2 & 127);
                        
                        switch (self->payloadLength) {
                            case 126:
                                self->state = kWebSocketFrameState16BitLength;
                                break;
                            
                            case 127:
                                self->state = kWebSocketFrameState64BitLength;
                                break;
                                
                            default:
                                self->state = kWebSocketFrameStateMaskingKey;
                                break;
                        }

                        self->payloadOffset += 2;
                        i += 2;
                    } else {
                        self->state = kWebSocketFrameStateError;
                    }
                    break;
                
                case kWebSocketFrameState16BitLength:
                    if (i + 2 <= n) {
                        self->payloadLength = __WebSocketFrameUInt16(self, i);
                        self->state = kWebSocketFrameStateMaskingKey;
                        self->payloadOffset += 2;
                        i += 2;
                    } else {
                        self->state = kWebSocketFrameStateError;
                    }
                    break;
                    
                case kWebSocketFrameState64BitLength:
                    if (i + 8 <= n) {
                        self->payloadLength = __WebSocketFrameUInt64(self, i);
                        self->state = kWebSocketFrameStateMaskingKey;
                        self->payloadOffset += 8;
                        i += 8;
                    } else {
                        self->state = kWebSocketFrameStateError;
                    }
                    break;

                case kWebSocketFrameStateMaskingKey:
                    if (self->isMasked) {
                        if (i + 4 <= n) {
                            self->maskingKey[0] = __WebSocketFrameUInt8(self, i);
                            self->maskingKey[1] = __WebSocketFrameUInt8(self, i + 1);
                            self->maskingKey[2] = __WebSocketFrameUInt8(self, i + 2);
                            self->maskingKey[3] = __WebSocketFrameUInt8(self, i + 3);
                            self->state = kWebSocketFrameStatePayload;
                            self->payloadOffset += 4;
                            i += 4;
                        } else {
                            self->state = kWebSocketFrameStateError;
                        }
                    } else {
                        self->state = kWebSocketFrameStatePayload;
                    }
                    break;
                    
                case kWebSocketFrameStatePayload:
                    if (i + WebSocketFrameGetPayloadLength(self)) {
                        self->state = kWebSocketFrameStateReady;
                        i += WebSocketFrameGetPayloadLength(self);
                    } else {
                        self->state = kWebSocketFrameStateError;
                    }
                    break;

                case kWebSocketFrameStateReady:
                    loop = FALSE;
                    break;

                case kWebSocketFrameStateError:
                    loop = FALSE;
                    break;
                    
                case kWebSocketFrameStateNone:
                    loop = FALSE;
                    break;
            }
        }
        result = self->state;
    }
    return result;
}

void
WebSocketFrameGetPayloadWithRange (WebSocketFrameRef self, CFRange range, UInt8 *buffer, Boolean unmask) {
    if_self_and (self->state == kWebSocketFrameStateReady && range.location >= 0 && (range.location + range.length) <= self->payloadLength) {
        CFIndex location = self->payloadOffset + range.location;
        if (unmask && self->isMasked) {
            const UInt8 *payload = CFDataGetBytePtr(self->data) + location;
            for (CFIndex i = 0; i < range.length; ++i) {
                *(buffer + i) = *(payload + i) ^ (self->maskingKey[(i + range.location) % 4]);
            }
        } else {
            CFDataGetBytes(self->data, CFRangeMake(location, range.length), buffer);
        }
    }
}

CFDataRef
WebSocketFrameCopyPayloadDataWithRange (WebSocketFrameRef self, CFRange range, Boolean unmask) {
    CFMutableDataRef result = NULL;
    if_self_and (self->state == kWebSocketFrameStateReady && range.location >= 0 && (range.location + range.length) <= self->payloadLength) {
        result = CFDataCreateMutable(self->allocator, range.length);
        if (result != NULL) {
            CFDataSetLength(result, range.length);
            WebSocketFrameGetPayloadWithRange(self, range, CFDataGetMutableBytePtr(result), unmask);
        }
    }
    return result;
}

CFStringRef
WebSocketFrameCopyPayloadStringWithRange (WebSocketFrameRef self, CFRange range, CFStringEncoding encoding) {
    CFStringRef result = NULL;
    if_self_and (self->state == kWebSocketFrameStateReady && range.location >= 0 && (range.location + range.length) <= self->payloadLength) {
        
        // TODO: Avoid this allocation.
        CFDataRef data = WebSocketFrameCopyPayloadDataWithRange(self, range, TRUE);
        if (data) {
            result = CFStringCreateFromExternalRepresentation(self->allocator, data, encoding);
            CFRelease(data);
        }
    }
    return result;
}

CFStringRef
WebSocketFrameCopyPayloadString (WebSocketFrameRef self, CFStringEncoding encoding) {
    CFStringRef result = NULL;
    if_self_and (self->state == kWebSocketFrameStateReady) {
        result = WebSocketFrameCopyPayloadStringWithRange(self, CFRangeMake(0, self->payloadLength), encoding);
    }
    return result;
}

const UInt8 *
WebSocketFrameGetBytesPtr (WebSocketFrameRef self) {
    const UInt8 *result = NULL;
    if_self {
        result = CFDataGetBytePtr(self->data);
    }
    return result;
}

