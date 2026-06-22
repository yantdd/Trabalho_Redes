#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "protocolo.h"

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    CabecalhoEST cab;
    uint16_t id_sensor_alvo = 100; // sensor de temperatura

    montar_cabecalho(&cab, MSG_CLIENT_REQ, 999, 2);

    send(sock, &cab, sizeof(cab), 0);
    uint16_t id_net = htons(id_sensor_alvo);
    send(sock, &id_net, sizeof(uint16_t), 0);
    printf("[Cliente] CLIENT_REQ enviado consultando ID %d\n", id_sensor_alvo);

    CabecalhoEST resposta;
    recv(sock, &resposta, sizeof(resposta), 0);

    if (resposta.msg_type == MSG_CLIENT_RESP)
    {
        uint16_t id_recebido;
        float valor_recebido;

        recv(sock, &id_recebido, sizeof(uint16_t), 0);
        recv(sock, &valor_recebido, sizeof(float), 0);

        printf("[Cliente] CLIENT_RESP recebido -> Sensor %d registrou: %.2f\n", ntohs(id_recebido), valor_recebido);
    }

    close(sock);
    return 0;
}