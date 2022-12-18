#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

const int QUEUE_SIZE = 5;
const int BUF_SIZE = 1024;


std::string response = "Gotcha!\n";
char buf[sizeof(response)];

std::string recv_all(int socket){
    char buf[BUF_SIZE];
    int total = 0;
    int n = BUF_SIZE;
    std::string message = "";
    while (n == BUF_SIZE) {
        n = recv(socket, buf, BUF_SIZE, 0);
        std::cout << "N: " << n << std::endl;
        if (n == 0) throw("Connection closed");
        message.append(buf);
    }
    return message;
}

void requet_handler(int client_socket){

    std::string msg = recv_all(client_socket);
    std::cout << "Response: " << msg;
    int n = send(client_socket, response.c_str(), sizeof(response), 0);
    if (n < 0){
        perror("sending");
        close(client_socket);
    }
    std::cout << "Sent " << n << " bytes" << std::endl;
    close(client_socket);
//    sleep(5);
    std::cout << "Socket closed, returning..." << std::endl;
    return;
}

int main()
{
    // инициализируем сокет
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0){
        perror("socket");
        exit(1);
    }

    sockaddr_in addr;

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(4567);
    addr.sin_family = AF_INET;

    if (bind(listen_sock, (sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_sock);
        exit(1);
    }

    if (listen(listen_sock, QUEUE_SIZE)) {
        perror("listen");
        close(listen_sock);
        exit(1);
    }

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_sock;
    while(1){
        if((accept_sock = accept(listen_sock, (sockaddr *) &client_addr, &client_addr_len)) < 0){
            perror("accept");
            close(listen_sock);
            exit(1);
        }

        std::cout << "Starting new thread..." << std::endl;
        try {
            // Создаем новый поток для сокета клиента, отправившего запрос
            std::thread(requet_handler, accept_sock).detach();
        }
        catch(std::exception e){
            std::cout << "Errno: " << errno << "Exception: " << e.what();
        }

//        exit(0);
    }
    return 0;
//    addr.sa_data;

    // связываем сокет с IP-адресом и портом
//    bind(websock, )

}
