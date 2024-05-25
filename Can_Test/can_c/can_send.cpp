
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
 
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
 
#include <linux/can.h>
#include <linux/can/raw.h>
#include </home/dronelab/Can_Test/can_c/HexDecimal.h>
#include <thread>
#include <chrono>

void hexToUnsignedChars(uint32_t hexNumber, unsigned char* data) {
	for(int i = 0; i < 4; i++) {
		data[i] = static_cast<unsigned char>((hexNumber >> (24 - 8*i))& 0x0F);
	}
}

int main(void)
{
	int s;
	int nbytes;
	struct sockaddr_can addr;
	struct can_frame frame_rotate;
	struct can_frame frame_identify;
	struct can_frame frame_heartbeat;
	struct ifreq ifr;
 
	const char *ifname = "can0";
 
	if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Error while opening socket");
		return -1;
	}
 
	strcpy(ifr.ifr_name, ifname);
	ioctl(s, SIOCGIFINDEX, &ifr);
	
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	
 
	printf("%s at index %d\n", ifname, ifr.ifr_ifindex);
 
	if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Error in socket bind");
		return -2;
	}

 
	// frame.can_id  = 0x2051d82;
	// frame.can_id |= CAN_EFF_FLAG;
	// frame.can_dlc = 2;
	// frame.data[0] = 0x11;
	// frame.data[1] = 0x22;
	frame_heartbeat.can_id = 0x2052C80;
	frame_heartbeat.can_id |= CAN_EFF_FLAG;
	frame_heartbeat.can_dlc = 8;
	frame_heartbeat.data[0] = 0xFF;
	frame_heartbeat.data[1] = 0xFF;
	frame_heartbeat.data[2] = 0xFF;
	frame_heartbeat.data[3] = 0xFF;
	frame_heartbeat.data[4] = 0xFF;
	frame_heartbeat.data[5] = 0xFF;
	frame_heartbeat.data[6] = 0xFF;
	frame_heartbeat.data[7] = 0xFF;

	frame_identify.can_id  = 0x2051D82;
	frame_identify.can_id |= CAN_EFF_FLAG;
	frame_identify.can_dlc = 2;
	frame_identify.data[0] = 0x00;
	frame_identify.data[1] = 0x00;

	frame_rotate.can_id  = 0x2050082;
	frame_rotate.can_id |= CAN_EFF_FLAG;
	frame_rotate.can_dlc = 8;
	// frame_rotate.data[0] = 0x3DU;
	// frame_rotate.data[1] = 0xCCU;
	// frame_rotate.data[2] = 0xCCU;
	// frame_rotate.data[3] = 0xCDU;
	// frame_rotate.data[4] = 0x00U;
	// frame_rotate.data[5] = 0x00U;
	// frame_rotate.data[6] = 0x00U;
	// frame_rotate.data[7] = 0x00U;

    float floatValue = -1.00000000f;
    unsigned char hexBytes[4];

 
	nbytes = write(s, &frame_heartbeat, sizeof(struct can_frame));
	if (nbytes < 0) {
		perror("CAN Frame write error!");
		return -1;
	}
	// nbytes = write(s, &frame_identify, sizeof(struct can_frame));
	// printf("Wrote CAN Frame -> CANID: 0x%X Data: \n", frame_identify.can_id &= ~CAN_EFF_FLAG);
	// printf("Wrote CAN Frame -> CANID: 0x%X Data: \n", frame_identify.can_id);

	int i = 0;

	int k = 0;
    while(i < 1000000000){

        // nbytes = write(s, &frame_heartbeat, sizeof(struct can_frame));
        // if (nbytes < 0) {
        //     perror("CAN Frame write error!");
        //     return -1;
        // }
        // printf("Wrote CAN Frame -> CANID: 0x%X Data: ", frame_heartbeat.can_id);
        // for (int i = 0; i < frame_heartbeat.can_dlc; i++) {
        //     printf("%02X ", frame_heartbeat.data[i]);
        // }
        // printf("\n");



		// nbytes = write(s, &frame_rotate, sizeof(struct can_frame));
        // if (nbytes < 0) {
        //     perror("CAN Frame write error!");
        //     return -1;
        // }
		
		if (k > 1000) {
			nbytes = write(s, &frame_rotate, sizeof(struct can_frame));
			if (nbytes < 0) {
				perror("CAN Frame write error!");
				return -1;
			}
			floatValue = floatValue + 0.01;
			k = 0;

			floatToUnsignedChar(floatValue, hexBytes);
			
			// Convert unsigned char array to hexadecimal number
			uint32_t hexNumber = unsignedCharToHex(hexBytes);
			// printf("Float value: %f --> Unsigned char array to hexadecimal number: 0x%08X\n", floatValue, hexNumber);
			// hexToUnsignedChars(hexNumber, frame_rotate.data);
			// frame_rotate.data[0] = 0x3DU;
			// frame_rotate.data[1] = 0xCCU;
			// frame_rotate.data[2] = 0xCCU;
			// frame_rotate.data[3] = 0xCDU;
			// frame_rotate.data[4] = 0x00U;
			// frame_rotate.data[5] = 0x00U;
			// frame_rotate.data[6] = 0x00U;
			// frame_rotate.data[7] = 0x00U;
			frame_rotate.data[0] = hexBytes[3];
			frame_rotate.data[1] = hexBytes[2];
			frame_rotate.data[2] = hexBytes[1];
			frame_rotate.data[3] = hexBytes[0];
			frame_rotate.data[4] = 0x00U;
			frame_rotate.data[5] = 0x00U;
			frame_rotate.data[6] = 0x00U;
			frame_rotate.data[7] = 0x00U;

			//printf("Wrote CAN Frame -> CANID: 0x%X Data: ", frame_rotate.can_id &= ~CAN_EFF_FLAG);
			printf("Wrote CAN Frame -> CANID: 0x%X Data: ", frame_rotate.can_id);
			for (int i = 0; i < frame_rotate.can_dlc; i++) {
				printf("%02X ", frame_rotate.data[i]);
			}
			printf("\n");
		}
		

		// floatToUnsignedChar(floatValue, hexBytes);
		
		// // Convert unsigned char array to hexadecimal number
		// uint32_t hexNumber = unsignedCharToHex(hexBytes);
		// // printf("Float value: %f --> Unsigned char array to hexadecimal number: 0x%08X\n", floatValue, hexNumber);
		// // hexToUnsignedChars(hexNumber, frame_rotate.data);
		// // frame_rotate.data[0] = 0x3DU;
		// // frame_rotate.data[1] = 0xCCU;
		// // frame_rotate.data[2] = 0xCCU;
		// // frame_rotate.data[3] = 0xCDU;
		// // frame_rotate.data[4] = 0x00U;
		// // frame_rotate.data[5] = 0x00U;
		// // frame_rotate.data[6] = 0x00U;
		// // frame_rotate.data[7] = 0x00U;
		// frame_rotate.data[0] = hexBytes[3];
		// frame_rotate.data[1] = hexBytes[2];
		// frame_rotate.data[2] = hexBytes[1];
		// frame_rotate.data[3] = hexBytes[0];
		// frame_rotate.data[4] = 0x00U;
		// frame_rotate.data[5] = 0x00U;
		// frame_rotate.data[6] = 0x00U;
		// frame_rotate.data[7] = 0x00U;

		// //printf("Wrote CAN Frame -> CANID: 0x%X Data: ", frame_rotate.can_id &= ~CAN_EFF_FLAG);
        // printf("Wrote CAN Frame -> CANID: 0x%X Data: ", frame_rotate.can_id);
		// for (int i = 0; i < frame_rotate.can_dlc; i++) {
        //     printf("%02X ", frame_rotate.data[i]);
        // }
        // printf("\n");
		std::this_thread::sleep_for(std::chrono::microseconds(500));
		i = i + 1;
		k = k + 1;
    }

 
	printf("Wrote %d bytes\n", nbytes);
	
	return 0;
}