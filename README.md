# Websocket_server

## How to build it

```
mkdir build
cd build
cmake .. [-DBUILD_SHARED_LIBS=ON]
```

## Quick start

Example:

On the server side:
```
	WebsocketServer server(some_ip, some_port);

	MyServer.subscribeCommand("Cmd_1", [](const std::string& params){
			// parse the parameter string
			// do something with those values
			// return the result to the client: 
			return theResult;
	});
```

In your Javascript:
```
	document.getElementById('ButtonCmd').addEventListener('click', function(){
		var someData=getData();
		MyWebsocket.sendMsg('Cmd_1:'+someData);
	});
```

## COPYRIGHT
WebsocketServer is released into the public domain by the copyright holder.

