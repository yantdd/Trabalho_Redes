# Protocolo EST - Estufa Inteligente

**Disciplina:** SSC0142 - Redes de Computadores   

**Integrantes do Grupo:**
* Yan Trindade Meireles 
* Renato Calacina Spessotto 
* Rafael Perez Carmanhani 
* Luis Guilherme Zanetti 

---

## Sobre o Projeto
Implementação da Etapa 2 do Trabalho Prático de Redes de Computadores. Consiste no Protocolo EST desenvolvido na Etapa 1.

O sistema utiliza a arquitetura Cliente-Servidor com Sockets TCP, um cabeçalho fixo de 8 bytes em Big Endian e tratamento de concorrência com uso de threads.

---

## Compilação
Requisitos: Ambiente Linux, `gcc` e `make`.

Na pasta do projeto, execute os seguintes comandos no terminal:

```bash
make clean
make
```

---

## Guia de Testes

Para testar o sistema, é necessário abrir múltiplos terminais em simultâneo para simular os diferentes dispositivos distribuídos na rede.

### Teste 1: Ligação e Identificação
Avalia a capacidade do Gerenciador em aceitar ligações TCP e processar as mensagens de controle `CONNECT` e `CONNACK`.

1. **Terminal 1:** Inicie o Gerenciador executando: `./gerenciador`
2. **Terminal 2:** Inicie um Aquecedor (Tipo 3, ID 200) executando: `./atuador 200 3`
3. **Terminal 3:** Inicie um Resfriador (Tipo 4, ID 201) executando: `./atuador 201 4`

**Resultado Esperado:** Os atuadores confirmam a receção da mensagem `CONNACK`. O Gerenciador registra a ligação dos IDs 200 e 201 e aguarda.

### Teste 2: Publicação de Dados e Ação Dinâmica
Avalia o envio de dados do sensor (`PUBLISH_DATA`), a avaliação dos limites no servidor e o envio de comandos (`ACTUATOR_CMD`).

1. **Terminal 4:** Inicie um Sensor de Temperatura (Tipo 0, ID 100) executando: `./sensor 100 0`
2. No Terminal 4, introduza o valor **25** (Temperatura Ideal).
   * **Resultado:** O Gerenciador regista a leitura. Nenhuma ação é despachada para os atuadores.
3. No Terminal 4, introduza o valor **10** (Abaixo do limite).
   * **Resultado:** O Gerenciador dispara a ação. O Terminal 2 (Aquecedor) imprime na tela que foi LIGADO.
4. No Terminal 4, introduza o valor **40** (Acima do limite).
   * **Resultado:** O Terminal 3 (Resfriador) imprime na tela que foi LIGADO. O Aquecedor indica que foi DESLIGADO.

### Teste 3: Modularidade por Tipo de Dispositivo
Garante que a ação de uma variável não interfere em outra (exemplo: a humidade do solo não pode alterar o estado do aquecedor).

1. **Terminal 5:** Inicie um Atuador de Irrigação (Tipo 5, ID 300) executando: `./atuador 300 5`
2. **Terminal 6:** Inicie um Sensor de Humidade do Solo (Tipo 1, ID 101) executando: `./sensor 101 1`
3. No Terminal 6, introduza o valor **20** (Abaixo do limite).
   * **Resultado:** O Gerenciador liga APENAS a irrigação (Terminal 5). Os estados térmicos do Aquecedor e Resfriador (Terminais 2 e 3) permanecem inalterados.

### Teste 4: Consulta de Cliente Externo
Avalia a requisição de estado atual efetuada por um utilizador externo através das mensagens `CLIENT_REQ` e `CLIENT_RESP`.

1. Confirme que o Sensor de ID 100 registou alguma temperatura no Teste 2.
2. **Terminal 7:** Execute o programa cliente com o comando: `./cliente`

**Resultado Esperado:** O Cliente envia a requisição, imprime a última temperatura do Sensor 100 na tela e encerra a execução automaticamente. O Gerenciador registra o envio da resposta.

### Teste 5: Resiliência da Ligação
Avalia o tratamento do servidor perante quebras abruptas de ligação (fechamento inesperado do socket).

1. Vá ao Terminal 4 (Sensor de Temperatura) e pressione `Ctrl + C` para forçar a paragem.

**Resultado Esperado:** O Gerenciador detecta o fecho do socket, libera a thread correspondente, imprime `Dispositivo ID 100 desconectou` e continua a operar normalmente sem falhar os restantes processos.