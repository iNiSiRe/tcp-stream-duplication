#include "TcpStreamDuplicator.h"

using namespace std;

int main(int argc, char *argv[])
{
    TcpStreamDuplicator server;

    if (argc != 4) {
        cerr << "Not enough arguments: <source_host> <source_port> <destination_port>" << endl;
        exit(-1);
    }

    const string source_host = argv[1];
    const int source_port = atoi(argv[2]);
    const int destination_port = atoi(argv[3]);

    server.start(source_host, source_port,destination_port);

    return 0;
}
