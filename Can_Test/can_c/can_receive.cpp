
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
 
int
main(void)
{
	int s;
	int nbytes;
	struct sockaddr_can addr;
	struct can_frame frame;
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


    while(1){

        nbytes = read(s, &frame, sizeof(struct can_frame));

		if((frame.can_id &= ~CAN_EFF_FLAG) == 0x02051842){
			if (nbytes < 0) {
				perror("CAN Frame read error!");
				return -1;
			}
			printf("Received CAN Frame -> CANID: 0x%X Data: ", frame.can_id &= ~CAN_EFF_FLAG);
			for (int i = 0; i < frame.can_dlc; i++) {
				printf("%02X ", frame.data[i]);
			}
			printf("\n");
		}
    }
	
	return 0;
}