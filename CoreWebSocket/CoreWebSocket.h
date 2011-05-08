//
//  WebSocket.h
//  WebSocketCore
//
//  Created by Mirek Rusin on 07/03/2011.
//  Copyright 2011 Inteliv Ltd. All rights reserved.
//

#ifndef __CORE_WEB_SOCKET__
#define __CORE_WEB_SOCKET__ 1

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

#include "CoreWebSocket/WebSocket.h"
#include "CoreWebSocket/WebSocketTypes.h"
#include "CoreWebSocket/WebSocketClient.h"
#include "CoreWebSocket/cuEnc64.h"

#endif

