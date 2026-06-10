
#define BACKLOG 10

class Server {
public:
    Server(int port) : port_(port) {};
    void start();
private:
    int port_;
};
