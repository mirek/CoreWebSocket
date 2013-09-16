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

#include "WebSocket.h"

#ifndef likely
#define likely(x)      __builtin_expect((x),1)
#endif

#ifndef unlikely
#define unlikely(x)    __builtin_expect((x),0)
#endif

#define if_likely(x)   if (likely(x))
#define if_self        if (likely((self) != NULL))
#define if_self_and(x) if (likely(((self) != NULL) && (x)))

#pragma mark Lifecycle

WebSocketRef
WebSocketCreateWithHostAndPort (CFAllocatorRef allocator, CFStringRef host, UInt16 port, void *userInfo) {
    WebSocketRef self = CFAllocatorAllocate(allocator, sizeof(WebSocket), 0);
    if (self) {
        self->allocator = allocator ? CFRetain(allocator) : NULL;
        self->retainCount = 1;
        self->userInfo = userInfo;

        self->clientsLength = 1024;
        self->clientsUsedLength = 0;
        if (NULL == (self->clients = CFAllocatorAllocate(allocator, self->clientsLength, 0))) {
            WebSocketRelease(self), self = NULL;
            goto fin;
        }

        // Callbacks
        self->callbacks.didAddClientCallback     = NULL;
        self->callbacks.willRemoveClientCallback = NULL;
        self->callbacks.didClientReadCallback    = NULL;

        // Setup the context;
        CFSocketContext context = {
            .copyDescription = NULL,
            .retain = NULL,
            .release = NULL,
            .version = 0,
            .info = self
        };

        if (NULL == (self->socket = CFSocketCreate(self->allocator, PF_INET, SOCK_STREAM, IPPROTO_TCP, kCFSocketAcceptCallBack, __WebSocketAcceptCallBack, &context))) {
            WebSocketRelease(self), self = NULL;
            goto fin;
        }

        // Re-use local addresses, if they're still in TIME_WAIT
        int yes = 1;
        setsockopt(CFSocketGetNative(self->socket), SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes));

        // Set the port and address we want to listen on
        memset(&self->addr, 0, sizeof(self->addr));
        self->addr.sin_len = sizeof(self->addr);
        self->addr.sin_family = AF_INET;

        if (CFEqual(kWebSocketHostAny, host)) {

            // Host is set to "0.0.0.0", set it to INADDR_ANY
            self->addr.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {

            // Set the host based on provided string. TODO: hostname resolution?
            CFIndex hostCStringLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(host), kCFStringEncodingASCII) + 1;
            char *hostCString = CFAllocatorAllocate(self->allocator, hostCStringLength, 0);
            if (hostCString) {
                if (CFStringGetCString(host, hostCString, hostCStringLength, kCFStringEncodingASCII)) {
                    inet_aton(hostCString, &self->addr.sin_addr);
                } else {

                    // TODO: Couldn't get CString
                }
                CFAllocatorDeallocate(self->allocator, hostCString);
            } else {

                // TODO: Couldn't allocate buffer
            }
        }

        self->addr.sin_port = htons(port);

        CFDataRef address = CFDataCreate(self->allocator, (const void *) &self->addr, sizeof(self->addr));
        if (address) {
            if (CFSocketSetAddress(self->socket, (CFDataRef) address) != kCFSocketSuccess) {
                WebSocketRelease(self), self = NULL;
                CFRelease(address);
                goto fin;
            }
            CFRelease(address);
        }

        // Create run loop source and add it to the current run loop
        CFRunLoopSourceRef source = CFSocketCreateRunLoopSource(self->allocator, self->socket, 0);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
        CFRelease(source);
    }

fin:

    return self;
}

WebSocketRef
WebSocketCreate (CFAllocatorRef allocator, void *userInfo) {
    return WebSocketCreateWithHostAndPort(allocator, kWebSocketHostLoopBack, kWebSocketPortAny, userInfo);
}

void
WebSocketRetain (WebSocketRef self) {
    if_self {
        ++self->retainCount;
    }
}

