#include <iostream>
// #include <cstdlib>
#include <string>

int main() {
    std::string canFrame = "000000";

    std::string command = "cansend can0 2531C40#" + canFrame;

    int result = system(command.c_str());

    if(result == 0) {
        std::cout << "CAN Frame sent successfully." << std::endl;
    } else {
        std::cerr << "Failed to send CAN Frame." << std::endl;
    }

    return 0;
}