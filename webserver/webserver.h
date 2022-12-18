#ifndef WEBSERVER_H
#define WEBSERVER_H

#pragma once

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <map>
#include <fstream>
#include <sstream>
#include <cstring>

const int QUEUE_SIZE = 5;
const int BUF_SIZE = 1024;
const int HTTP_OK = 200;

enum HttpCode
{
    OK = 200,
    badRequest = 400,
    notFound = 404,
    methodNotAllowed = 405,
    internalServerError = 500
};

enum mimeTypes
{
    TEXT,
    HTML,
    PDF
};

typedef std::map<std::string, std::string> http_request;

class WebServer
{
//    int port;
    int listenSocket;
    sockaddr_in address;
    std::string workDir = "/home/andrew/Ucheba/FPMI/AMI_1st_sem/SIT/lab4";

    void handleRequest(int client_socket);
    http_request parseRequest(std::string request);
    std::string recvAll(int client_socket);
    int sendAll(int client_socket, std::string message);
    std::string httpPhrase(HttpCode code);
    std::string contentTypeHeader(mimeTypes type);
    int sendResponse(int client_socket, std::string path);
    void errorResponse(int client_socket, HttpCode code);

public:
    WebServer(int port);
    int run();
    void stop();
    ~WebServer();
};

#endif // WEBSERVER_H
