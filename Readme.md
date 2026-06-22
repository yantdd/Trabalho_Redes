# Protocolo EST - Estufa Inteligente

**Disciplina:** SSC0142 - Redes de Computadores  


**Integrantes do Grupo:**
* Yan Trindade Meireles 
* Renato Calacina Spessotto
* Rafael Perez Carmanhani 
* Luis Guilherme Zanetti 

---

## 1. Sobre o Projeto
Implementação da Etapa 2 do Trabalho Prático de Redes de Computadores. Consiste na implementação em C do Protocolo EST desenvolvido na Etapa 1.

O sistema simula uma Estufa Inteligente e utiliza a arquitetura Cliente-Servidor com Sockets TCP, um cabeçalho fixo de 8 bytes na ordem Big Endian e tratamento de concorrência com a biblioteca pthreads.

---

## 2. Compilação
Requisitos: Ambiente Linux, compilador GCC e Make.

Na pasta do projeto, execute os seguintes comandos no terminal para compilar todos os arquivos:

    make clean
    make

Isso irá gerar os executáveis principais: gerenciador, sensor, atuador e cliente.

---

## 3. Guia de Testes

Para testar o sistema na íntegra, é necessário abrir múltiplos terminais simultaneamente para simular os diferentes dispositivos distribuídos na rede.

### Teste 1: Conexão e Identificação
Avalia a capacidade do Gerenciador em aceitar conexões TCP e processar as mensagens de controle CONNECT e CONNACK.

1. Terminal 1: Inicie o Gerenciador executando: ./gerenciador
2. Terminal 2: Inicie um Aquecedor (Tipo 3, ID 200) executando: ./atuador 200 3
3. Terminal 3: Inicie um Resfriador (Tipo 4, ID 201) executando: ./atuador 201 4

Resultado Esperado: Os atuadores confirmam o recebimento da mensagem CONNACK. O Gerenciador registra a conexão dos IDs 200 e 201 e aguarda.

### Teste 2: Publicação de Dados e Ação Dinâmica
Avalia o envio de dados do sensor (PUBLISH_DATA), a avaliação dos limites no servidor e o envio de comandos (ACTUATOR_CMD).

1. Terminal 4: Inicie um Sensor de Temperatura (Tipo 0, ID 100) executando: ./sensor 100 0
2. No Terminal 4, digite o valor 25 (Temperatura Ideal).
   * Resultado: O Gerenciador registra a leitura. Nenhuma ação é despachada para os atuadores.
3. No Terminal 4, digite o valor 10 (Abaixo do limite).
   * Resultado: O Gerenciador despacha a ação. O Terminal 2 (Aquecedor) imprime na tela que foi LIGADO.
4. No Terminal 4, digite o valor 40 (Acima do limite).
   * Resultado: O Terminal 3 (Resfriador) imprime na tela que foi LIGADO. O Aquecedor indica que foi DESLIGADO.

### Teste 3: Modularidade por Tipo de Dispositivo
Garante que a ação de uma variável não interfere em outra (exemplo: a umidade do solo não pode alterar o estado do aquecedor).

1. Terminal 5: Inicie um Atuador de Irrigação (Tipo 5, ID 300) executando: ./atuador 300 5
2. Terminal 6: Inicie um Sensor de Umidade do Solo (Tipo 1, ID 101) executando: ./sensor 101 1
3. No Terminal 6, digite o valor 20 (Abaixo do limite).
   * Resultado: O Gerenciador liga APENAS a irrigação (Terminal 5). Os estados térmicos do Aquecedor e Resfriador (Terminais 2 e 3) permanecem inalterados.

### Teste 4: Consulta de Cliente Externo
Avalia a requisição de estado atual efetuada por um usuário externo através das mensagens CLIENT_REQ e CLIENT_RESP.

1. Confirme que o Sensor de ID 100 registrou alguma temperatura no Teste 2.
2. Terminal 7: Execute o programa cliente com o comando: ./cliente

Resultado Esperado: O Cliente envia a requisição, imprime a última temperatura do Sensor 100 na tela e encerra a execução automaticamente. O Gerenciador registra o envio da resposta.

### Teste 5: Resiliência da Conexão
Avalia o tratamento do servidor perante quedas abruptas de conexão (fechamento inesperado do socket).

1. Vá ao Terminal 4 (Sensor de Temperatura) e pressione Ctrl + C para forçar a parada.

Resultado Esperado: O Gerenciador detecta o fechamento do socket, libera a thread correspondente, imprime "Dispositivo ID 100 desconectou" e continua a operar normalmente sem falhar os demais processos.
