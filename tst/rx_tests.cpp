#include "../lib/audio/audiorx/audioreceiver.h"

uint8_t err = 0;

int main()
{
    std::cout << "Starting RX test" << std::endl;
    string result;
    err = init_receiver();
    std::cout << "Receiver initialized" << std::endl;
    err = listen(result);
    std::cout << "Listened" << std::endl;
    err = close_receiver();
    std::cout << "Finished RX test" << std::endl;
    return err;
}