// Graph Engine
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//
#include <os/os.h>
#if !defined(TRINITY_PLATFORM_WINDOWS)
#include "ClientSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

namespace Trinity
{
    namespace Network
    {
        uint64_t CreateClientSocket()
        {
            return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        }

        bool ClientSocketConnect(uint64_t clientsocket, uint32_t ip, uint16_t port)
        {
            //struct sockaddr_in {
            //    short            sin_family;   
            //    unsigned short   sin_port;     
            //    struct in_addr   sin_addr;     
            //    char             sin_zero[8];  
            //}
            struct sockaddr_in ipaddr;
            ipaddr.sin_family = AF_INET;
            ipaddr.sin_port = htons(port);
            ipaddr.sin_addr.s_addr = ip;

            //TODO handshake
            if (0 == connect((int)clientsocket, (struct sockaddr*)&ipaddr, sizeof(sockaddr_in)))
                return true;
            return false;
        }

        bool ClientSend(uint64_t socket, char* buf, int32_t len)
        {
            do
            {
                ssize_t bytesSent = write((int)socket, buf, len);
                if (-1 == bytesSent)
                {
                    //TODO log error
                    close((int)socket);
                    return false;
                }
                buf += bytesSent;
                len -= bytesSent;
            } while (len > 0);
            return true;
        }

        bool _do_recv(int socket, char* buf, int32_t len)
        {
            while (len)
            {
                ssize_t bytesRecvd = read(socket, buf, len);
                if (-1 == bytesRecvd || 0 == bytesRecvd)
                {
                    //TODO log error
                    close((int)socket);
                    return false;
                }
                buf += bytesRecvd;
                len -= bytesRecvd;
            }
            return true;
        }

        TrinityErrorCode ClientReceive(uint64_t _socket, char* & buf, int32_t & len)
        {
            int socket = (int)_socket;
            if (!_do_recv(socket, (char*)&len, sizeof(int32_t)))
            {
                return TrinityErrorCode::E_NETWORK_RECV_FAILURE;
            }

            if (len < 0) return (TrinityErrorCode)len;

            buf = (char*)malloc(len); // will be freed by a C# message reader.
            if (NULL == buf)
            {
                Trinity::Diagnostics::FatalError("Cannot allocate memory in network Receive.");
                return TrinityErrorCode::E_NOMEM
            }

            if (!_do_recv(socket, buf, len))
            {
                return TrinityErrorCode::E_NETWORK_RECV_FAILURE;
            }

            return TrinityErrorCode::E_SUCCESS;
        }

        TrinityErrorCode WaitForAckPackage(uint64_t _socket)
        {
            int32_t buf;
            char *recvbuf = (char*)&buf;
            int socket    = (int)_socket;
            if (!_do_recv(socket, recvbuf, sizeof(int32_t)))
            {
                return TrinityErrorCode::E_NETWORK_RECV_FAILURE;
            }
            else
            {
                return (TrinityErrorCode)buf;
            }
        }

        void CloseClientSocket(uint64_t socket)
        {
            shutdown((int)socket, SHUT_RDWR);
            close((int)socket);
        }

        uint32_t IP2UInt32(const char* ip)
        {
            in_addr ip_addr;
            inet_aton(ip, &ip_addr);
            return ip_addr.s_addr;
        }
    }
}

#endif//platform