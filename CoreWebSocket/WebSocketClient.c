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

#include "WebSocketClient.h"
//#include <CoreWebSocket/WebSocketClient.h>

#define if_self if (self)

#pragma mark Write

// Internal, write provided buffer as websocket frame [0x00 buffer 0xff]
CFIndex
__WebSocketClientWriteFrameWithData (WebSocketClientRef client, WebSocketFrameOpCode opCode, Boolean isMasked, UInt8 *maskingKey, CFDataRef payload) {
    CFIndex result = 0;
    WebSocketFrameRef frame = WebSocketFrameCreateWithPayloadData(client->allocator, opCode, isMasked, maskingKey, payload);
    if (frame != NULL) {
        CFIndex length = CFDataGetLength(frame->data);
        while (CFWriteStreamCanAcceptBytes(client->write) && (result < length)) {
            CFIndex didWrite = CFWriteStreamWrite(client->write, WebSocketFrameGetBytesPtr(frame) + result, length - result);
            if (didWrite > 0) {
                result += didWrite;
            }
        }
        WebSocketFrameDealloc(frame);
    }
    return result;
}

// Write data as websocket frame (with binary frame opcode).
CFIndex
WebSocketClientWriteWithData (WebSocketClientRef self, CFDataRef value) {
    return __WebSocketClientWriteFrameWithData(self, kWebSocketFrameOpCodeBinary, FALSE, NULL, value);
}

// Write UTF-8 encoded string as a websocket frame (with text frame opcode).
CFIndex
WebSocketClientWriteWithString (WebSocketClientRef self, CFStringRef value) {
    CFIndex result = -1;
    if_self {
        if (value) {
            CFDataRef data = CFStringCreateExternalRepresentation(self->allocator, value, kCFStringEncodingUTF8, 0);
            if (data) {
                result = __WebSocketClientWriteFrameWithData(self, kWebSocketFrameOpCodeText, FALSE, NULL, data);
                CFRelease(data);
            }
        }
    }
    return result;
}

// Write UTF-8 encoded string as a websocket frame (with text frame opcode).
CFIndex
WebSocketClientWriteWithFormat (WebSocketClientRef self, CFStringRef fmt, ...) {
    CFIndex result = -1;
    if_self if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        CFStringRef string = CFStringCreateWithFormatAndArguments(self->allocator, NULL, fmt, args);
        va_end(args);
        if (string != NULL) {
            result = WebSocketClientWriteWithString(self, string);
            CFRelease(string);
        }
    }
    return result;
}

#pragma mark Read callback

Boolean
__WebSocketClientWriteHandShake (WebSocketClientRef client);

void
__WebSocketClientReadFrame (WebSocketClientRef self, CFReadStreamRef stream) {

    // Reset last frame if it has been completed.
    if (WebSocketFrameGetState(self->frame) == kWebSocketFrameStateReady) {
        WebSocketFrameReset(self->frame);
    }

    // Did handshake already and there are bytes to read.
    // It's an incomming message.
    {
        CFIndex size = 64 * 1024;
        UInt8 *buffer = (UInt8 *) CFAllocatorAllocate(self->allocator, size, 0);
        if (buffer != NULL) {
            CFIndex didRead = 0;
            while (CFReadStreamHasBytesAvailable(self->read)) {
                if ((didRead = CFReadStreamRead(stream, buffer, size)) != -1) {
                    WebSocketFrameAppend(self->frame, buffer, didRead);
                } else {
                    break;
                }
            }
            CFAllocatorDeallocate(self->allocator, buffer);
        }
    }

    if (WebSocketFrameParse(self->frame) == kWebSocketFrameStateReady) {
        CFStringRef string = WebSocketFrameCopyPayloadString(self->frame, kCFStringEncodingUTF8);
        if (string) {
            self->webSocket->callbacks.didClientReadCallback(self->webSocket, self, string);
            CFRelease(string);
        }
    }
}

