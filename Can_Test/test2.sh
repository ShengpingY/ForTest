cansend can0 82052C80#FFFFFFFFFFFFFFFF
cansend can0 82051D82#0000
sleep 0.3
for n in {0..1000};
        do cansend can0 82052C80#FFFFFFFFFFFFFFFF;
	cansend can0 02050082#bf666666000000000;
	sleep 0.0001;
done

