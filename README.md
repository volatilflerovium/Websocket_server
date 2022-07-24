# Websocket_server

# Quick start

Example:

On the server side:

	MyServer.subscribeCommand("Cmd_1", [](const std::string& params){
			// parse the params string
			// do something with those values
			// return the result to the client: 
			return theResult;
	});

In your Javascript:

	document.getElementById('ButtonCmd').addEventListener('click', function(){
		var someData=getData();
		MyWebsocket.sendMsg('Cmd_1:'+someData);
	});


# COPYRIGHT
WebsocketServer is released into the public domain by the copyright holder.

