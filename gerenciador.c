#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "protocolo.h"

#define PORTA 8080
#define MAX_DISPOSITIVOS 20

typedef struct
{
    int socket_fd;
    uint16_t device_id;
    uint8_t tipo_dispositivo;
    float ultima_leitura;
    float limite_min;
    float limite_max;
    int conectado;
} Dispositivo;

Dispositivo dispositivos[MAX_DISPOSITIVOS];
pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;

int buscar_socket_atuador(uint8_t tipo_atuador_desejado)
{
    for (int i = 0; i < MAX_DISPOSITIVOS; i++)
    {
        if (dispositivos[i].conectado && dispositivos[i].tipo_dispositivo == tipo_atuador_desejado)
        {
            return dispositivos[i].socket_fd;
        }
    }
    return -1;
}

// Controle da estufa
void verificar_limites(int index_sensor, float leitura)
{
    uint8_t tipo = dispositivos[index_sensor].tipo_dispositivo;
    float min = dispositivos[index_sensor].limite_min;
    float max = dispositivos[index_sensor].limite_max;

    int sock_atuador_1 = -1;
    int sock_atuador_2 = -1;
    uint8_t acao_1 = 255, acao_2 = 255;

    if (tipo == TIPO_TEMP)
    {
        sock_atuador_1 = buscar_socket_atuador(TIPO_AQUECEDOR);
        sock_atuador_2 = buscar_socket_atuador(TIPO_RESFRIADOR);

        if (leitura < min)
        {
            acao_1 = ATUADOR_ON;  // liga aquecedor
            acao_2 = ATUADOR_OFF; // desliga resfriador
        }
        else if (leitura > max)
        {
            acao_1 = ATUADOR_OFF; // desliga aquecedor
            acao_2 = ATUADOR_ON;  // liga resfriador
        }
        else
        {
            acao_1 = ATUADOR_OFF;
            acao_2 = ATUADOR_OFF; // temp ideal
        }
    }
    else if (tipo == TIPO_UMID_SOLO)
    {
        sock_atuador_1 = buscar_socket_atuador(TIPO_IRRIGACAO);
        if (leitura < min)
            acao_1 = ATUADOR_ON;
        else if (leitura > max)
            acao_1 = ATUADOR_OFF;
    }
    else if (tipo == TIPO_CO2)
    {
        sock_atuador_1 = buscar_socket_atuador(TIPO_INJETOR_CO2);
        if (leitura < min)
            acao_1 = ATUADOR_ON;
        else if (leitura > max)
            acao_1 = ATUADOR_OFF;
    }

    // Envia os comandos se os atuadores estiverem conectados e a ação foi definida
    CabecalhoEST cab_cmd;
    montar_cabecalho(&cab_cmd, MSG_ACTUATOR_CMD, 0, 1);

    if (sock_atuador_1 != -1 && acao_1 != 255)
    {
        send(sock_atuador_1, &cab_cmd, sizeof(cab_cmd), 0);
        send(sock_atuador_1, &acao_1, 1, 0);
        printf("[Gerenciador] Comando enviado p/ Atuador 1 (Ação: %d)\n", acao_1);
    }
    if (sock_atuador_2 != -1 && acao_2 != 255)
    {
        send(sock_atuador_2, &cab_cmd, sizeof(cab_cmd), 0);
        send(sock_atuador_2, &acao_2, 1, 0);
        printf("[Gerenciador] Comando enviado p/ Atuador 2 (Ação: %d)\n", acao_2);
    }
}

