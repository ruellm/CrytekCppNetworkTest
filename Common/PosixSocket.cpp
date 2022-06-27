#include "PosixSocket.h"

CPosixSocket::CPosixSocket(SOCKET_FD handle)
{
	SetHandle(handle);
}
CPosixSocket::~CPosixSocket()
{
	Disconnect();
}

bool CPosixSocket::CreateAsServer(int port)
{
	int opt = 1;

	// Creating socket file descriptor
	if ((m_handle = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				   &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	m_address.sin_family = AF_INET;
	m_address.sin_addr.s_addr = INADDR_ANY;
	m_address.sin_port = htons( port );

	// Forcefully attaching socket to the port 8080
	if (bind(m_handle, (struct sockaddr *)&m_address,
			 sizeof(m_address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(m_handle, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	return true;
}

std::shared_ptr<ISocketBase> CPosixSocket::Accept()
{
	int newSocket = -1;
	int addrlen = sizeof(m_address);

	if ((newSocket = accept(m_handle, (struct sockaddr*)&m_address,
						  (socklen_t*)&addrlen)) < 0) {
		std::cout << "[ERROR] Accept Failed \n";
		return nullptr;
	}

	return std::move(std::make_shared<CPosixSocket>(newSocket));
}

bool CPosixSocket::Connect(const std::string& address, int port)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;

	m_handle = socket(AF_INET, SOCK_STREAM, 0);
	if (m_handle < 0) {
		std::cout <<  "[ERROR] unable to create socket object \n";
		return false;
	}

	server = gethostbyname(address.c_str());
	if (server == NULL) {
		std::cout <<  "[ERROR] No such host \n";
		return false;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	// Send a connection request to the server
	if (connect(m_handle, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		std::cout <<  "[ERROR] unable to create socket object \n";
		return false;
	}

	return true;
}
void CPosixSocket::Disconnect()
{
	if(m_handle){
		close(m_handle);
	}
}

int CPosixSocket::Read(void* ptr, int size)
{
	return read(m_handle,ptr,size);
}

int CPosixSocket::Write(void* ptr, int size)
{
	return write(m_handle,ptr,size);
}