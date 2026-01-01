#pragma once

//标准库
#include <stdio.h>
#include <iostream>
#include <atomic>
#include <string>
#include <algorithm>

//g3log库
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <g3log/loglevels.hpp>
#include <g3log/logmessage.hpp>

int GetLine(int sock, char* msg_buffer, int size);

#ifdef _WIN32 // WINDOWS环境所需文件

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "WS2_32.lib")

int StartupWin(unsigned short port);

//Windows初始化函数
#define STARTUP_FUNC(port) StartupWin(port)
#define AcceptRequest AcceptRequestWin

#else // Linux/POSIX Sockets 头文件
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <cstring>
#include <pthread.h>
#include <signal.h>

int StartupPosix(unsigned short port);

// Linux 初始化函数 (通常为空操作)
#define STARTUP_FUNC(port) StartupPosix(port)
#define AcceptRequest AcceptRequestPosix
#endif