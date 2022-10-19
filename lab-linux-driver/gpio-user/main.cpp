#include <chrono>
#include <string_view>
#include <thread>

#include <fcntl.h>
#include <unistd.h>

using namespace std::literals;

template <typename file_t>
auto write(file_t file, std::string_view data)
{
    return write(file, data.data(), data.size());
}

int main(int argc, char* argv[])
{
    int fp = open("/dev/gpio", O_WRONLY);
    while (true)
    {
        write(fp, "18=0");
        std::this_thread::sleep_for(1s);
        write(fp, "18=1");
        std::this_thread::sleep_for(1s);
    }
    close(fp);
}
