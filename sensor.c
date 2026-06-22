#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "protocolo.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Uso: %s <DEVICE_ID> <TIPO_SENSOR>\n", argv[0]);
        printf("Tipos: 0=Temp, 1=Umid, 2=CO2\n");
        return 1;
    }

    uint16_t meu_id = atoi(argv[1]);
    uint8_t meu_tipo = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    CabecalhoEST cab;
    montar_cabecalho(&cab, MSG_CONNECT, meu_id, 1);
    send(sock, &cab, sizeof(cab), 0);
    send(sock, &meu_tipo, 1, 0);

    CabecalhoEST resposta;
    recv(sock, &resposta, sizeof(resposta), 0);
    if (resposta.msg_type == MSG_CONNACK)
    {
        printf("[Sensor %d] Conectado com sucesso!\n", meu_id);
    }

    float leitura;
    while (1)
    {
        printf("\nDigite o valor para enviar (ou Ctrl+C para sair): ");
        if (scanf("%f", &leitura) != 1)
            break;

        montar_cabecalho(&cab, MSG_PUBLISH_DATA, meu_id, 4);
        send(sock, &cab, sizeof(cab), 0);
        send(sock, &leitura, sizeof(float), 0);
        printf("[Sensor %d] PUBLISH_DATA enviado: %.2f\n", meu_id, leitura);
    }

    close(sock);
    return 0;
}