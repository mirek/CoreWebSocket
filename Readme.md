# CoreWebSocket - Core Foundation based WebSocket Library for iOS/OSX

CoreWebSocket is an C language, Core Foundation based library for iOS and Mac OSX. It can be used from Objective-C.

WebSocket enables low latency, bi-directional, full-duplex communication channel over TCP with web browser. It works
with all modern web browsers including Safari, Chrome, Firefox and Opera. It works with iOS Safari as well.

## Installation

To get the library using git in your project:

    git submodule add git://github.com/mirek/CoreWebSocket.git CoreWebSocket

Then to add the library to your project
    
1. Add CoreWebSocket.xcodeproj to project
2. In to targets Build Phases
    1. In Target Dependencies add
          + CoreWebSocket
    2. In Link Binary With Libraries add
          + CFNetwork.framework
          + CoreServices.framework
          + libcrypto.dylib
          + CoreWebSocket.framework
    3. Add Build Phase > Add Copy Files
         1. Set Destination to be Frameworks
         2. Add CoreWebSocket.framework
3. Clean
4. Build CoreWebSocket
5. Build your app

# Usage

Exmaple AppDelegate.m


	#import "AppDelegate.h"
	#include "CoreWebSocket/CoreWebSocket.h"

	@implementation AppDelegate

	void Callback(WebSocketRef self, WebSocketClientRef client, CFStringRef value) {
	    if (value) {
	        CFShow(value);
	    }
	}

	- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
	{
	    WebSocketRef webSocket = WebSocketCreateWithHostAndPort(NULL, kWebSocketHostAny, 6001, NULL);
	    if (webSocket) {
	        webSocket->callbacks.didClientReadCallback = Callback;
	    }
	}

	@end

## Web Browser Usage

    <!DOCTYPE html>
    <html>
      <head>
        <meta http-equiv="Content-type" content="text/html; charset=utf-8">
        <script type="text/javascript" charset="utf-8">
  
          // WebSocket
          var ws = null;
  
          // When the page is loaded...
          window.addEventListener('load', function(e) {
    
            // Check if the browser supports WebSockets...
            if ("WebSocket" in window) {
      
              // ...it does, let's connect to localhost default port.
              // Make sure Quartz Composer composition is running with
              // the WebSocket Patch set to port 60001. 
              ws = new WebSocket('ws://localhost:60001');
        
              // Invoked when there was an error with the connection. 
              ws.onerror = function(e) {
                console.log('error', e);
              }
        
              // Invoked when the socket has been opened successfully.
              ws.onopen = function(e) {
                console.log('open', e);
              }
        
              // Callback invoked when incoming messages arrive. Event `data` attribute
              // holds the string passed. WebSocket in current spec supports utf8 text-based
              // communication only. Binary data  is base64 encoded.
              ws.onmessage = function(e) {
                var json = JSON.parse(e.data);
                console.log('message', json);
              }
        
              // Invoked when the socket has been closed
              ws.onclose = function(e) {
                console.log('close', e);
              }
        
            } else {
      
              // ...seems like the web browser doesn't support WebSockets.
              alert('WebSocket not supported by your browser, use Safari, Chrome or Firefox');
        
            }
          }, false);
    
        </script>
      </head>
    </html>

## License

Unless otherwise stated, the code is released under Open Source MIT License, (c) Copyright Mirek Rusin <mirek [at] me [dot] com>

Parts of the source code (particularly Base64 encoding functions) have been copied from http://opensource.apple.com which are released
under Apple Public Source License 2.0 http://www.opensource.apple.com/apsl.

Portions of the copied source code could be modified.
