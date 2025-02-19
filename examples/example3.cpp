#include <csignal>
#include <string>
#include <iostream>
#include <fstream>

#include "websocket/websocket_server.h"


#ifdef DATA_FILE //DATA_FILE set in CMakeLists.txt
#define GPS_FILE_DATA DATA_FILE
#else
#define GPS_FILE_DATA "../Websocket_GPS_path/GPS_data.txt"
#endif

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

	const uint16_t PORT=22003;

	try {
		WebsocketServer server("127.0.0.1", PORT);
		serverPtr=&server;

		Command cmd1=[](const std::string& data){
			return "Do some calculations and return result as string.";
		};
		
		server.subscribeCommand("Command_1", cmd1);

		server.subscribeCommand("Command_2", [](const std::string& data){
			return "Command 2: "+data;
		});

		server.subscribeCommand("Command_3", [](const std::string& data){
			return "Command 3 do operation 1+2=3";
		});

		std::thread processReq(&WebsocketServer::processRequests, &server);
		
		std::ifstream gpsData;
		gpsData.open(GPS_FILE_DATA);

		if(!gpsData.is_open()){
			std::string coordinates;
			coordinates.reserve(100);
			while(true){
				if(exitLoop){
					break;
				}
				if(server.clientsListening()){
					std::getline(gpsData, coordinates);
					server.pushData(coordinates);
					coordinates.clear();
				}
				usleep(1000000);
			}
		}

		processReq.join();
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
