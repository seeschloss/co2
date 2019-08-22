all: co2

co2: co2.c
	gcc co2.c -lwiringPi -oco2

install: co2
	install -m a+x,u+s co2 /usr/bin
