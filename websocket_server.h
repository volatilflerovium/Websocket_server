/*********************************************************************
* WebsocketServer class                       								*
*                                                                    *
* Version: 1.0                                                       *
* Date:    23-07-2022                                                *
* Author:  Dan Machado <dan-machado@yandex.com>                      *
**********************************************************************/
#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <iostream>
#include <string>
#include <cstring>
#include <array>
#include <queue>
#include <map>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "sha1.hpp"

const unsigned int MAX_SIZE=2048;

//======================================================================

typedef std::function<std::string(const std::string&)> Command;

class WebsocketServer
{
	public:
		WebsocketServer(const char* ip, uint16_t port);
		WebsocketServer(unsigned int inaddr, uint16_t port);
		virtual ~WebsocketServer();

		/**
		* Stop the server from listening and accepting new requests
		* and stop executing commands.
		*/ 
		void stop();

		/**
		* Return the current number of clients.
		*/
		int clientsListening() const;

		/**
		* Subscribe commands to be executed on the server side and result
		* to be sent back to the specific client who requested it.
		* 
		* Clients would be able to send requests like:
		* "Command_Name: extra_data" 
		* 
		* @param commandName unique name for the command
		* @param cmd a function that takes a std::string as argument and 
		* returns a std::string which the reserver will send back to the client
		* who request the command.
		* 
		* @see example_cmds.cpp
		* @see README
		*/
		void subscribeCommand(const char* commandName, Command cmd);

		/**
		* Send data to all the clients listening.
		* 
		* @param data string to be sent to all the clients.
		* @return return true if data was set to all the clients, 
		* false otherwise.
		* 
		* @see example_gps.cpp
		*/
		bool pushData(const std::string& data);

		/**
		* Loop through client requests, execute respective subscribed commands 
		* and send back the result to the respective client. 
		* 
		* This is a blocking method.
		*/
		void processRequests();

	private:
		struct Sockaddr
		{
			Sockaddr(const char* ip){
				m_addr.sin_addr.s_addr=inet_addr(ip);
			}
		
			Sockaddr(unsigned int inaddr){
				m_addr.sin_addr.s_addr=htonl(inaddr);
			}

			sockaddr_in m_addr;
		};
		
		struct Client
		{
			Client(int fd, std::string&& command)
			:m_command(std::move(command)),
			m_fd(fd)
			{}
		
			std::string m_command;
			int m_fd;
		};

		Sockaddr m_sockAddr;
		std::queue<Client> m_requests;
		std::map<const std::string, Command> m_commands;
		SHA1 m_sha1;
		std::mutex m_controlMutex;
		std::condition_variable m_cv;
		std::unique_lock<std::mutex> m_ulock;
		std::array<char, MAX_SIZE> m_recBuffer;
		unsigned char m_pack[MAX_SIZE];
		pollfd m_fds[200];
		char m_hashSha1[21];
		std::thread* m_trd;  // look mummy he is using a raw pointer!
		char* m_encode64;    // look mummy look! he did it again!
		unsigned int m_dataLength;
		int m_fd;
		int m_nfds;
		int m_recDataLength;
		int m_buffer64Size;
		bool m_exit;

		static const char* base64Index;

		bool getData(int client);
		ssize_t sendData(int client, const std::string& text);
		void encoder(unsigned char a, unsigned char b, unsigned char c, int j);		

		WebsocketServer(WebsocketServer::Sockaddr sockAddr, uint16_t port);
		void listening();
		void encodeData(const std::string& text);
		bool handShake(int client);
		std::string unmask();
		const char* base64Encode(const char* input);
		void rawHash(const std::string& str);		
};

//----------------------------------------------------------------------

inline WebsocketServer::WebsocketServer(const char* ip, uint16_t port)
:WebsocketServer(Sockaddr(ip), port)
{}

//----------------------------------------------------------------------

inline WebsocketServer::WebsocketServer(unsigned int inaddr, uint16_t port)	
:WebsocketServer(Sockaddr(inaddr), port)
{}

//----------------------------------------------------------------------

inline WebsocketServer::~WebsocketServer()
{
	delete[] m_encode64;
	stop();
	m_trd->join();
	delete m_trd;
	for (int i = 0; i<m_nfds; i++){
		if(m_fds[i].fd>-1){
			close(m_fds[i].fd);
		}
	}
}

//----------------------------------------------------------------------

inline void WebsocketServer::stop()
{
	m_exit=true;
	m_cv.notify_one();
}

//----------------------------------------------------------------------

inline int WebsocketServer::clientsListening() const
{
	return m_nfds;
}

//----------------------------------------------------------------------

inline void WebsocketServer::subscribeCommand(const char* commandName, Command cmd)
{
	m_commands[std::string(commandName)]=cmd;
}

//----------------------------------------------------------------------

inline bool WebsocketServer::getData(int client)
{
	m_recDataLength=recv(client, m_recBuffer.data(), m_recBuffer.size()-1, 0);
	m_recBuffer[MAX_SIZE-1]=0;

	return m_recDataLength<1;
}

//----------------------------------------------------------------------

inline ssize_t WebsocketServer::sendData(int client, const std::string& data)
{
	if(data.length()==0){
		return 0;
	}
	encodeData(data);
	return ::send(client, m_pack, m_dataLength, 0);
}

//----------------------------------------------------------------------

inline void WebsocketServer::encoder(unsigned char a, unsigned char b, unsigned char c, int j)
{
	m_encode64[j++]=base64Index[a>>2];
	m_encode64[j++]=base64Index[((a&3)<<4) | (b>>4)];
	m_encode64[j++]=base64Index[((b&15)<<2) | (c>>6)];				
	m_encode64[j++]=base64Index[c & 63];		
}

//----------------------------------------------------------------------

#endif