void
__WebSocketClientReadCallBack (CFReadStreamRef stream, CFStreamEventType eventType, void *info) {
    WebSocketClientRef self = (WebSocketClientRef) info;
    if (self) {
        switch (eventType) {
            case kCFStreamEventOpenCompleted:
                break;
                
            case kCFStreamEventHasBytesAvailable:
                if (!self->didReadHandShake) {
                    if (__WebSocketClientReadHandShake(self)) {
                        if (!self->didWriteHandShake) {
                            if (CFWriteStreamCanAcceptBytes(self->write)) {
                                if (__WebSocketClientWriteHandShake(self)) {
                                    //                  printf("Successfully written handshake\n");
                                } else {
                                    printf("TODO: Error writting handshake\n");
                                }
                            } else {
                                //                printf("TODO: Didn't handshake and client doesn't accept bytes yet. Write callback will handle writting handshake as soon as we can write.\n");
                            }
                        } else {
                            printf("TODO: Just read handshake and handshake already written, shouldn't happen, fault?\n");
                        }
                        __WebSocketAppendClient(self->webSocket, self);
                    } else {
                        printf("TODO: Didn't read handshake and __WebSocketClientReadHandShake failed.\n");
                    }
                } else {
                    
                    __WebSocketClientReadFrame(self, stream);
                    
                }
                break;
                
            case kCFStreamEventErrorOccurred:
                break;
                
            case kCFStreamEventEndEncountered:
                break;
                
            default:
                break;
        }
    }
}

Boolean
__WebSocketClientWriteWithHTTPMessage (WebSocketClientRef client, CFHTTPMessageRef message) {
    Boolean result = FALSE;
    if (client && message) {
        CFDataRef data = CFHTTPMessageCopySerializedMessage(message);
        if (data) {
            CFIndex written = CFWriteStreamWrite(client->write, CFDataGetBytePtr(data), CFDataGetLength(data));
            if (written == CFDataGetLength(data)) {
                result = TRUE; // TODO: do it properly
            }
            client->didWriteHandShake = 1;
            CFRelease(data);
        }
    }
    return result;
}

Boolean
__WebSocketClientWriteHandShakeDraftIETF_HYBI_00 (WebSocketClientRef client) {
    Boolean result = FALSE;
    if (client) {
        if (client->protocol == kWebSocketProtocolDraftIETF_HYBI_00) {
            CFStringRef key1 = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Sec-Websocket-Key1"));
            CFStringRef key2 = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Sec-Websocket-Key2"));
            CFDataRef key3 = CFHTTPMessageCopyBody(client->handShakeRequestHTTPMessage);
            CFStringRef origin = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Origin"));
            CFStringRef host = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Host"));
            
            CFHTTPMessageRef response = CFHTTPMessageCreateEmpty(NULL, 0);
            CFHTTPMessageAppendBytes(response, (const UInt8 *)"HTTP/1.1 101 Web Socket Protocol Handshake\r\n", 44);
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Upgrade"), CFSTR("WebSocket"));
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Connection"), CFSTR("Upgrade"));
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Sec-Websocket-Origin"), origin);
            
            CFMutableStringRef location = CFStringCreateMutable(client->allocator, 0);
            CFStringAppend(location, CFSTR("ws://"));
            CFStringAppend(location, host);
            CFStringAppend(location, CFSTR("/"));
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Sec-Websocket-Location"), location);
            CFRelease(location);
            
            // Set MD5 hash
            {
                CFMutableDataRef mutable = CFDataCreateMutable(client->allocator, 0);
                __WebSocketDataAppendMagickNumberWithKeyValueString(mutable, key1);
                __WebSocketDataAppendMagickNumberWithKeyValueString(mutable, key2);
                CFDataAppendBytes(mutable, CFDataGetBytePtr(key3), CFDataGetLength(key3));
                CFDataRef data = __WebSocketCreateMD5Data(client->allocator, mutable);
                CFHTTPMessageSetBody(response, data);
                CFRelease(mutable);
                CFRelease(data);
            }
            
            result = __WebSocketClientWriteWithHTTPMessage(client, response) ? TRUE : FALSE;
            
            CFRelease(response);
            
            CFRelease(host);
            CFRelease(origin);
            CFRelease(key3);
            CFRelease(key2);
            CFRelease(key1);
        }
    }
    return result;
}

