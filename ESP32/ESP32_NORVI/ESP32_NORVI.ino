#include <Adafruit_ADS1X15.h>
#include <Arduino.h>
#include <Wire.h> 
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DI0_PIN 27
#define DI1_PIN 34
#define DI2_PIN 35
#define DI3_PIN 14
#define DI4_PIN 13
#define DI5_PIN 5

const char* ssid = ""; // SSID DA REDE
const char* password = "";  // SENHA DA REDE
const char* mqtt_host = "test.mosquitto.org"; // HOST DO SERVIDOR MQTT (BROKER MOSQUITTO)
const uint16_t mqtt_port = 1883; // PORTA DE COMUNICAÇ~ÇAO COM O BORKER
const char* mqtt_client_id = "NORVI_ESP32"; // ID DO DISPOSITIVO
const char* mqtt_topic = "texto_norvi_wifi"; // TÓPICO MQTT 

// INSTANCIA A FUNÇÃO QUE TORNA O DISPOSITIVO UM CLIENTE WIFI
WiFiClient wifi_client;
// INSTANCIA A FUNÇÃO QUE PERMITE O DISPOSITIVO USAR MQTT POR MEIO DE UM CLIENTE WIFI
PubSubClient mqtt(wifi_client);

// ADS EXTERNO, USA TENSÃO ENTRE O PINO ANALÓGICO E O TERRA PARA RETORNAR UM VALOR DIGITAL
Adafruit_ADS1115 ads1;

// FUNÇÃO PARA CONVERTER O VALOR DIGITAL EM UE, NO CASO, CALCULA A CORRENTE
float calcularCorrente(int valor_adc){
    float corrente_max, corrente_min, canal_max, canal_min, corrente_real;  

    // CORRENTE MAX/MIN REFERE-SE AO QUANTO O SENSOR É CAPAZ DE LER, NO CASO, O SENSOR PODE LER DE 0 ATÉ 150 AMPERES
    corrente_max = 150.0;
    corrente_min = 0.0;

    // TENDO-SE O MIN E MÁXIMO DO CANAL PODEMOS USAR INTERPOLAÇÃO LINEAR, AFINAL CADA PONTO DE UMA RETA REFERENTE AO CANAL, TERÁ UM UM PONTO NA RETA DA CORRENTE
    canal_max = 16000.0;
    canal_min = 0.0;

    // CALCULO DE CORRENTE 
    // EXEMPLO:  42,1875 = (((150 - 0) / (16000 - 0)) *  4500) + MIN DA SAIDA
    corrente_real = (((corrente_max - corrente_min)/(canal_max - canal_min)) * valor_adc) + 0;

    return corrente_real;
}

float calcularVazao(int valor_adc){
    float vazao_max, vazao_min, canal_max, canal_min, vazao_real; 
    vazao_max = 1000.0;
    vazao_min = 0.0;
    canal_max = 16000.0;
    canal_min = 0.0;
  
    vazao_real = (((vazao_max - vazao_min)/(canal_max - canal_min)) * valor_adc) + 0;

    return vazao_real;
}

float calcularPressao(int valor_adc){
    float pressao_max, pressao_min, canal_max, canal_min, pressao_real; 
    pressao_max = 150.0;
    pressao_min = 0.0;
    canal_max = 16000.0;
    canal_min = 0.0;
  
    pressao_real = (((pressao_max - pressao_min)/(canal_max - canal_min)) * valor_adc) + 0;

    return pressao_real;
}

void setup(void) {
  Serial.begin(115200); // INICIA COMUNICAÇÃO SERIAL COM BOUD RATE EM 115200 
  delay(10);
  Serial.println("Iniciando Norvi Wi-Fi e sensores...");
  Wire.begin(16, 17); INICIA PINOS I2C PARA COMUNICAÇÃO SERIAL
  ads1.begin(0x48); // INICIA O ADS NO ENDEREÇO 0x48
  ads1.setGain(GAIN_ONE); // DEFINE O GANHO, ISSO FAZ COM QUE A RETA DE VALORES DIGITAIS TENHA MAIS PONTOS, TORNANDO A CONVERSÃO MAIS PRECISA

  // MAPEAMENTO DOS PINOS DE ENTRADAS DIGITAIS
  pinMode(DI0_PIN, INPUT);
  pinMode(DI1_PIN, INPUT);
  pinMode(DI2_PIN, INPUT);
  pinMode(DI3_PIN, INPUT);
  pinMode(DI4_PIN, INPUT);
  pinMode(DI5_PIN, INPUT);

  // PROTOCOLO DE CONEXÃO WIFI
  Serial.print("Conectando a ");
  Serial.println(ssid); // MOSTRA A QUAL WIFI ESTA TENTANDO SE CONECTAR
  WiFi.begin(ssid, password); // TENTA ESTABELECER UMA CONEXÃO WIFI USANDO O SSID E SENHA FORNECIDOS
  
  // ENQUANTO A FUNÇÃO WIFI.STATUS() NÃO RETORNAR A MENSAGEM WL_CONNECTED ELE IRA FICAR NESSE LOOP E PRINTANDO " ......."
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // A EXECUÇÃO DO RESTO DO CÓDIGO PRESSUPÔE QUE OS BLOCOS ACIMA FORAM EXECUTADOS E OBTIVERAM SUCESSO EM SUAS FUNÇÕES


  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());

  // TENTA ESTABELECER UMA COMUNICAÇÃO COM O BROKER MQTT
  Serial.println("Conectando ao broker MQTT...");
  mqtt.setServer(mqtt_host, mqtt_port); // HOST E PORTA FORNECIDAS
  mqtt.setBufferSize(512); // TAMANHO DO BUFFER DEDICADO AO MQTT
  

  // !MQTT.CONNECT FAZ UMA PERGUNTA AO BROKER, (MQTT_CLIENT_ID) ESTÁ CONECTADO ?, SE SIM ELE CONTINUA O CÓDIGO, SENÃO ELE CAI NUM LOOP INFINITO
  // ISSO SERVE PARA QUE O DISPOSITIVO NÃO FIQUE ALOCANDO MEMÓRIA E PROCESSANDO OS DADOS SEM POSTAR ELES, PORTANDO DEVE SER EXECUTADO PELO MENOS UMA VEZ NO SETUP()
  if (!mqtt.connect(mqtt_client_id)) {
    Serial.println("Falha ao conectar ao broker MQTT.");
    while (true);
  }
  Serial.println("Conectado ao MQTT!");
}

