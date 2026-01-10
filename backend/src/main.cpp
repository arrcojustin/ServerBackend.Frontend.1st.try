#include "main.h"
//头文件
static std::atomic<bool> g_should_exit{ false };
static int server_sock = -1;

int GetLine(int sock, char* msg_buffer, int size) {
	return 0;
}

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD signal) {
	if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
		LOG(INFO) << "捕获到 Windows 关闭信号";
		g_should_exit = true;
		closesocket(server_sock);
		return TRUE;
	}
	return FALSE;
}
int StartupWin(unsigned short* port) {
	LOG(INFO) << "STARTUP_FUNC启动（win）";
	WSADATA data;
	int ret = WSAStartup(MAKEWORD(1, 1), &data);
	if (ret) {
		LOG(FATAL) << "WSAStartup 失败, 错误码:"<< ret;
		return -1;
	}
	else {
		LOG(INFO) << "WSAStartup 成功";
	}
	int server_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (server_socket < 0) { 
		int error_backup = WSAGetLastError();
		WSACleanup();
		LOG(FATAL) << "套接字未创建成功，错误码:" << error_backup;
		return -1;
	}
	else {
		LOG(INFO) << "套接字创建成功";
	}
	int opt = 1;
	ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (ret == SOCKET_ERROR) {
		int error_backup = WSAGetLastError();
		closesocket(server_socket);
		WSACleanup();
		LOG(FATAL) << "设置 SO_REUSEADDR 失败, 错误码:" << error_backup;
		return -1;
	}
	else {
		LOG(INFO) << "设置 SO_REUSEADDR 成功";
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0 , sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(*port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int bind_ret = bind(server_socket, (const sockaddr*)&server_addr, sizeof(server_addr));
	if (bind_ret == SOCKET_ERROR) {
		int error_backup = WSAGetLastError();
		closesocket(server_socket);
		WSACleanup();
		LOG(FATAL) << "绑定套接字失败，错误码:" << error_backup;
		return -1;
	}
	else {
		LOG(INFO) << "绑定套接字成功";
	}

	int name_len = sizeof(server_addr);
	if (*port == 0) {
		if (getsockname(server_socket, (struct sockaddr*)&server_addr, &name_len) < 0) {
			LOG(FATAL) << "端口无法正确分配";
		}
		else {
			LOG(INFO) << "端口已正确分配";
		}
		*port = ntohs(server_addr.sin_port);
		LOG(INFO) << "端口已重新分配，为:" << *port;
	}

	int listen_ret = listen(server_socket, 20);
	if (listen_ret == SOCKET_ERROR) {
		int error_backup = WSAGetLastError();
		closesocket(server_socket);
		WSACleanup();
		LOG(FATAL) << "监听失败，错误码:" << error_backup;
		return -1;
	}
	else {
		LOG(INFO) << "监听成功";
	}
	return (int)server_socket;
}

DWORD WINAPI AcceptRequestWin(LPVOID arg) {
	char msg_buffer[1024];

	int client = (SOCKET)arg;

	int read_count = GetLine(client, msg_buffer, sizeof(msg_buffer));
	LOG(INFO) << "读取到数据：" << msg_buffer;
	return 0;
}
#endif

#ifndef _WIN32
void PosixSignalHandler(int signal) {
	if (signal == SIGINT || signal == SIGTERM) {
		LOG(INFO) << "捕获到 POSIX 关闭信号。";
		g_should_exit = true;
		close(server_sock);
	}
}
int StartupPosix(unsigned short* port) {
	LOG(INFO) << "STARTUP_FUNC启动（linux）";
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		LOG(FATAL) << "套接字未创建成功，错误码:" << errno;
		return -1;
	}
	else {
		LOG(INFO) << "套接字创建成功";
	}
	int opt = 1; 
	int ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (ret < 0) { 
		int error_backup = errno;
		close(server_socket);
		LOG(FATAL) << "设置 SO_REUSEADDR 失败，错误码: " << error_backup;
		return -1;
	}
	else {
		LOG(INFO) << "设置 SO_REUSEADDR 成功";
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(*port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int bind_ret = bind(server_socket, (const sockaddr*)&server_addr, sizeof(server_addr));
	if (bind_ret < 0) {
		LOG(FATAL) << "绑定套接字失败，错误码:" << errno;
		return -1;
	}
	else {
		LOG(INFO) << "绑定套接字失败成功";
	}

	int name_len = sizeof(server_addr);
	if (*port == 0) {
		if (getsockname(server_socket, (struct sockaddr*)&server_addr, &name_len) < 0) {
			LOG(FATAL) << "端口无法正确分配";
		}else {
			LOG(INFO) << "端口已正确分配";
		}
		*port = htons(server_addr.sin_port);
		LOG(INFO) << "端口已重新分配，为:" << *port;
	}
	int listen_ret = listen(server_socket, 20);
	if (listen_ret < 0) {
		LOG(FATAL) << "监听失败，错误码:" << errno;
		return 0;
	}
	else {
		LOG(INFO) << "监听成功";
	}
	return (int)server_socket;
}
void* AcceptRequestPosix(void* arg) {
	
	return NULL;
}
#endif

void SetupSignalHandlers() {
#ifdef _WIN32
	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
#else
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = PosixSignalHandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
#endif
}

int main(void) {
	const std::string log_dir = "logs/";
	const std::string log_prefix = "BackendServerLog";
	std::unique_ptr<g3::LogWorker> logworker = g3::LogWorker::createLogWorker();
	auto sinkHandle = logworker->addSink(
		std::make_unique<g3::FileSink>(log_prefix, log_dir),
		&g3::FileSink::fileWrite
	);
	g3::initializeLogging(logworker.get());
	LOG(INFO) << "TEST";
	unsigned short port = 80;
	server_sock = STARTUP_FUNC(&port);
	LOG(INFO)<< "端口为:" << port;
	
	SetupSignalHandlers();
	struct sockaddr_in client_addr;
	int client_addr_size = sizeof(client_addr);
	//while (TRUE) {
	while (!g_should_exit){
		int client_sock = accept(server_sock, (struct sockaddr*) & client_addr, &client_addr_size);
		if (client_sock == INVALID_SOCKET) {
		#ifdef _WIN32
			int error_backup = WSAGetLastError();
			if (g_should_exit || error_backup == WSAEINTR || error_backup == WSAENOTSOCK) {
				LOG(INFO) << "捕获到套接字关闭信号，退出主循环。";
				continue;
			}
			LOG(WARNING) << "接受连接失败，错误码:" << error_backup;
			#else
			if (g_should_exit || errno == EINTR || errno == EBADF) {
				LOG(INFO) << "捕获到套接字关闭信号，退出主循环。";
				continue;
			}
			LOG(WARNING) << "接受连接失败，错误码:" << errno;
			#endif
		}
		#ifdef _WIN32
		DWORD thread_id = 0;
		CreateThread(0, 0, AcceptRequest, (void*)client_sock, 0, &thread_id);
		#else
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, AcceptRequest, (void*)client_sock);
		#endif
	}

	#ifdef _WIN32
		closesocket(server_sock);
		LOG(INFO) << "Windows 套接字已关闭。";
		WSACleanup();
		LOG(INFO) << "WinSock DLL 已清理。";
	#else
	close(server_sock);
	LOG(INFO) << "POSIX 套接字已关闭。";
	#endif
	g3::internal::shutDownLogging();
	return 0;
}