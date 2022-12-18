#include "webserver.h"

int main()
{
    WebServer server = WebServer(4567);
    server.run();
    server.stop();
}
