#include "webserver.h"

void WebServer::handleRequest(int client_socket)
{
    try{
        std::cout << "================\r\nNew thread started!" << std::endl;
        std::string msg = recvAll(client_socket);
        std::cout << "Request: " << msg << std::endl;

        http_request req_data = parseRequest(msg);
//        std::cout << req_data["method"] << " " << req_data["path"] << " " << req_data["version"] << std::endl;

        if ((req_data["method"]).compare("GET") != 0){
            errorResponse(client_socket, HttpCode::methodNotAllowed);
            return;
        }

        std::string path = workDir;
        path.append(req_data["path"]);
        std::cout << "Full path:" << path << std::endl;

        sendResponse(client_socket, path);


    }
    catch(std::exception e){
        errorResponse(client_socket, HttpCode::internalServerError);
        close(client_socket);
    }

    return;
}

http_request WebServer::parseRequest(std::string request)
{
    auto head = request.begin();
    auto end = request.begin();
    http_request req_data;

    while (*end !=  ' ') end++;
    req_data["method"] = std::string(head, end);

    end++;
    head = end;
    while (*end != ' ') end++;

    if(std::string(head, end).compare("/") != 0)
        req_data["path"] = std::string(head, end);
    else req_data["path"] = "/index.html";

    end++;
    head = end;
    while (*end !=  '\r') end++;
    req_data["version"] = std::string(head, end);

    return req_data;
}

std::string WebServer::recvAll(int client_socket)
{

    char buf[BUF_SIZE];
    int n = BUF_SIZE;
    std::string message = "";
    while (n == BUF_SIZE) {
        n = recv(client_socket, buf, BUF_SIZE, 0);
        if (n == 0) throw("Connection closed");
        message.append(buf);
    }
    return message;
}

int WebServer::sendAll(int client_socket, std::string message)
{
    const char* buf = message.c_str();

    int total = 0;
    int bytes_sent = 0;
    while (total < message.length()) {
        bytes_sent = send(client_socket, buf + total, message.length(), 0);
        if (bytes_sent == -1) break; //throw("Connection closed");
        total += bytes_sent;
    }
    return (bytes_sent == -1 ? -1 : total);
}

std::string WebServer::httpPhrase(HttpCode code)
{
    switch(code){
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return std::string();
    }
}

std::string WebServer::contentTypeHeader(mimeTypes type)
{
    switch(type){
        case mimeTypes::TEXT: return "application/text; charset=utf8";
        case mimeTypes::HTML: return "text/html; charset=utf8";
        case mimeTypes::PDF: return "applcation/pdf";
        default: return std::string();
    }
}

int WebServer::sendResponse(int client_socket, std::string path)
{

    std::ifstream fp;
    fp.open(path);
    if (!fp.good()){
        errorResponse(client_socket, HttpCode::notFound);
        return -1;
    }

    auto it = path.begin();
    std::string format;

    while (*it !=  '.') it++;
    format = std::string(it, path.end());

    mimeTypes type;
    if (format.compare(".html") == 0)
        type = mimeTypes::HTML;
    else if (format.compare(".pdf") == 0)
        type = mimeTypes::PDF;
    else
        errorResponse(client_socket, HttpCode::badRequest);

    std::string response_body;
    std::string buf;
    while (getline (fp, buf)) response_body.append(buf);

    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Version: HTTP/1.1\r\n"
             << "Content-Type: " << contentTypeHeader(type) << "\r\n"
             << "Content-length: " << std::to_string(response_body.length()) << "\r\n\r\n"
             << response_body;


//    std::cout << "Response: " << response.str() << std::endl;
    int n = sendAll(client_socket, response.str());
    if (n < 0){
        perror("sending");
        close(client_socket);
    }
    std::cout << "Sent " << n << " bytes" << std::endl;
    fp.close();
    close(client_socket);
    std::cout << "Socket closed, returning...\r\n====================\r\n\r\n" << std::endl;
}

void WebServer::errorResponse(int client_socket, HttpCode code)
{
    std::cout << "Sending error with code " << code;
    std::stringstream response;
    std::stringstream response_body;
    response_body << "<html><head><title>" << code << " " << httpPhrase(code) << "</title></head>"
                  << "<body><h1>" << httpPhrase(code) << "</h1></body></html>";
    response << "HTTP/1.1 " << code << " " << httpPhrase(code) << "\r\n"
             << "Content-Type: text/html; charset=utf-8\r\n"
             << "Content-length: " << std::to_string(response_body.str().length()) << "\r\n\r\n"
             << response_body.str();
    sendAll(client_socket, response.str());
    close(client_socket);
    std::cout << "Socket closed, returning...\r\n====================\r\n\r\n" << std::endl;
    return;
}

WebServer::WebServer(int port)
{
    this->listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0){
        perror("socket");
        exit(1);
    }

    this->address.sin_addr.s_addr = htonl(INADDR_ANY);
    this->address.sin_port = htons(port);
    this->address.sin_family = AF_INET;

    // Включаем возможность переиспользовать порт, чтобы не приходилось ждать пока он освободится после закрытия сервера
    int enable = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (bind(listenSocket, (sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind");
        close(listenSocket);
        exit(1);
    }
}

int WebServer::run()
{
    if (listen(listenSocket, QUEUE_SIZE)) {
        perror("listen");
        close(listenSocket);
        exit(1);
    }

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_sock;
    while(1){
        if((accept_sock = accept(listenSocket, (sockaddr *) &client_addr, &client_addr_len)) < 0){
            perror("accept");
            close(listenSocket);
            exit(1);
        }

        std::cout << "Starting new thread..." << std::endl;
        try {
            // Создаем новый поток для сокета клиента, отправившего запрос
            std::thread(&WebServer::handleRequest, this, accept_sock).detach();
        }
        catch(std::exception e){
            std::cout << "Errno: " << errno << "Exception: " << e.what();
        }
    }
}

void WebServer::stop()
{
    close(listenSocket);
//    freeaddrinfo()
}

WebServer::~WebServer()
{
    stop();
}
