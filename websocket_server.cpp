#include "websocket_server.h"

//----------------------------------------------------------------------

const char* WebsocketServer::base64Index="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//----------------------------------------------------------------------

bool WebsocketServer::pushData(const std::string& data)
{
	bool a=true;
	for(int i=1; i<m_nfds; i++){
		if(sendData(m_fds[i].fd, data)<0){
			a=false;
		}
	}
	return a;
}

//----------------------------------------------------------------------

void WebsocketServer::processRequests()
{
	std::map<std::string, Command>::iterator it;
	while(true){
		m_cv.wait(m_ulock, [this]{
			return m_commands.size()>0 && (!m_requests.empty() || m_exit);
		});

		if(m_exit){
			break;
		}

		Client& client=m_requests.front();

		std::string& command=client.m_command;
		std::size_t n=command.find_first_of(':');
		if(n!= std::string::npos){
			it=m_commands.find(command.substr(0, n));

			if(it!=m_commands.end()){
				std::string result=it->second(command.substr(n+1));
				sendData(client.m_fd, result);
			}
		}
		m_requests.pop();
	}
}

//----------------------------------------------------------------------

WebsocketServer::WebsocketServer(WebsocketServer::Sockaddr sockAddr, uint16_t port)
:m_sockAddr(sockAddr),
m_ulock(std::unique_lock<std::mutex>(m_controlMutex)),
m_trd(nullptr),
m_encode64(nullptr),
m_dataLength(0),
m_fd(::socket(AF_INET, SOCK_STREAM, 0)),
m_nfds(0),
m_recDataLength(0),
m_buffer64Size(0),
m_exit(false)
{
	if(m_fd == -1) {
		throw std::runtime_error(strerror(errno));
	}

	setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, NULL, 1);

	m_sockAddr.m_addr.sin_family=AF_INET;
	m_sockAddr.m_addr.sin_port=htons(port);

	int on=1;
	if(ioctl(m_fd, FIONBIO, (char *)&on)<0){
		std::cout<<"ioctl() failed\n";
	}

	if(-1==::bind(
		m_fd,reinterpret_cast<sockaddr*>(&(m_sockAddr.m_addr)),
		sizeof(m_sockAddr.m_addr)))
	{
		throw std::runtime_error(strerror(errno));
	}

	std::memset(m_fds, 0, sizeof(m_fds));
	
	m_fds[m_nfds].fd=m_fd;
	m_fds[m_nfds].events=POLLIN;
	m_nfds=1;
	
	m_trd=new std::thread(&WebsocketServer::listening, this);
}

//----------------------------------------------------------------------

void WebsocketServer::listening()
{
	if(::listen(m_fd, 0)==-1){
		throw std::runtime_error(strerror(errno));
	}

	int rc;
	int timeOut=1*10*1000;
	bool compressArray=false;
	while(true){
		compressArray=false;
		rc=poll(m_fds, m_nfds, timeOut);

		if(m_exit || rc<0){
			break;
		}

		if(rc==0){
			continue;
		}

		int new_fd=-1;
		for(int i=0; i<m_nfds; i++){
			if(m_fds[i].revents==0 || m_fds[i].fd==-1){
				continue;
			}

			if(m_fds[i].revents!=POLLIN){
				close(m_fds[i].fd);
				m_fds[i].fd=-1;
				compressArray=true;
				continue;
			}

			if(m_fds[i].fd==m_fd){
				new_fd=-1;
				do{
					new_fd=::accept(m_fd, NULL, NULL);
					if(new_fd<0){
						break;
					}

					if(handShake(new_fd)){
						m_fds[m_nfds].fd=new_fd;
						m_fds[m_nfds].events=POLLIN;
						m_nfds++;
					}
					else{
						close(new_fd);
					}
				}while(new_fd!=-1);
			}
			else
			{
				if(getData(m_fds[i].fd)){
					close(m_fds[i].fd);
					m_fds[i].fd=-1;
					compressArray=true;
				}
				else{
					std::string&& str=unmask();
					if(str.length()>0){
						m_requests.emplace(m_fds[i].fd, std::move(str));
						m_cv.notify_one();
					}
				}
			}
		}

		if(compressArray){
			int i=0; 
			while(i<m_nfds){
				if(m_fds[i].fd==-1){
					while(m_nfds>1 && m_fds[m_nfds].fd==-1){
						m_nfds--;
					}
					m_fds[i].fd=m_fds[m_nfds].fd;
					m_fds[i].events=POLLIN;
					m_fds[m_nfds].fd=-1;
				}
				i++;
			}
		}
	}
}

//----------------------------------------------------------------------

