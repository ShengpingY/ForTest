#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iomanip>
#include <iostream>
#include <mutex>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <thread>
#include <chrono>

#include "keyboard.h"

#include "can_receive.h"

#define Motor5_Zero_Position 89.02
#define Motor6_Zero_Position 46.40
#define Motor7_Zero_Position 92.86


class CanController {
private:
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    int s_read;
    struct sockaddr_can addr_read;
    struct ifreq ifr_read;
    std::mutex mutex_;




public:
    float wheel_velocity = 0.0f;
    float wheel_position;
    float velocity_rtt = 0.0f;
    float velocity_fwd = 0.0f;
    float position_rtt = 0.0f;
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

    
    void setPosition(uint32_t can_id, float position) {
        // number of position representing the target position in rotations
        int nbytes;
        struct can_frame frame_rotate;

        frame_rotate.can_id  = can_id;
        frame_rotate.can_id |= CAN_EFF_FLAG;
        frame_rotate.can_dlc = 8;

        unsigned char hexBytes[4];
        floatToUnsignedChar(position, hexBytes);

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

    }

    void readonce_2_float_velocity(int canid){
        int nbytes;
        struct can_frame frame_rotate_read;

        
        nbytes = read(s, &frame_rotate_read, sizeof(struct can_frame));
        if((frame_rotate_read.can_id &= ~CAN_EFF_FLAG) == canid){
            if (nbytes < 0) {
                perror("CAN Frame read error!");
                return;
            }
            // printf("Received CAN Frame -> CANID: 0x%X Data: ", frame_rotate_read.can_id &= ~CAN_EFF_FLAG);
            // for (int i = 0; i < frame_rotate_read.can_dlc; i++) {
            //     printf("%02X ", frame_rotate_read.data[i]);
            // }
            // printf("\n");

            // tranfer 32bit data to float number
            wheel_velocity = UnsignedCharToFloat_32(frame_rotate_read.data[3],frame_rotate_read.data[2],frame_rotate_read.data[1],frame_rotate_read.data[0]);
            
            // printf("transfered float value is :%f \n", wheel_velocity);
        }
        return;
    }

