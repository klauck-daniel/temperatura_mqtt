# temperatura_mqtt
Implementação de um sensor de temperatura com o uso do ESP32, freeRTOS e troca de mensagens em tópicos MQTT

Ao rodar a aplicação?
- Conecta na wifi
- Conecta ao broker test.mosquitto.org
- Publica nos tópicos "temperatura" "umidade" "ligar_cooler"
- Assina o tópico "ligar_cooler"

- Ao atingir + 25°C, o cooler ativa (quando o dispositivo está em automático)
- Ao enviar algum comando MQTT "0" ou "1" no tópico "ligar_cooler", o LED vermelho acende, indicando que o dispositivo está sendo operado de modo manual e não automático.

Para visualizar as mensagens no terminal CMD
- Instalar o Mosquitto no Windows;
- Rodar CMD como administrador;
- Navegar para a pasta onde o Mosquitto foi instalado;

- Para ASSINAR um tópico, rodar o comando:
	mosquitto_sub.exe -h test.mosquitto.org -p 1883 -t "topico_sensor_temperatura"

- Para PUBLICAR em um tópico, rodar o comando:
	mosquitto_pub.exe -h test.mosquitto.org -p 1883 -t "topico_sensor_temperatura" -m "mensagem a ser enviada"
	mosquitto_pub.exe -h test.mosquitto.org -p 1883 -t "topico_liga_cooler" -m "mensagem a ser enviada"

Sobre as tasks, são como funções, e podemos chamar uma para cada núcleo físico do processador. Com isso, garantimos uma latência menor na resposta do dispositivo.
- Task para ler o sensor, roda em um loop infinito, com um delay de 5s entre as leituras de temperatura.
- Task para acionamento ou resposta roda dentro de outro loop infinito e serve para acionar o cooler, por exemplo.

- Além disso, temos o void loop, que chama a função MQTT que mantem a comunicação com o broker ativa.

Mais referências em https://www.youtube.com/watch?v=nSCJfK3BRCQ
