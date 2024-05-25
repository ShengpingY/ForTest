#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iomanip>
#include <iostream>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>
// #include </home/dronelab/Can_Test/can_c/HexDecimal.h>
#include <thread>
#include <chrono>

#include "keyboard.h"

class CanController {
private:
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    void floatToUnsignedChar(float floatValue, unsigned char* hex) {
        union {
            float value;
            unsigned char bytes[4];
        } data;

        data.value = floatValue;

        for (int i = 0; i < 4; ++i) {
            hex[i] = data.bytes[3 - i];
        }
    }

public:
    CanController() {
        const char *ifname = "can0";

        if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
            perror("Error while opening socket");
            exit(EXIT_FAILURE);
        }

        strcpy(ifr.ifr_name, ifname);
        ioctl(s, SIOCGIFINDEX, &ifr);

        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        // printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

        if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("Error in socket bind");
            exit(EXIT_FAILURE);
        }
    }

    ~CanController() {
        close(s);
    }
    void send_Heartbeat(){
        int nbytes;
        struct can_frame frame_heartbeat;

        frame_heartbeat.can_id  = 0x2052C80;
        frame_heartbeat.can_id |= CAN_EFF_FLAG;
        frame_heartbeat.can_dlc = 8;

        frame_heartbeat.data[0] = 0xFFU;
        frame_heartbeat.data[1] = 0xFFU;
        frame_heartbeat.data[2] = 0xFFU;
        frame_heartbeat.data[3] = 0xFFU;
        frame_heartbeat.data[4] = 0xFFU;
        frame_heartbeat.data[5] = 0xFFU;
        frame_heartbeat.data[6] = 0xFFU;
        frame_heartbeat.data[7] = 0xFFU;

        nbytes = write(s, &frame_heartbeat, sizeof(struct can_frame));
        if (nbytes < 0) {
            perror("CAN Frame write error!");
            exit(EXIT_FAILURE);
        }

        // printf("Wrote CAN Frame -> CANID: 0x%X Data: ", frame_heartbeat.can_id);
        // for (int i = 0; i < frame_heartbeat.can_dlc; i++) {
        //     printf("%02X ", frame_heartbeat.data[i]);
        // }
        // printf("\n");
    }

    void setRotateVelocity(uint32_t can_id, float velocity) {
        int nbytes;
        struct can_frame frame_rotate;

        frame_rotate.can_id  = can_id;
        frame_rotate.can_id |= CAN_EFF_FLAG;
        frame_rotate.can_dlc = 8;

        unsigned char hexBytes[4];
        floatToUnsignedChar(velocity, hexBytes);

        frame_rotate.data[0] = hexBytes[3];
        frame_rotate.data[1] = hexBytes[2];
        frame_rotate.data[2] = hexBytes[1];
        frame_rotate.data[3] = hexBytes[0];
        frame_rotate.data[4] = 0x00U;
        frame_rotate.data[5] = 0x00U;
        frame_rotate.data[6] = 0x00U;
        frame_rotate.data[7] = 0x00U;

        nbytes = write(s, &frame_rotate, sizeof(struct can_frame));
        if (nbytes < 0) {
            perror("CAN Frame write error!");
            exit(EXIT_FAILURE);
        }

        // printf("Wrote CAN Frame -> CANID: 0x%X Data: ", frame_rotate.can_id);
        // for (int i = 0; i < frame_rotate.can_dlc; i++) {
        //     printf("%02X ", frame_rotate.data[i]);
        // }
        // printf("\n");
    }
};



int main() {
    Keyboard::initTermios();// initialize keyboard
    CanController canController;
    float velocity_fwd = 0.0f;
    float velocity_rtt = 0.0f;
    uint32_t can_id_fwd = 0x2050082; // Set your desired CAN ID here
    uint32_t can_id_rtt = 0x2050085; // Set your desired CAN ID here
    canController.send_Heartbeat();
    canController.setRotateVelocity(can_id_fwd, velocity_fwd);
    canController.setRotateVelocity(can_id_rtt, velocity_rtt);
    std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
    std::cout << "(decreased) Current Velocity: " << velocity_fwd << std::endl;
    
    while(true) {
        canController.send_Heartbeat();
        

        if (Keyboard::kbhit()) { // ¼ì²éÊÇ·ñÓÐ°´¼üÊäÈë
            char c = Keyboard::getch(); // ¶ÁÈ¡×Ö·û

            switch(c) { // »ñÈ¡·½Ïò
                case 'A': // UP arrow key
                    velocity_fwd += 0.01f; // increase velocity
                    std::cout << "(increased) Current Velocity: " << velocity_fwd << std::endl;
                    break;
                case 'B': // DOWN arrow key
                    velocity_fwd -= 0.01f; // decrease velocity
                    std::cout << "(decreased) Current Velocity: " << velocity_fwd << std::endl;
                    break;
                case 'D': // left arrow key
                velocity_rtt -= 0.01f; // decrease velocity
                std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
                break;
                case 'C': // right arrow key
                velocity_rtt += 0.01f; // decrease velocity
                std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
                break;
            }
            canController.setRotateVelocity(can_id_fwd, velocity_fwd);
            canController.setRotateVelocity(can_id_rtt, velocity_rtt);
        }
        // Sleep for 100 microseconds
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
    Keyboard::resetTermios();
    return 0;
}
// int main() {
//     CanController canController;
//     int i = 0;
// 	float k = 0;

//     uint32_t can_id = 0x2050082; // Set your desired CAN ID here

//     while(i < 1000000000) {
//         // Perform other operations if needed

//         canController.setRotateVelocity(can_id, -1.00000000f+k);

//         // Sleep for 500 microseconds
//         std::this_thread::sleep_for(std::chrono::microseconds(100));
//         i++;
// 		k = k + 0.00002;
//     }

//     return 0;
// }
