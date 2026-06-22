CC = gcc
CFLAGS = -Wall -Wextra -g -pthread -std=c99

EXECUTABLES = gerenciador sensor atuador cliente

all: $(EXECUTABLES)

gerenciador: gerenciador.c protocolo.h
	$(CC) $(CFLAGS) gerenciador.c -o gerenciador $(LDFLAGS)

sensor: sensor.c protocolo.h
	$(CC) $(CFLAGS) sensor.c -o sensor $(LDFLAGS)

atuador: atuador.c protocolo.h
	$(CC) $(CFLAGS) atuador.c -o atuador $(LDFLAGS)

cliente: cliente.c protocolo.h
	$(CC) $(CFLAGS) cliente.c -o cliente $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES)


.PHONY: all clean