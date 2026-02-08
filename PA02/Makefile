# Makefile for PA02 Assignment
#MT25034

CC = gcc
CFLAGS = -pthread

# Targets
all: MT25034_Part_A1_Server MT25034_Part_A1_Client \
     MT25034_Part_A2_Server MT25034_Part_A2_Client \
     MT25034_Part_A3_Server MT25034_Part_A3_Client

MT25034_Part_A1_Server: MT25034_Part_A1_Server.c
	$(CC) $(CFLAGS) -o $@ $<

MT25034_Part_A1_Client: MT25034_Part_A1_Client.c
	$(CC) $(CFLAGS) -o $@ $<

MT25034_Part_A2_Server: MT25034_Part_A2_Server.c
	$(CC) $(CFLAGS) -o $@ $<

MT25034_Part_A2_Client: MT25034_Part_A2_Client.c
	$(CC) $(CFLAGS) -o $@ $<

MT25034_Part_A3_Server: MT25034_Part_A3_Server.c
	$(CC) $(CFLAGS) -o $@ $<

MT25034_Part_A3_Client: MT25034_Part_A3_Client.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f MT25034_Part_A1_Server MT25034_Part_A1_Client \
	      MT25034_Part_A2_Server MT25034_Part_A2_Client \
	      MT25034_Part_A3_Server MT25034_Part_A3_Client
