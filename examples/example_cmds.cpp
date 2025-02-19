#include <csignal>
#include <string>
#include <iostream>
#include <fstream>

#include "websocket/websocket_server.h"

sig_atomic_t signal_caught = 0;
bool exitLoop=false;

WebsocketServer* serverPtr=nullptr;

void sigint_handler(int sig){
	exitLoop=true;
	if(serverPtr){
		serverPtr->stop();
	}
	std::cout<<"Terminating server\n";
}

//######################################################################
//######################################################################

int main(int argc, char** argv)
{
	signal(SIGINT, &sigint_handler);

	const uint16_t PORT=22001;

	try {
		WebsocketServer server("127.0.0.1", PORT);
		serverPtr=&server;

		Command cmd1=[](const std::string& data){
			return "Do some calculations and return result as string.";
		};
		
		server.subscribeCommand("Command_1", cmd1);

		server.subscribeCommand("Command_2", [](const std::string& params){
			std::size_t n=params.find(",");
		
			int x=0, y=0;
			if(n!=std::string::npos){
				x=std::stoi(params.substr(0, n));
				y=std::stoi(params.substr(n+1));
			}
			return std::to_string(x)+"+"+std::to_string(y)=std::to_string(x+y);
		});

		server.subscribeCommand("Command_3", [](const std::string& data){
			return "Command 3 do operation 1+2=3";
		});

		server.processRequests();
	}
	catch (const std::exception &e) {
		std::cerr << "Caught unhandled exception:\n";
		std::cerr << " - what(): " << e.what() << '\n';
	}
	catch (...) {
		std::cerr << "Caught unknown exception\n";
	}

	return 0;
}
