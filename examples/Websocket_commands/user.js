var WebsocketClient={
	client:document.getElementById('conversation'),
	sendCommand:function(dataText){
		this.socket.send(dataText);
		this.insertMessage('client sent:', dataText);
	},
	insertMessage:function(name, mgs){
		var container=document.createElement('P');
		var b=document.createElement('B');
		b.appendChild(document.createTextNode(name+': '));
		container.appendChild(b);
		container.appendChild(document.createTextNode(mgs));
		this.client.appendChild(container);			
		this.scrolling();
	},
	scrolling:function(){
		var w=this.client.clientHeight;
		var u=document.getElementById('conversation_wrapper').clientHeight;
		if(w>u){
			document.getElementById('conversation_wrapper').scrollBy(0, w-u);
		}
	},
	socket:null,
	init:function(){
		this.socket = new WebSocket("ws://127.0.0.1:22001");
		this.socket.onerror = function(event) {
			console.error("WebSocket error observed:", event);
		};
		this.socket.onopen = function (event) {
			console.log('Ready');
			document.getElementById('curtain').style.display='none';
		};
		this.socket.onmessage = function (event) {			
			WebsocketClient.insertMessage('server response', event.data);
		};
	},
};
