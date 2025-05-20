#include <zg/net/udpmc_receiver.hpp>
#include <zg/net/udpmc_sender.hpp>
#include <zg/Serial.hpp>
int main()
{
    std::string host = "238.247.227.33"; // localhost
    int port = 3338;
    zg::net::udpmc_receiver receiver(host, port);
    zg::net::udpmc_sender sender(host, port);
    Serial receiver_serial(receiver);
    Serial sender_serial(sender);
    (sender_serial << 'A').synchronize();
    std::cout << "udpmc_sender: synchronized" << std::endl;
    char _char = 0;
    receiver_serial >> _char;
    std::cout << "udpmc_receiver: received: " << _char << std::endl;
}