// The source code has been copied and modified from
// http://www.opensource.apple.com/source/CFNetwork/CFNetwork-128/HTTP/CFHTTPAuthentication.c
// See _CFEncodeBase64 function. The source code has been released under
// Apple Public Source License Version 2.0 http://www.opensource.apple.com/apsl/
CFStringRef
__WebSocketCreateBase64StringWithData (CFAllocatorRef allocator, CFDataRef inputData) {
	unsigned outDataLen;
	CFStringRef result = NULL;
	unsigned char *outData = cuEnc64(CFDataGetBytePtr(inputData), (unsigned int)CFDataGetLength(inputData), &outDataLen);
	if(outData) {
		// current cuEnc64 appends \n and NULL, trim them
		unsigned char *c = outData + outDataLen - 1;
		while((*c == '\n') || (*c == '\0')) {
			c--;
			outDataLen--;
		}
		result = CFStringCreateWithBytes(allocator, outData, outDataLen, kCFStringEncodingASCII, FALSE);
		free(outData);
	}
	return result;
}

Boolean
__WebSocketClientWriteHandShakeDraftIETF_HYBI_06 (WebSocketClientRef client) {
    Boolean success = 0;
    if (client) {
        if (client->protocol == kWebSocketProtocolDraftIETF_HYBI_06) {
            
            CFStringRef key = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Sec-WebSocket-Key"));
            CFStringRef keyWithMagick = CFStringCreateWithFormat(client->allocator, NULL, CFSTR("%@%@"), key, CFSTR("258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
            CFDataRef keyWithMagickSHA1 = __WebSocketCreateSHA1DataWithString(client->allocator, keyWithMagick, kCFStringEncodingUTF8);
            CFStringRef keyWithMagickSHA1Base64 = __WebSocketCreateBase64StringWithData(client->allocator, keyWithMagickSHA1);
            
            CFStringRef origin = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Sec-WebSocket-Origin"));
            CFStringRef host = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Host"));
            
            CFHTTPMessageRef response = CFHTTPMessageCreateEmpty(NULL, 0);
            CFHTTPMessageAppendBytes(response, (const UInt8 *)"HTTP/1.1 101 Switching Protocols\r\n", 44);
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Upgrade"), CFSTR("websocket"));
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Connection"), CFSTR("Upgrade"));
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Sec-WebSocket-Accept"), keyWithMagickSHA1Base64);
            
            success = __WebSocketClientWriteWithHTTPMessage(client, response);
            
            CFRelease(response);
            
            CFRelease(keyWithMagickSHA1Base64);
            CFRelease(keyWithMagickSHA1);
            CFRelease(keyWithMagick);
            CFRelease(key);
            CFRelease(origin);
            CFRelease(host);
        }
    }
    return success;
}

Boolean
__WebSocketClientWriteHandShakeRFC6455_13 (WebSocketClientRef client) {
    Boolean success = 0;
    if (client) {
        if (client->protocol == kWebSocketProtocol_RFC6455_13) {
            
            CFStringRef key = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Sec-WebSocket-Key"));
            CFStringRef keyWithMagick = CFStringCreateWithFormat(client->allocator, NULL, CFSTR("%@%@"), key, CFSTR("258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
            CFDataRef keyWithMagickSHA1 = __WebSocketCreateSHA1DataWithString(client->allocator, keyWithMagick, kCFStringEncodingUTF8);
            CFStringRef keyWithMagickSHA1Base64 = __WebSocketCreateBase64StringWithData(client->allocator, keyWithMagickSHA1);
            
//            CFStringRef host = CFHTTPMessageCopyHeaderFieldValue(client->handShakeRequestHTTPMessage, CFSTR("Host"));
            
            CFHTTPMessageRef response = CFHTTPMessageCreateEmpty(NULL, 0);
            CFHTTPMessageAppendBytes(response, (const UInt8 *)"HTTP/1.1 101 Switching Protocols\r\n", 44);
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Upgrade"), CFSTR("websocket"));
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Connection"), CFSTR("Upgrade"));
            CFHTTPMessageSetHeaderFieldValue(response, CFSTR("Sec-WebSocket-Accept"), keyWithMagickSHA1Base64);
            
            success = __WebSocketClientWriteWithHTTPMessage(client, response);
            
            CFRelease(response);
            
            CFRelease(keyWithMagickSHA1Base64);
            CFRelease(keyWithMagickSHA1);
            CFRelease(keyWithMagick);
            CFRelease(key);
//            CFRelease(host);
        }
    }
    return success;
}

Boolean
__WebSocketClientWriteHandShake (WebSocketClientRef self) {
    Boolean success = 0;
    if (self->didReadHandShake) {
        if (!self->didWriteHandShake) {
            switch (self->protocol) {
                case kWebSocketProtocolDraftIETF_HYBI_00:
                    success = __WebSocketClientWriteHandShakeDraftIETF_HYBI_00(self);
                    break;
                    
                case kWebSocketProtocolDraftIETF_HYBI_06:
                    success = __WebSocketClientWriteHandShakeDraftIETF_HYBI_06(self);
                    break;
                    
                case kWebSocketProtocol_RFC6455_13:
                    success = __WebSocketClientWriteHandShakeRFC6455_13(self);
                    break;
                    
                default:
                    printf("Unknown protocol, can't write handshake. TODO: disconnect\n");
                    // Unknown protocol, can't write handshake
                    break;
            }
        }
    }
    return success;
}

void
__WebSocketClientWriteCallBack (CFWriteStreamRef stream, CFStreamEventType eventType, void *info) {
    WebSocketClientRef client = info;
    if (client) {
        switch (eventType) {
                
            case kCFStreamEventCanAcceptBytes:
                if (!client->didWriteHandShake && client->didReadHandShake)
                    __WebSocketClientWriteHandShake(client);
                break;
                
            case kCFStreamEventEndEncountered:
                break;
                
            case kCFStreamEventErrorOccurred:
                printf("kCFStreamEventErrorOccurred (write)\n");
                CFErrorRef error = CFWriteStreamCopyError(stream);
                if (error) {
                    CFShow(error);
                    CFRelease(error);
                }
                break;
                
            default:
                break;
        }
    }
}

#pragma mark Lifecycle

WebSocketClientRef
WebSocketClientCreate (WebSocketRef webSocket, CFSocketNativeHandle handle) {
    WebSocketClientRef self = NULL;
    if (webSocket) {
        self = CFAllocatorAllocate(webSocket->allocator, sizeof(WebSocketClient), 0);
        if (self) {
            self->allocator = webSocket->allocator ? CFRetain(webSocket->allocator) : NULL;
            self->retainCount = 1;
            
            WebSocketRetain(webSocket), self->webSocket = webSocket;
            self->handle = handle;

            self->frame = WebSocketFrameCreate(self->allocator);
            
            self->read = NULL;
            self->write = NULL;
            
            self->context.version = 0;
            self->context.info = self;
            self->context.copyDescription = NULL;
            self->context.retain = NULL;
            self->context.release = NULL;
            
            self->handShakeRequestHTTPMessage = NULL;
            self->didReadHandShake = 0;
            self->didWriteHandShake = 0;
            self->protocol = kWebSocketProtocolUnknown;
            
            CFStreamCreatePairWithSocket(self->allocator, handle, &self->read, &self->write);
            if (!self->read || !self->write) {
                close(handle);
                fprintf(stderr, "CFStreamCreatePairWithSocket() failed, %p, %p\n", read, write);
            } else {
                //        printf("ok\n");
            }

            CFOptionFlags flags = kCFStreamEventOpenCompleted | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered;
            CFReadStreamSetClient(self->read, flags | kCFStreamEventHasBytesAvailable, __WebSocketClientReadCallBack, &self->context);
            CFWriteStreamSetClient(self->write, flags | kCFStreamEventCanAcceptBytes, __WebSocketClientWriteCallBack, &self->context);
            
            CFReadStreamScheduleWithRunLoop(self->read, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
            CFWriteStreamScheduleWithRunLoop(self->write, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
            
            if (!CFReadStreamOpen(self->read)) {
                printf("couldn't open read stream\n");
            } else {
                //        printf("opened read stream\n");
            }
            
            if (!CFWriteStreamOpen(self->write)) {
                printf("couldn't open write stream\n");
            } else {
                //        printf("opened write stream\n");
            }
        }
    }
    return self;
}

void
WebSocketClientRetain (WebSocketClientRef self) {
    if_self {
        ++self->retainCount;
    }
}

void
WebSocketClientRelease (WebSocketClientRef self) {
    if_self {
        if (--self->retainCount == 0) {
            CFAllocatorRef allocator = self->allocator;
            
            if (self->read) {
                if (CFReadStreamGetStatus(self->read) != kCFStreamStatusClosed)
                    CFReadStreamClose(self->read);
                CFRelease(self->read);
                self->read = NULL;
            }
            
            if (self->write) {
                if (CFWriteStreamGetStatus(self->write) != kCFStreamStatusClosed)
                    CFWriteStreamClose(self->write);
                CFRelease(self->write);
                self->write = NULL;
            }
            
            if (self->frame != NULL) {
                WebSocketFrameRelease(self->frame), self->frame = NULL;
            }
            
            CFAllocatorDeallocate(allocator, self);
            self = NULL;
            
            if (allocator)
                CFRelease(allocator);
        }
    }
}

#pragma Handshake

// Return magic number for the key needed to generate handshake hash
uint32_t
__WebSocketGetMagicNumberWithKeyValueString (CFStringRef string) {
    uint32_t magick = -1;
    if (string) {
        UInt8 buffer[__WebSocketMaxHeaderKeyLength];
        CFIndex usedBufferLength = 0;
        char numberBuffer[__WebSocketMaxHeaderKeyLength];
        memset(numberBuffer, 0, sizeof(numberBuffer));
        CFIndex usedNumberBufferLength = 0;
        CFStringGetBytes(string, CFRangeMake(0, CFStringGetLength(string)), kCFStringEncodingASCII, 0, 0, buffer, sizeof(buffer), &usedBufferLength);
        UInt32 number = 0;
        UInt32 spaces = 0;
        for (int i = 0; i < usedBufferLength; i++) {
            if (buffer[i] >= '0' && buffer[i] <= '9')
                numberBuffer[usedNumberBufferLength++] = buffer[i];
            if (buffer[i] == ' ')
                spaces++;
        }
        if (spaces > 0) {
            number = (UInt32)strtoul(numberBuffer, NULL, 10);
            magick = number / spaces;
        }
    }
    return magick;
}

// Appends big-endian uint32 magic number with key string to the mutable data
Boolean
__WebSocketDataAppendMagickNumberWithKeyValueString (CFMutableDataRef data, CFStringRef string) {
    Boolean success = 0;
    if (data && string) {
        uint32_t magick = __WebSocketGetMagicNumberWithKeyValueString(string);
        uint32_t swapped = CFSwapInt32HostToBig(magick);
        CFDataAppendBytes(data, (const void *)&swapped, sizeof(swapped));
        success = 1;
    }
    return success;
}

CFDataRef
__WebSocketCreateMD5Data (CFAllocatorRef allocator, CFDataRef value) {
    unsigned char digest[CC_MD5_DIGEST_LENGTH];
    CC_MD5((unsigned char *)CFDataGetBytePtr(value), (CC_LONG)CFDataGetLength(value), digest);
    return CFDataCreate(allocator, digest, CC_MD5_DIGEST_LENGTH);
}

CFDataRef
__WebSocketCreateSHA1DataWithData (CFAllocatorRef allocator, CFDataRef value) {
    unsigned char digest[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1((unsigned char *)CFDataGetBytePtr(value), (CC_LONG)CFDataGetLength(value), digest);
    return CFDataCreate(allocator, digest, CC_SHA1_DIGEST_LENGTH);
}

CFDataRef
__WebSocketCreateSHA1DataWithString (CFAllocatorRef allocator, CFStringRef value, CFStringEncoding encoding) {
    CFDataRef data = NULL;
    if (value) {
        CFDataRef valueData = CFStringCreateExternalRepresentation(allocator, value, encoding, 0);
        if (valueData) {
            data = __WebSocketCreateSHA1DataWithData(allocator, valueData);
            CFRelease(valueData);
        }
    }
    return data;
}

Boolean
__WebSocketClientHandShakeConsumeHTTPMessage (WebSocketClientRef client) {
    Boolean success = 0;
    if (client) {
        UInt8 buffer[4096];
        CFIndex bytes = 0;
        client->handShakeRequestHTTPMessage = CFHTTPMessageCreateEmpty(client->allocator, 1);
        while (CFReadStreamHasBytesAvailable(client->read)) {
            if ((bytes = CFReadStreamRead(client->read, buffer, sizeof(buffer))) > 0) {
                CFHTTPMessageAppendBytes(client->handShakeRequestHTTPMessage, buffer, bytes);
            } else if (bytes < 0) {
                CFErrorRef error = CFReadStreamCopyError(client->read);
                CFShow(error);
                CFRelease(error);
                goto fin;
            }
        }
        success = 1;
    }

fin:
    return success;
}

// Private method

Boolean
__WebSocketClientHandShakeUpdateProtocolBasedOnHTTPMessage (WebSocketClientRef self) {
    Boolean result = FALSE;
    if (self) {
        
        // Get the protocol version
        self->protocol = kWebSocketProtocolUnknown;
        
        // Is Upgrade header available? It has to for ws...
        CFStringRef upgrade = CFHTTPMessageCopyHeaderFieldValue(self->handShakeRequestHTTPMessage, CFSTR("Upgrade"));
        if (upgrade) {
            if (CFStringCompare(CFSTR("WebSocket"), upgrade, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                CFStringRef version = CFHTTPMessageCopyHeaderFieldValue(self->handShakeRequestHTTPMessage, CFSTR("Sec-WebSocket-Version"));
                if (version) {
                    if (CFStringCompare(CFSTR("6"), version, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                        self->protocol = kWebSocketProtocolDraftIETF_HYBI_06;
                        result = TRUE;
                    } else if (CFStringCompare(CFSTR("13"), version, kCFCompareCaseInsensitive) == kCFCompareEqualTo)  {
                        self->protocol = kWebSocketProtocol_RFC6455_13;
                        result = TRUE;
                    } else {
                        
                        // Version different than 6, we don't know of any other, leave the protocol as unknown
                    }
                    CFRelease(version);
                } else {
                    
                    // Sec-WebSocket-Version header field is missing.
                    // It may be 00 protocol, which doesn't have this field.
                    // 00 protocol has to have Sec-WebSocket-Key1 and Sec-WebSocket-Key2
                    // fields - let's check for those.
                    CFStringRef key1 = CFHTTPMessageCopyHeaderFieldValue(self->handShakeRequestHTTPMessage, CFSTR("Sec-WebSocket-Key1"));
                    if (key1) {
                        CFStringRef key2 = CFHTTPMessageCopyHeaderFieldValue(self->handShakeRequestHTTPMessage, CFSTR("Sec-WebSocket-Key2"));
                        if (key2) {
                            self->protocol = kWebSocketProtocolDraftIETF_HYBI_00;
                            result = TRUE;
                            CFRelease(key2);
                        }
                        CFRelease(key1);
                    } else {
                        
                        // Key2 missing, no version specified = unknown protocol
                    }
                }
            } else {
                
                // Upgrade HTTP field seems to be different from "WebSocket" (case ignored)
            }
            CFRelease(upgrade);
        } else {
            
            // Upgrade HTTP field seems to be absent
        }
    }
    return result;
}

Boolean
__WebSocketClientReadHandShake (WebSocketClientRef self) {
    Boolean result = FALSE;
    if_self {
        if ((result = __WebSocketClientHandShakeConsumeHTTPMessage(self))) {
            if ((result = __WebSocketClientHandShakeUpdateProtocolBasedOnHTTPMessage(self))) {
                
                // Dump http message:
                //
                // CFDictionaryRef headerFields = CFHTTPMessageCopyAllHeaderFields(self->handShakeRequestHTTPMessage);
                // if (headerFields) {
                //    CFRelease(headerFields);
                // }
            }
        }
        self->didReadHandShake = TRUE;
    }
    return result;
}

