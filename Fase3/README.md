# Gêmeo Digital de uma Mesa de Equilíbrio – Fase 3

## 1. Introdução

Este repositório corresponde à **Fase 3 do projeto de desenvolvimento de um gêmeo digital aplicado a uma mesa de equilíbrio**. O objetivo principal desta etapa é integrar o sistema embarcado ao ambiente computacional, permitindo a **aquisição, armazenamento e visualização em tempo real da orientação da mesa**, representada pelos ângulos de **pitch** e **roll**.

O conceito de **gêmeo digital** adotado neste trabalho consiste na criação de uma representação virtual sincronizada com o comportamento do sistema físico, possibilitando monitoramento contínuo e análise do estado dinâmico da mesa.

A Fase 3 utiliza como base o firmware desenvolvido na **Fase 2**, adicionando exclusivamente os mecanismos de comunicação com o computador e integração com ferramentas de visualização e banco de dados.

## 2. Arquitetura do Sistema

### 2.1 Visão Geral

O sistema é composto por módulos de hardware e software que atuam de forma integrada para aquisição, processamento e visualização dos dados de orientação da mesa.

Os principais componentes são:

- **ESP32-S3**: responsável pelo controle físico da mesa e leitura dos sensores;
- **Sensor MPU6050**: utilizado para obtenção dos ângulos de orientação (pitch e roll);
- **Comunicação Serial (USB/UART)**: meio de transmissão dos dados do sistema embarcado para o computador;
- **Script em Python**: responsável pela leitura da serial, decodificação dos dados e envio ao banco de dados;
- **InfluxDB**: banco de dados orientado a séries temporais para armazenamento das medições;
- **Grafana**: ferramenta de visualização para construção de dashboards em tempo real.

### 2.2 Fluxo de Dados

ESP32-S3 → Serial (UART) → Script Python → InfluxDB → Grafana

## 3. Requisitos de Hardware

- ESP32-S3  
- Sensor MPU6050  
- Joystick analógico  
- Dois servomotores  
- Cabo USB  

## 4. Requisitos de Software

- ESP-IDF v5.x  
- Python 3.10 ou superior  
- InfluxDB  
- Grafana  
- Docker e Docker Compose  

## 5. Firmware do Sistema Embarcado

O firmware utilizado nesta fase é o mesmo desenvolvido na **Fase 2 do projeto**.

### 5.1 Build e Flash

```bash
idf.py build
idf.py flash
```

Não utilizar `idf.py monitor`, pois a porta serial será utilizada pelo script Python.

## 6. Formato dos Dados Transmitidos

Exemplo de mensagem JSON:

```json
{"pitch":-2.35,"roll":1.78}
```

- **pitch**: inclinação no eixo X (graus)  
- **roll**: inclinação no eixo Y (graus)  

## 7. Script em Python

A pasta `pc/` contém o script responsável pela leitura serial e envio ao InfluxDB.
Nesta fase, o Docker deve estar ativado. Ao rodar o comando docker compose up -d, algo como:

Grafana Done
InfluxDB Done

Deve ser exibido no terminal.

```bash
cd pc
python -m pip install -r requirements.txt
docker compose up -d
python serial_to_influx.py
```

## 8. InfluxDB

- **Bucket:** mesa  
- **Measurement:** orientacao  
- **Fields:** pitch, roll  

## 9. Grafana

Acesso: http://localhost:3000  
Usuário: admin  
Senha: admin  

Consultas Flux:

```flux
from(bucket: "mesa")
  |> range(start: -1m)
  |> filter(fn: (r) => r._measurement == "orientacao")
  |> filter(fn: (r) => r._field == "pitch")
```

```flux
from(bucket: "mesa")
  |> range(start: -1m)
  |> filter(fn: (r) => r._measurement == "orientacao")
  |> filter(fn: (r) => r._field == "roll")
```

## 10. Resultados Esperados

- Leitura contínua do MPU6050  
- Envio serial dos dados  
- Armazenamento no InfluxDB  
- Visualização em tempo real no Grafana  

## 11. Conclusão

A Fase 3 consolida a aplicação do conceito de **gêmeo digital**, integrando o sistema físico a ferramentas modernas de armazenamento e visualização de dados.
