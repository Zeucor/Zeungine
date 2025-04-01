#include <zg/net/udpmc_receiver.hpp>
#include <zg/net/udpmc_sender.hpp>
#include <zg/Serial.hpp>
int main()
{
    std::string host = "localhost";
    int port = 3338;
    zg::net::udpmc_receiver receiver(host, port);
    zg::net::udpmc_sender sender(host, port);
    Serial receiver_serial(receiver);
    Serial sender_serial(sender);
    sender_serial << 'A';
    char _char = 0;
    receiver_serial >> _char;
}