void *tratar_conexao(void *arg)
{
    int sock = *(int *)arg;
    free(arg);
    CabecalhoEST cabecalho;
    int index_disp = -1;

    while (recv(sock, &cabecalho, sizeof(CabecalhoEST), 0) > 0)
    {
        uint16_t id = ntohs(cabecalho.device_id);
        uint16_t len = ntohs(cabecalho.payload_len);

        (void)len;

        if (cabecalho.msg_type == MSG_CONNECT)
        {
            uint8_t tipo;
            recv(sock, &tipo, 1, 0);
            printf("[Gerenciador] CONNECT -> ID: %d, Tipo: %d\n", id, tipo);

            CabecalhoEST resp;
            montar_cabecalho(&resp, MSG_CONNACK, id, 0);
            send(sock, &resp, sizeof(resp), 0);

            pthread_mutex_lock(&trava);
            for (int i = 0; i < MAX_DISPOSITIVOS; i++)
            {
                if (!dispositivos[i].conectado)
                {
                    dispositivos[i].socket_fd = sock;
                    dispositivos[i].device_id = id;
                    dispositivos[i].tipo_dispositivo = tipo;

                    // define limites baseados no tipo do sensor
                    if (tipo == TIPO_TEMP)
                    {
                        dispositivos[i].limite_min = 15.0;
                        dispositivos[i].limite_max = 30.0;
                    }
                    else if (tipo == TIPO_UMID_SOLO)
                    {
                        dispositivos[i].limite_min = 40.0;
                        dispositivos[i].limite_max = 80.0;
                    }
                    else if (tipo == TIPO_CO2)
                    {
                        dispositivos[i].limite_min = 400.0;
                        dispositivos[i].limite_max = 1000.0;
                    }

                    dispositivos[i].conectado = 1;
                    index_disp = i;
                    break;
                }
            }
            pthread_mutex_unlock(&trava);
        }
        else if (cabecalho.msg_type == MSG_PUBLISH_DATA)
        {
            float leitura;
            recv(sock, &leitura, sizeof(float), 0);
            printf("[Gerenciador] PUBLISH_DATA (ID %d): %.2f\n", id, leitura);

            pthread_mutex_lock(&trava);
            if (index_disp != -1)
            {
                dispositivos[index_disp].ultima_leitura = leitura;
                verificar_limites(index_disp, leitura);
            }
            pthread_mutex_unlock(&trava);
        }
        else if (cabecalho.msg_type == MSG_CLIENT_REQ)
        {
            uint16_t id_alvo;
            recv(sock, &id_alvo, sizeof(uint16_t), 0);
            id_alvo = ntohs(id_alvo);

            float ultima_leitura = 0.0;
            for (int i = 0; i < MAX_DISPOSITIVOS; i++)
            {
                if (dispositivos[i].conectado && dispositivos[i].device_id == id_alvo)
                {
                    ultima_leitura = dispositivos[i].ultima_leitura;
                    break;
                }
            }

            CabecalhoEST resp;
            montar_cabecalho(&resp, MSG_CLIENT_RESP, id, 6);
            send(sock, &resp, sizeof(resp), 0);

            uint16_t id_net = htons(id_alvo);
            send(sock, &id_net, sizeof(uint16_t), 0);
            send(sock, &ultima_leitura, sizeof(float), 0);
        }
    }

    if (index_disp != -1)
    {
        printf("[Gerenciador] Dispositivo ID %d desconectou.\n", dispositivos[index_disp].device_id);
        dispositivos[index_disp].conectado = 0;
    }
    close(sock);
    return NULL;
}

int main()
{
    int server_fd, novo_socket;
    struct sockaddr_in endereco;
    int addrlen = sizeof(endereco);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = INADDR_ANY;
    endereco.sin_port = htons(PORTA);

    bind(server_fd, (struct sockaddr *)&endereco, sizeof(endereco));
    listen(server_fd, 5);
    printf("Gerenciador da Estufa Inteligente iniciado...\n");

    while (1)
    {
        novo_socket = accept(server_fd, (struct sockaddr *)&endereco, (socklen_t *)&addrlen);
        int *novo_sock_ptr = malloc(sizeof(int));
        *novo_sock_ptr = novo_socket;

        pthread_t thread;
        pthread_create(&thread, NULL, tratar_conexao, (void *)novo_sock_ptr);
        pthread_detach(thread);
    }
    return 0;
}