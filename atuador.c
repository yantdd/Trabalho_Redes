#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "protocolo.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Uso: %s <DEVICE_ID> <TIPO_ATUADOR>\n", argv[0]);
        printf("Tipos: 3=Aquecedor, 4=Resfriador, 5=Irrigacao, 6=InjetorCO2\n");
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
        printf("[Atuador %d] Conectado. Aguardando comandos do Gerenciador...\n", meu_id);
    }

    while (recv(sock, &resposta, sizeof(resposta), 0) > 0)
    {
        if (resposta.msg_type == MSG_ACTUATOR_CMD)
        {
            uint8_t comando;
            recv(sock, &comando, 1, 0);
            if (comando == ATUADOR_ON)
            {
                printf(">>> [Atuador %d] LIGADO! <<<\n", meu_id);
            }
            else
            {
                printf(">>> [Atuador %d] DESLIGADO! <<<\n", meu_id);
            }
        }
    }

    close(sock);
    return 0;
}