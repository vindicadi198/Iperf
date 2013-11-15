CFLAGS += -g -std=c99 -O3 -Wall -c -I../include
LDFLAGS = -lm 
IND_SRC = iperf_api.c iperf_defaults.c iperf_udp.c iperf_auto_udp.c main.c 
IND_OBJ = $(IND_SRC:.c=.o)
IND_OUT = $(IND_SRC:.c=)
TARGET = iperf

.PHONY: all clean test rmobj 


all: $(TARGET) rmobj 

$(TARGET): $(IND_OBJ)
	$(CC) $(LDFLAGS) *.o -o $@

iperf_api.o: iperf_api.c
	$(CC) $(CFLAGS) -o $@ $<

iperf_defaults.o: iperf_defaults.c
	$(CC) $(CFLAGS) -o $@ $<

iperf_udp.o: iperf_udp.c
	$(CC) $(CFLAGS) -o $@ $<

iperf_auto_udp.o: iperf_auto_udp.c
	$(CC) $(CFLAGS) -o $@ $<

main.o: main.c
	$(CC) $(CFLAGS) -o $@ $<

rmobj:
	rm *.o

clean:
	rm -f *.o
	rm -f $(TARGET)