    void readonce_2_float_position(int canid){
        int nbytes;
        struct can_frame frame_position_read;

        
        nbytes = read(s, &frame_position_read, sizeof(struct can_frame));
        if((frame_position_read.can_id &= ~CAN_EFF_FLAG) == canid){
            if (nbytes < 0) {
                perror("CAN Frame read error!");
                return;
            }


            // tranfer 32bit data to float number
            wheel_position = UnsignedCharToFloat_32(frame_position_read.data[3],frame_position_read.data[2],frame_position_read.data[1],frame_position_read.data[0]);
            printf("Received CAN Frame -> CANID: 0x%X Data: ", frame_position_read.can_id &= ~CAN_EFF_FLAG);
            // for (int i = 0; i < frame_position_read.can_dlc; i++) {
            //     printf("%02X ", frame_position_read.data[i]);
            // }
            printf(" Value is %3f\n",wheel_position);
            // printf("transfered float value is :%f \n", wheel_velocity);
        }
        else {
            return;
        }
        return;
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

void canread(int canid,CanController& canController1)
{
    while(1)
    {
        canController1.readonce_2_float_velocity(canid);
        // printf("%f ",canController1.wheel_velocity);
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
}

void keyboardcatch(CanController& canController)
{
    Keyboard::initTermios();// initialize keyboard
    canController.send_Heartbeat();
    while(true) {
        if (Keyboard::kbhit()) { // ¼ì²éÊÇ·ñÓÐ°´¼üÊäÈë
            char c = Keyboard::getch(); // ¶ÁÈ¡×Ö·û
            switch(c) { // »ñÈ¡·½Ïò
                case 'A': // UP arrow key
                    // update velocity
                    canController.velocity_fwd += 0.015f; // increase velocity
                    std::cout << "(increased) Current Velocity: " << canController.velocity_fwd << std::endl;
                    
                    break;

                case 'B': // DOWN arrow key
                    // update velocity
                    canController.velocity_fwd -= 0.015f; // increase velocity
                    std::cout << "(decreased) Current Velocity: " << canController.velocity_fwd << std::endl;
                    break;

                // rotation speed set
                case 'D': // left arrow key
                    canController.velocity_rtt -= 0.01f; // decrease velocity
                    std::cout << "(decreased) Current Rotating Velocity: " << canController.velocity_rtt << std::endl;
                    break;
                case 'C': // right arrow key
                    canController.velocity_rtt += 0.01f; // decrease velocity
                    std::cout << "(decreased) Current Rotating Velocity: " << canController.velocity_rtt << std::endl;
                    break;
                
                // emergency stop
                case 'x': // right arrow key
                    canController.velocity_fwd = 0.00f; // decrease velocity
                    std::cout << "(decreased) Current Velocity: " << canController.velocity_fwd << std::endl;
                    canController.velocity_rtt = 0.00f; // decrease velocity
                    std::cout << "(decreased) Current Rotating Velocity: " << canController.velocity_rtt << std::endl;
                    break;
            }
        }
        // Sleep for 100 microseconds
        std::this_thread::sleep_for(std::chrono::microseconds(50));
        
    }
    Keyboard::resetTermios();
}

void update_motorspeed(CanController& canController){
    float radius = 1;    
    float wheel_v;
    canController.send_Heartbeat();


    uint32_t can_id_fwd_2 = 0x2050082; // Set your desired CAN ID here
    uint32_t can_id_fwd_3 = 0x2050083; // Set your desired CAN ID here
    uint32_t can_id_fwd_4 = 0x2050084; // Set your desired CAN ID here
    uint32_t can_id_rtt_5 = 0x2050485; // Set your desired CAN ID here
    uint32_t can_id_rtt_6 = 0x2050486; // Set your desired CAN ID here
    uint32_t can_id_rtt_7 = 0x2050487; // Set your desired CAN ID here

    std::cout << "Current Rotating Velocity: " << canController.velocity_rtt << std::endl;
    std::cout << "Current Velocity: " << canController.velocity_fwd << std::endl;

    while(true){
        canController.send_Heartbeat();
        // read current motor velocity in rpm
        wheel_v = canController.wheel_velocity/ 4.71 * 38 / 1000 * 3.1415926 * 2;
        printf("Current speed is %f per min\n",wheel_v);
        // calculate rotation speed of motor
        canController.velocity_rtt = wheel_v/ radius;
        printf("Current rtt speed is %f rpm \n",canController.velocity_rtt);
        // update steering speed
        canController.setRotateVelocity(can_id_rtt_5, canController.velocity_rtt);
        canController.setRotateVelocity(can_id_rtt_6, canController.velocity_rtt);
        canController.setRotateVelocity(can_id_rtt_7, canController.velocity_rtt);
        canController.setRotateVelocity(can_id_fwd_2, canController.velocity_fwd);
        canController.setRotateVelocity(can_id_fwd_3, canController.velocity_fwd);
        canController.setRotateVelocity(can_id_fwd_4, canController.velocity_fwd);
        std::this_thread::sleep_for(std::chrono::microseconds(5000));
    }
}

void keyboardcontroll() {
    Keyboard::initTermios();// initialize keyboard
    CanController canController;
    float velocity_fwd = 0.0f;
    float velocity_rtt = 0.0f;
    float position_rtt = 0.0f;
    uint32_t can_id_fwd_2 = 0x2050082; // Set your desired CAN ID here
    uint32_t can_id_fwd_3 = 0x2050083; // Set your desired CAN ID here
    uint32_t can_id_fwd_4 = 0x2050084; // Set your desired CAN ID here
    uint32_t can_id_rtt_5 = 0x2050085; // Set your desired CAN ID here
    uint32_t can_id_rtt_6 = 0x2050086; // Set your desired CAN ID here
    uint32_t can_id_rtt_7 = 0x2050087; // Set your desired CAN ID here
    uint32_t can_id_rtp_5 = 0x2050C85; // Set your desired CAN ID here
    uint32_t can_id_rtp_6 = 0x2050C86; // Set your desired CAN ID here
    uint32_t can_id_rtp_7 = 0x2050C87; // Set your desired CAN ID here
    canController.send_Heartbeat();
    canController.setRotateVelocity(can_id_fwd_2, velocity_fwd);
    canController.setRotateVelocity(can_id_fwd_3, velocity_fwd);
    canController.setRotateVelocity(can_id_fwd_4, velocity_fwd);
    // canController.setRotateVelocity(can_id_rtt_5, velocity_rtt);
    // canController.setRotateVelocity(can_id_rtt_6, velocity_rtt);
    // canController.setRotateVelocity(can_id_rtt_7, velocity_rtt);
    // canController.setPosition(can_id_rtp_5, position_rtt);
    // canController.setPosition(can_id_rtp_6, position_rtt);
    // canController.setPosition(can_id_rtp_7, position_rtt);
    
    std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
    std::cout << "(decreased) Current Velocity: " << velocity_fwd << std::endl;
    
    while(true) {
        canController.send_Heartbeat();
        

        if (Keyboard::kbhit()) { // ¼ì²éÊÇ·ñÓÐ°´¼üÊäÈë
            char c = Keyboard::getch(); // ¶ÁÈ¡×Ö·û

            switch(c) { // »ñÈ¡·½Ïò
                case 'A': // UP arrow key
                    velocity_fwd += 0.1f; // increase velocity
                    std::cout << "(increased) Current Velocity: " << velocity_fwd << std::endl;
                    break;
                case 'B': // DOWN arrow key
                    velocity_fwd -= 0.1f; // decrease velocity
                    std::cout << "(decreased) Current Velocity: " << velocity_fwd << std::endl;
                    break;

                // rotation position set
                case 'D': // left arrow key
                    position_rtt -= 0.8f; // decrease velocity
                    std::cout << "(decreased) Current Rotating Position: " << position_rtt << std::endl;
                    break;
                case 'C': // right arrow key
                    position_rtt += 0.8f; // decrease velocity
                    std::cout << "(decreased) Current Rotating Position: " << position_rtt << std::endl;
                    break;

                // // rotation speed set
                // case 'D': // left arrow key
                //     velocity_rtt -= 0.01f; // decrease velocity
                //     std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
                //     break;
                // case 'C': // right arrow key
                //     velocity_rtt += 0.01f; // decrease velocity
                //     std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
                //     break;

                // emergency stop
                case 'x': // right arrow key
                    velocity_fwd = 0.00f; // decrease velocity
                    std::cout << "(decreased) Current Velocity: " << velocity_fwd << std::endl;
                    velocity_rtt = 0.00f; // decrease velocity
                    std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
                    break;
            }
            // canController.setRotateVelocity(can_id_fwd, velocity_fwd);
            // canController.setRotateVelocity(can_id_rtt, velocity_rtt);
            canController.setRotateVelocity(can_id_fwd_2, velocity_fwd);
            canController.setRotateVelocity(can_id_fwd_3, velocity_fwd);
            canController.setRotateVelocity(can_id_fwd_4, velocity_fwd);
            // canController.setRotateVelocity(can_id_rtt_5, velocity_rtt);
            // canController.setRotateVelocity(can_id_rtt_6, velocity_rtt);
            // canController.setRotateVelocity(can_id_rtt_7, velocity_rtt);
            canController.setPosition(can_id_rtp_5, position_rtt);
            canController.setPosition(can_id_rtp_6, position_rtt);
            canController.setPosition(can_id_rtp_7, position_rtt);
        }
        // Sleep for 100 microseconds
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
    Keyboard::resetTermios();
    return;
}

void position_read(CanController& canController1){
    while(true){
        std::this_thread::sleep_for(std::chrono::microseconds(50000));
        // read current position
        canController1.readonce_2_float_position(0x2051885);
        // float position_rtt_5 = canController1.wheel_position;
        // printf("motor 5's postion is: %3f \n",position_rtt_5);
        canController1.readonce_2_float_position(0x2051886);
        // float position_rtt_6 = canController1.wheel_position;
        // printf("motor 6's postion is: %3f \n",position_rtt_6);
        canController1.readonce_2_float_position(0x2051887);
        // float position_rtt_7 = canController1.wheel_position;
        // printf("motor 7's postion is: %3f \n",position_rtt_7);

        // check if any key has been pressed to exit the loop
        if(Keyboard::kbhit()){
            return;
        }
    }
    return;
}

void steering_pose_init(CanController& canController1){
    
    uint32_t can_id_rtp_5 = 0x2050C85; // Set your desired CAN ID here
    uint32_t can_id_rtp_6 = 0x2050C86; // Set your desired CAN ID here
    uint32_t can_id_rtp_7 = 0x2050C87; // Set your desired CAN ID here
    int i = 0;
    while(i<1000){
        canController1.send_Heartbeat();
        canController1.setPosition(can_id_rtp_5, Motor5_Zero_Position+15.5);
        canController1.setPosition(can_id_rtp_6, Motor6_Zero_Position-15.5);
        canController1.setPosition(can_id_rtp_7, Motor7_Zero_Position);
        i++;
        std::this_thread::sleep_for(std::chrono::microseconds(5000));
    }
}

int main()
{
    CanController canController1;

    std::cout << "Program started, press any key to Read current position!\n" << std::endl;
    std::cin.ignore();
    //std::cin.get();
    position_read(canController1);

    std::cout << "Steering Postition read, press any key to adjust initial steering pose!\nIt may takes a few Secs" << std::endl;
    std::cin.ignore();
    // steering_pose_init(canController1);

    std::cout << "Position adjusted, press any key to continue!" << std::endl;
    std::cin.ignore();


    // float delta = 1;
    // while(delta > 0.0005){
    //     canController1.send_Heartbeat();
    //     canController1.setPosition(can_id_rtp_6,0);
    // }

    
    std::thread t1(canread,0x2051842,std::ref(canController1));
    std::thread t2(keyboardcatch,std::ref(canController1));
    std::thread t3(update_motorspeed,std::ref(canController1));
    t1.join();
    t2.join();
    t3.join();
}

// int main() {
//     double radius = 3;
//     Keyboard::initTermios();// initialize keyboard
//     CanController canController;
//     float velocity_fwd = 0.0f;
//     float velocity_rtt = 0.0f;
//     float position_rtt = 0.0f;
//     uint32_t can_id_fwd_2 = 0x2050082; // Set your desired CAN ID here
//     uint32_t can_id_fwd_3 = 0x2050083; // Set your desired CAN ID here
//     uint32_t can_id_fwd_4 = 0x2050084; // Set your desired CAN ID here
//     uint32_t can_id_rtt_5 = 0x2050085; // Set your desired CAN ID here
//     uint32_t can_id_rtt_6 = 0x2050086; // Set your desired CAN ID here
//     uint32_t can_id_rtt_7 = 0x2050087; // Set your desired CAN ID here
//     uint32_t can_id_rtp_5 = 0x2050C85; // Set your desired CAN ID here
//     uint32_t can_id_rtp_6 = 0x2050C86; // Set your desired CAN ID here
//     uint32_t can_id_rtp_7 = 0x2050C87; // Set your desired CAN ID here
//     canController.send_Heartbeat();
//     canController.setRotateVelocity(can_id_fwd_2, velocity_fwd);
//     canController.setRotateVelocity(can_id_fwd_3, velocity_fwd);
//     canController.setRotateVelocity(can_id_fwd_4, velocity_fwd);
//     canController.setRotateVelocity(can_id_rtt_5, velocity_rtt);
//     canController.setRotateVelocity(can_id_rtt_6, velocity_rtt);
//     canController.setRotateVelocity(can_id_rtt_7, velocity_rtt);
//     canController.setPosition(can_id_rtp_5, position_rtt);
//     canController.setPosition(can_id_rtp_6, position_rtt);
//     canController.setPosition(can_id_rtp_7, position_rtt);
    
//     std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
//     std::cout << "(decreased) Current Velocity: " << velocity_fwd << std::endl;
    

//     while(true) {
        
//         canController.send_Heartbeat();
        

//         if (Keyboard::kbhit()) { // ¼ì²éÊÇ·ñÓÐ°´¼üÊäÈë
//             char c = Keyboard::getch(); // ¶ÁÈ¡×Ö·û
//             float wheel_v = canController.readonce_2_float(0x2051842);
//             switch(c) { // »ñÈ¡·½Ïò
//                 case 'A': // UP arrow key
//                     // update velocity
//                     velocity_fwd += 0.01f; // increase velocity
//                     std::cout << "(increased) Current Velocity: " << velocity_fwd << std::endl;
                    
//                     break;

//                 case 'B': // DOWN arrow key
//                     // update velocity
//                     velocity_fwd -= 0.01f; // increase velocity
//                     std::cout << "(increased) Current Velocity: " << velocity_fwd << std::endl;
//                     break;
//                 // emergency stop
//                 case 'x': // right arrow key
//                     velocity_fwd = 0.00f; // decrease velocity
//                     std::cout << "(decreased) Current Velocity: " << velocity_fwd << std::endl;
//                     velocity_rtt = 0.00f; // decrease velocity
//                     std::cout << "(decreased) Current Rotating Velocity: " << velocity_rtt << std::endl;
//                     break;
//             }
//             // read current motor velocity in rpm
//             wheel_v = canController.readonce_2_float(0x2051842);
//             printf("1Current speed is %f \n",wheel_v);
//             wheel_v = wheel_v / 4.71 * 38 / 1000;
//             printf("flag");
//             // calculate rotation speed of motor
//             velocity_rtt = wheel_v / radius;

//             // update steering speed
//             canController.setRotateVelocity(can_id_rtt_5, velocity_rtt);
//             canController.setRotateVelocity(can_id_rtt_6, velocity_rtt);
//             canController.setRotateVelocity(can_id_rtt_7, velocity_rtt);

//             canController.setRotateVelocity(can_id_fwd_2, velocity_fwd);
//             canController.setRotateVelocity(can_id_fwd_3, velocity_fwd);
//             canController.setRotateVelocity(can_id_fwd_4, velocity_fwd);

//         }
//         // Sleep for 100 microseconds
//         std::this_thread::sleep_for(std::chrono::microseconds(500));
        
//     }
//     Keyboard::resetTermios();
//     return 0;
// }