void WebsocketServer::encodeData(const std::string& text)
{
	const unsigned char b1=0x80 | (0x1 & 0x0f);
	int length=text.length();
	m_dataLength=0;
	m_pack[0]=static_cast<unsigned char>(b1);

	if(length<=125){
		m_pack[1]=static_cast<unsigned char>(length);
		m_pack[1]=m_pack[1] & ~(1<<7);			
		m_dataLength=2;
	}
	else if(length>125 && length<65536){
		m_pack[1]=static_cast<unsigned char>(126);
		m_pack[2]=static_cast<unsigned short>(length);
					
		union Package
		{
			unsigned short num1;	
			unsigned char data[2];
		};
			
		Package pkg;
		pkg.num1=static_cast<unsigned short>(length); 
		
		m_pack[3]= pkg.data[1];
		m_pack[4]= pkg.data[0];

		m_dataLength=5;
	}
	else if(length>=65536){		
		m_pack[1]=static_cast<unsigned char>(127);

		union Package2
		{
			unsigned long num1;	
			unsigned char data[4];
		};
			
		Package2 pkg2;			
		pkg2.num1=static_cast<unsigned long>(length); 
		
		m_pack[3]=pkg2.data[3];
		m_pack[4]=pkg2.data[2];
		m_pack[5]=pkg2.data[1];
		m_pack[6]=pkg2.data[0];

		m_dataLength=7;
	}

	for (int i=0; i<=length; i++){
		m_pack[m_dataLength++]=static_cast<char>(text[i]);
	}
	m_dataLength--;
}

//----------------------------------------------------------------------

bool WebsocketServer::handShake(int client){
	std::array<char, MAX_SIZE> buf{};
	if(::recv(client, buf.data(), buf.size(), 0)!= 0){
		std::string headers(buf.data());
		buf[0]='\0';

		std::size_t found=headers.find("Sec-WebSocket-Key:");
		if(found!=std::string::npos){
			found=headers.find(":", found);
			std::size_t found2=headers.find("\r\n", found);

			std::string acceptKey=headers.substr(found+2, found2-found-2) + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

			rawHash(acceptKey);
			std::string&& acceptKeyHash=base64Encode(m_hashSha1);
			std::string hs="HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "+ acceptKeyHash+"\r\n\r\n";

			return ::send(client, hs.data(), hs.length(), 0)>0;
		}
	}

	return false;
}

//----------------------------------------------------------------------

std::string WebsocketServer::unmask()
{
	if(m_recDataLength<1){
		return "";
	}

	unsigned int length=static_cast<int>(m_recBuffer[1]) & 127;

	int mask=2;
	int offset=6;
	if(length==126){
		mask=4;
		offset=8;
	}
	else if(length==127){
		mask=10;
		offset=14;
	}

	if(m_recDataLength<offset){
		return "";
	}

	std::string result;
	result.reserve(m_recDataLength-offset);
	for(int i=0; i<m_recDataLength-offset; i++) {
		result+=m_recBuffer[offset+i]^m_recBuffer[mask+(i%4)];
	}
	return result;
}

//----------------------------------------------------------------------

const char* WebsocketServer::base64Encode(const char* input)
{
	const int size=std::strlen(input);
	const int tripples=size/3;
	int bufferSize=1+(tripples+1)*4;

	if(m_buffer64Size<bufferSize){
		delete[] m_encode64;
		m_buffer64Size=bufferSize;
		m_encode64=new char[m_buffer64Size];
	}

	int i=0, j=0;
	for(; i<tripples; i++){
		encoder(input[3*i], input[3*i+1], input[3*i+2], j);
		j+=4;
	}

	int k=size-3*tripples;
	if(k>0){
		if(k==1){
			encoder(input[3*i], 0x00, 0x00, j);
			j+=2;
			m_encode64[j++]='=';
			m_encode64[j++]='=';
		}
		else{
			encoder(input[3*i], input[3*i+1], 0x00, j);
			j+=3;
			m_encode64[j++]='=';
		}
	}

	m_encode64[j]=0;

	return m_encode64;
}

//----------------------------------------------------------------------

void WebsocketServer::rawHash(const std::string& str2)
{
	m_hashSha1[0]=0;
	m_sha1.update(str2);
	std::string&& str=m_sha1.final();

	unsigned char a;
	unsigned int j=0;
	for(unsigned int i=0; i<str.length(); i++){
		a=static_cast<unsigned char>(str[i]);
		if(a>87){
			a-=87;
		}
		else{
			a-=48;
		}

		j=i>>1;
		if((i&1)==0){
			m_hashSha1[j]=a<<4;
		}
		else{
			m_hashSha1[j]=m_hashSha1[j] | a;
		}
	}
	m_hashSha1[20]=0;
}

//----------------------------------------------------------------------
