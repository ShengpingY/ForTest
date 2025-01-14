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

#include "can_receive.h"



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
            wheel_position = UnsignedCharToFloat_32(frame_rotate_read.data[3],frame_rotate_read.data[2],frame_rotate_read.data[1],frame_rotate_read.data[0]);
            
            // printf("transfered float value is :%f \n", wheel_velocity);
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
