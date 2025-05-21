#include <zg/net/udpmc_receiver.hpp>
#include <zg/net/udpmc_sender.hpp>
#include <zg/Serial.hpp>
int main()
{
    std::string host = "239.255.255.255";
    int port = 3338;
    std::unordered_map<size_t, zg::net::udpmc_receiver*> rcv_map;
    for (size_t c = 1; c <= 5; c++)
    {
        rcv_map[c] = new zg::net::udpmc_receiver(host, port);
    }
    zg::net::udpmc_sender sender(host, port);
    std::unordered_map<size_t, Serial*> serial_map;
    for (size_t c = 1; c <= 5; c++)
    {
        serial_map[c] = new Serial(*rcv_map[c]);
    }
    Serial sender_serial(sender);
    (sender_serial << 'A').synchronize();
    std::cout << "udpmc_sender: synchronized" << std::endl;
    char _char = 0;
    for (size_t c = 1; c <= 5; c++)
    {
        auto& receiver_serial = *serial_map[c];
        receiver_serial >> _char;
        std::cout << "udpmc_receiver[" << c << "]: received: " << _char << std::endl;
    }
}