void
WebSocketRelease (WebSocketRef self) {
    if_self {
        if (--self->retainCount == 0) {
            CFAllocatorRef allocator = self->allocator;

            if (self->clients) {
                while (--self->clientsUsedLength >= 0) {
                    WebSocketClientRelease(self->clients[self->clientsUsedLength]);
                }
                CFAllocatorDeallocate(allocator, self->clients), self->clients = NULL;
            }

            if (self->socket) {
                CFSocketInvalidate(self->socket);
                CFRelease(self->socket), self->socket = NULL;
            }

            CFAllocatorDeallocate(allocator, self), self = NULL;

            if (allocator) {
                CFRelease(allocator), allocator = NULL;
            }
        }
    }
}

UInt16
WebSocketGetPort (WebSocketRef self) {
    UInt16 result = UINT16_MAX;
    if_self_and (self->socket != NULL) {
        struct sockaddr_in sock;
        socklen_t sockLength = sizeof(struct sockaddr_in);
        if (getsockname(CFSocketGetNative(self->socket), (struct sockaddr *) &sock, &sockLength) != -1) {
            result = ntohs(sock.sin_port);
        }
    }
    return result;
}

// Send string frame to all connected clients
void
WebSocketWriteWithString (WebSocketRef self, CFStringRef value) {
    if_self_and (value != NULL) {
        for (CFIndex i = 0; i < self->clientsUsedLength; ++i) {
            WebSocketWriteWithStringAndClientIndex(self, value, i);
        }
    }
}

CFIndex
WebSocketWriteWithStringAndClientIndex (WebSocketRef self, CFStringRef value, CFIndex index) {
    CFIndex result = -1;
    if_self_and (value != NULL && index >= 0 && index < self->clientsUsedLength) {
        result = WebSocketClientWriteWithString(self->clients[index], value);
    }
    return result;
}

WebSocketClientRef
WebSocketGetClientAtIndex (WebSocketRef self, CFIndex index) {
    WebSocketClientRef result = NULL;
    if_self_and (index < self->clientsUsedLength) {
        result = self->clients[index];
    }
    return result;
}

CFIndex
WebSocketGetClientCount (WebSocketRef self) {
    CFIndex result = 0;
    if_self {
        result = self->clientsUsedLength;
    }
    return result;
}

#pragma mark Callbacks

void
WebSocketSetClientReadCallback (WebSocketRef self, WebSocketDidClientReadCallback callback) {
    if_self {
        self->callbacks.didClientReadCallback = callback;
    }
}

#pragma mark Internal, client management

CFIndex
__WebSocketAppendClient (WebSocketRef self, WebSocketClientRef client) {
    CFIndex result = -1;
    if_self_and (client != NULL) {
        WebSocketClientRetain(client), self->clients[self->clientsUsedLength++] = client;
        result = self->clientsUsedLength;
        if (self->callbacks.didAddClientCallback) {
            self->callbacks.didAddClientCallback(self, client);
        }
    }
    return result;
}

CFIndex
__WebSocketRemoveClient (WebSocketRef self, WebSocketClientRef client) {
    CFIndex result = -1;
    if_self_and (client != NULL) {
        for (CFIndex i = 0; i < self->clientsUsedLength; ++i) {
            if_likely (self->clients[i] == client) {

                // Invoke callback before removing client if defined.
                if (self->callbacks.willRemoveClientCallback) {
                    self->callbacks.willRemoveClientCallback(self, client);
                }

                // Swap last client with this one.
                self->clients[i] = self->clients[result = --self->clientsUsedLength];
                WebSocketClientRelease(client);

                break;
            }
        }
    }
    return result;
}

#pragma mark Callbacks

void __WebSocketAcceptCallBack (CFSocketRef socket, CFSocketCallBackType type, CFDataRef address, const void *sock, void *info) {
    WebSocketRef self = (WebSocketRef) info;
    if_self {
        WebSocketClientRef client = WebSocketClientCreate(self, *((CFSocketNativeHandle *) sock));
        printf("adding %p client\n", client);
    }
}
