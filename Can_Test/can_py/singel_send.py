from __future__ import print_function

import can

def send_one():
    bus = can.interface.Bus()

    msg = can.Message(
        arbitration_id = 0x123, data = [0,25,0,1,3,1,4,1],is_extended_id = True
    )

    try:
        bus.send(msg)
        print("Message sent on {}".format(bus.channel_info))
    except can.CanError:
        print("Can Error")

if __name__ == "__main__"
    send_one()