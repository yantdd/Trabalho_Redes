#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <stdint.h>

// códigos das Mensagens (MSG_TYPE)
#define MSG_CONNECT 0x01
#define MSG_CONNACK 0x02
#define MSG_PUBLISH_DATA 0x03
#define MSG_ACTUATOR_CMD 0x04
#define MSG_CLIENT_REQ 0x05
#define MSG_CLIENT_RESP 0x06
#define MSG_CONFIG_SET 0x07
#define MSG_CONFIG_ACK 0x08

// códigos dos tipos de dispositivos
#define TIPO_TEMP 0x00
#define TIPO_UMID_SOLO 0x01
#define TIPO_CO2 0x02
#define TIPO_AQUECEDOR 0x03
#define TIPO_RESFRIADOR 0x04
#define TIPO_IRRIGACAO 0x05
#define TIPO_INJETOR_CO2 0x06

// estados dos ttuadores
#define ATUADOR_OFF 0x00
#define ATUADOR_ON 0x01

// header fixo de 8 bytes
// __attribute__((packed)) p evitar padding da struct
typedef struct __attribute__((packed))
{
    uint8_t protocol_id[3]; // 'E', 'S', 'T'
    uint8_t msg_type;
    uint16_t device_id;
    uint16_t payload_len;
} CabecalhoEST;

// função para montar o cabeçalho
void montar_cabecalho(CabecalhoEST *cabecalho, uint8_t tipo_msg, uint16_t id, uint16_t tam_payload)
{
    cabecalho->protocol_id[0] = 0x45; // 'E'
    cabecalho->protocol_id[1] = 0x53; // 'S'
    cabecalho->protocol_id[2] = 0x54; // 'T'
    cabecalho->msg_type = tipo_msg;
    // htons para garantir ordem de bytes
    cabecalho->device_id = htons(id);
    cabecalho->payload_len = htons(tam_payload);
}

#endif