void loop(void) {
    
    // SE PERDER A CONEXÃO COM BROKER ELE TENTA REESTABELECER
    if (!mqtt.connected()) {
    Serial.println("Reconectando ao MQTT...");
    if (!mqtt.connect(mqtt_client_id)) {
      Serial.println("Falha na reconexao.");
      delay(5000); 
      return;
    }
  }

  bool di0, di1, di2, di3, di4, di5; 

  if (digitalRead(DI0_PIN) == 1){
    di0 = false;
  }else{
    di0 = true;
  }

  if (digitalRead(DI1_PIN) == 1){
    di1 = false;
  }else{
    di1 = true;
  }

  if (digitalRead(DI2_PIN) == 1){
    di2 = false;
  }else{
    di2 = true;
  }

  if (digitalRead(DI3_PIN) == 1){
    di3 = false;
  }else{
    di3 = true;
  }

  if (digitalRead(DI4_PIN) == 1){
    di4 = false;
  }else{
    di4 = true;
  }

  if (digitalRead(DI5_PIN) == 1){
    di5 = false;
  }else{
    di5 = true;
  }

  // USA O ADS PARA MAPEAR AS ENTRADAS ANALÓGICAS E APLICAR A FUNÇÃO DE CONVERSÃO PARA ESCALAS DE ENGENHARIA
  float corrente = calcularCorrente(ads1.readADC_SingleEnded(0));
  float vazao = calcularVazao(ads1.readADC_SingleEnded(1));
  float pressao = calcularPressao(ads1.readADC_SingleEnded(2));

  // CRIA UM OBJETO JSON
  StaticJsonDocument<300> doc; 

  
  Serial.print("Corrente:");
  Serial.print(corrente);
  Serial.println("A");

  Serial.print("Vazão:");
  Serial.print(vazao);
  Serial.println("m³/h");

  Serial.print("Pressão:");
  Serial.print(pressao);
  Serial.println("bar");

    // O DISPOSITIVO USADO PARA TESTES PASSAVA DE 20mA, O QUE PODERIA DANIFICAR O NORVI AE04 L, PARA ISSO DEFINI UM ALERTA CASO O ADS ULTRAPASSE O VALOR REFERENTE A 20mA
    while(ads1.readADC_SingleEnded(0) > 16000){
      delay(100);
      Serial.println("Corrente de entrada no pino 0 fora do padrão, risco de danificar equipamento !");
    }

    while(ads1.readADC_SingleEnded(1) > 16000){
      delay(100);
      Serial.println("Corrente de entrada no pino 1 fora do padrão, risco de danificar equipamento !");
    }

    while(ads1.readADC_SingleEnded(2) > 16000){
      delay(100);
      Serial.println("Corrente de entrada no pino 2 fora do padrão, risco de danificar equipamento !");
    }

  // INSTANCIA O JSON COM OS VALORES LÓGICOS DAS ENTRADAS DIGITAIS 
  doc["entrada_digital_1"] = String(di0);
  doc["entrada_digital_2"] = String(di1);
  doc["entrada_digital_3"] = String(di2);
  doc["entrada_digital_4"] = String(di3);
  doc["entrada_digital_5"] = String(di4);
  doc["entrada_digital_6"] = String(di5);

  // INSTANCIA O JSON COM OS VALORES ANALÓGICOS DE LEITURA CONVERTIDOS EM UE 
  doc["Corrente"] = corrente;
  doc["Pressao"] = pressao;
  doc["vazao"] = vazao;

  // TRANSFORMA O JSON EM UM VETOR DE CARACTERES
  char output_json[300];
  serializeJson(doc, output_json); // SERIALIZA O JSON DE SAIDA COM OS DADOS DO JSON INTERNO
  
  // Publica o JSON no tópico MQTT
  if (mqtt.publish(mqtt_topic, output_json)) {
    Serial.println("JSON publicado com sucesso");
  } else {
    Serial.println("Falha na publicacao do JSON.");
  }

  delay(5000);
}
