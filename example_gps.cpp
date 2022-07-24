#include <csignal>
#include <string>
#include <iostream>
#include <fstream>

#include "websocket_server.h"

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

	const uint16_t PORT=22002;

	try {
		WebsocketServer server("127.0.0.1", PORT);
		//WebSocketServer server(INADDR_LOOPBACK, PORT);//INADDR_ANY
		serverPtr=&server;

		std::ifstream gpsData;
		gpsData.open("Websocket_GPS_path/GPS_data.txt");

		std::string coordinates;
		coordinates.reserve(100);
		while(true){
			if(exitLoop){
				break;
			}
			if(server.clientsListening()>0){
				std::getline(gpsData, coordinates);
				if(coordinates.length()<10){
					break;
				}
				server.pushData(coordinates);
				coordinates.clear();
			}
			usleep(1000000);
		}
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
