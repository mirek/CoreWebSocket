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

#ifndef __CORE_WEB_SOCKET_CORE_WEB_SOCKET_H__
#define __CORE_WEB_SOCKET_CORE_WEB_SOCKET_H__ 1

#define kWebSocketDefaultPort        80
#define kWebSocketDefaultSecurePort 443

#define kWebSocketScheme       "ws"
#define kWebSocketSecureScheme "wss"

#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
#include <netdb.h>

#import <CommonCrypto/CommonDigest.h>

#if (TARGET_OS_IPHONE)
#include <CFNetwork/CFNetwork.h>
#else
#include <CoreServices/CoreServices.h>
//#include <openssl/evp.h>
//#include <openssl/err.h>
#endif

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "CoreWebSocket/WebSocketTypes.h"
#include "CoreWebSocket/WebSocket.h"
#include "CoreWebSocket/WebSocketClient.h"
#include "CoreWebSocket/cuEnc64.h"

#endif

