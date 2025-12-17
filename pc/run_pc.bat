@echo off
echo Iniciando InfluxDB e Grafana...
docker-compose -f docker\docker-compose.yml up -d

timeout /t 5

echo Iniciando leitura da serial...
python scripts\serial_to_influx.py

pause
