Gêmeo Digital da Mesa – Fase 3

Este repositório corresponde à Fase 3 do projeto da mesa de equilíbrio, cujo objetivo é criar um gêmeo digital do sistema físico, permitindo a visualização em tempo real da orientação da mesa (pitch e roll) em um painel Grafana.

A Fase 3 utiliza como base o firmware desenvolvido na Fase 2, adicionando apenas a integração com o computador para coleta e visualização dos dados.

Visão Geral do Sistema

O sistema é composto pelos seguintes blocos:

ESP32-S3 responsável pelo controle físico da mesa

Sensor MPU6050 para leitura da orientação (pitch e roll)

Comunicação serial via USB

Script em Python no computador

InfluxDB para armazenamento dos dados

Grafana para visualização em tempo real

Fluxo de dados:

ESP32-S3 → Serial (UART) → Script Python → InfluxDB → Grafana

Pré-requisitos
Hardware

ESP32-S3

Joystick analógico

Dois servomotores

Sensor MPU6050

Cabo USB para conexão com o computador

Software

ESP-IDF v5.x instalado e configurado

Python 3.10 ou superior (instalado e no PATH)

InfluxDB instalado e em execução

Grafana instalado e em execução

Build e Flash do Firmware

O firmware utilizado nesta fase é o mesmo da Fase 2.

Dentro do repositório, execute:

idf.py build
idf.py flash

Após o flash, não execute idf.py monitor, pois a porta serial será utilizada pelo script Python.

Formato dos Dados Enviados pelo ESP32

O ESP32 envia periodicamente os dados do MPU6050 pela serial no formato JSON.

Exemplo:

{"pitch":-2.35,"roll":1.78}

Onde:

pitch representa a inclinação da mesa no eixo X (em graus)

roll representa a inclinação da mesa no eixo Y (em graus)

Execução do Script Python

A pasta pc/ contém o script responsável por:

Ler os dados enviados pela serial

Decodificar o JSON

Registrar os valores no InfluxDB

Passos:

Entrar na pasta:

cd pc

Instalar dependências (uma única vez):

python -m pip install -r requirements.txt

Inicializar o processo do Docker (inicia a grafana e o influx):

docker compose up -d

Executar o script:

python serial_to_influx.py

Durante a execução, o terminal exibirá mensagens indicando o envio dos dados para o banco.

InfluxDB

Os dados são gravados no InfluxDB com as seguintes configurações:

Bucket: mesa

Measurement: orientacao

Fields:

pitch

roll

Cada leitura do ESP32 gera um novo registro no banco de dados.

Configuração do Grafana
Acesso

Abrir no navegador:

http://localhost:3000

Login padrão:

Usuário: admin

Senha: admin

Data Source (InfluxDB)

Acessar Settings → Data Sources

Selecionar Add data source

Escolher InfluxDB

Configurar:

URL: http://localhost:8086

Query Language: Flux

Organization: mesa

Bucket: mesa

Token: token criado no InfluxDB

Salvar e testar a conexão

Criação do Dashboard
Gráfico de Pitch

Consulta Flux:

from(bucket: "mesa")
|> range(start: -1m)
|> filter(fn: (r) => r._measurement == "orientacao")
|> filter(fn: (r) => r._field == "pitch")

Gráfico de Roll

from(bucket: "mesa")
|> range(start: -1m)
|> filter(fn: (r) => r._measurement == "orientacao")
|> filter(fn: (r) => r._field == "roll")

Representação Visual

Pode-se utilizar painéis do tipo Gauge para representar:

Pitch

Roll

Intervalo sugerido:

Mínimo: -45

Máximo: +45

Resultado Esperado

Movimentação física da mesa via joystick

Leitura contínua do MPU6050

Envio periódico dos dados via serial

Armazenamento no InfluxDB

Visualização em tempo real no Grafana

O dashboard representa o gêmeo digital sincronizado com o comportamento físico da mesa.

Observações Importantes

Apenas um processo pode acessar a porta serial por vez

Não utilizar idf.py monitor enquanto o script Python estiver em execução

Caso não apareçam dados no Grafana:

Verificar se o script Python está rodando

Verificar bucket, measurement e token do InfluxDB

Conclusão

A Fase 3 completa o projeto ao integrar o sistema embarcado com ferramentas de visualização, permitindo acompanhar em tempo real a orientação da mesa e demonstrando a sincronização entre o sistema físico e o modelo digital.