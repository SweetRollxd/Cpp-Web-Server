#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

//using namespace std;

std::string message = "Hello there!\n";
char buf[sizeof(message)];

int main()
{
    std::cout << "Creating socket" << std::endl;
    // инициализируем сокет
    int websock = socket(AF_INET, SOCK_STREAM, 0);
    if (websock < 0){
        perror("socket");
        exit(1);
    }

    std::cout << "Connecting to socket" << std::endl;
    sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(4567);
    addr.sin_family = AF_INET;

    if(connect(websock, (sockaddr *) &addr, sizeof(addr))){
        perror("connect");
        exit(2);
    }

    std::cout << "Sending data" << std::endl;
    try{
        int n = send(websock, message.c_str(), sizeof(message), 0);
        std::cout << "N " << n << std::endl;
    }
    catch(std::exception e){
        std::cout << "Excetion: " << e.what();
    }


    recv(websock, buf, sizeof(message), 0);

    std::cout << "Response: " << buf;
    close(websock);

//    addr.sa_data;

    // связываем сокет с IP-адресом и портом
//    bind(websock, )

}
