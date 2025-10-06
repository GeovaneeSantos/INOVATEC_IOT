# ⚙️ Código Fonte: Monitoramento IoT Industrial - NORVI GSM AE04 L (ESP32)

---

Este documento descreve o código-fonte C++ desenvolvido para o microcontrolador **ESP32** embutido no módulo **NORVI GSM AE04 L**. O programa é responsável por:

1. Estabelecer conexão Wi-Fi e MQTT.
2. Ler dados analógicos (Corrente, Vazão, Pressão) através do conversor **ADS1115**.
3. Ler o estado de seis entradas digitais (DI0 a DI5).
4. Converter as leituras brutas em Unidades de Engenharia (UE).
5. Montar uma mensagem **JSON** com todos os dados.
6. Publicar a mensagem em um tópico MQTT.

## 🌟 Visão Geral do Hardware e Software

| Componente | Função | Detalhes no Código |
| :--- | :--- | :--- |
| **NORVI GSM AE04 L (ESP32)** | Microcontrolador principal. | Lida com Wi-Fi, MQTT e lógica do programa. |
| **ADS1115** | Conversor Analógico-Digital de 16-bits. | Usado para ler as entradas analógicas de 4-20mA (Corrente, Vazão, Pressão). |
| **Entradas Digitais (DI0-DI5)** | Leitura de sensores digitais (e.g., status de chave/relé). | Pinos 27, 34, 35, 14, 13, 5. |
| **Protocolo** | **MQTT** | Broker: `test.mosquitto.org`, Tópico: `texto_norvi_wifi`. |

## 🛠️ Instalação e Configuração

### Pré-requisitos (Bibliotecas)

O projeto requer as seguintes bibliotecas na sua IDE Arduino:

* **`Adafruit_ADS1X15`**: Para comunicação com o conversor ADC externo.
* **`PubSubClient`**: Para comunicação via protocolo MQTT.
* **`ArduinoJson`**: Para manipulação e serialização da mensagem JSON.

### Parâmetros de Conexão

Antes de compilar e fazer o *upload* do código, atualize as seguintes constantes:

| Constante | Descrição | Exemplo Padrão |
| :--- | :--- | :--- |
| `ssid` | Nome da Rede Wi-Fi. | `""` |
| `password` | Senha da Rede Wi-Fi. | `""` |
| `mqtt_host` | Endereço do Servidor MQTT (Broker). | `"test.mosquitto.org"` |
| `mqtt_topic` | Tópico para publicação dos dados. | `"texto_norvi_wifi"` |

---

## 🔌 Detalhes do Código

### 1. Conversão de Unidades de Engenharia (UE) - Incluindo a Soma da Mínima da Saída

As funções de cálculo (`calcularCorrente`, `calcularVazao`, `calcularPressao`) implementam a **interpolação linear**, essencial para mapear o valor digital (ADC) para a escala real do sensor (UE).

O cálculo agora adiciona a **Mínima da Saída** ($\mathbf{UE_{\text{mín}}}$), garantindo que a escala comece no valor mínimo configurado, o que é fundamental para sensores com *offset* ou calibração de zero.

| Medição | Fórmula (Interpolação Linear) | Variável Mínima da Saída (`MIN_SAIDA`) |
| :--- | :--- | :--- |
| **Corrente** | $\text{UE} = \left(\frac{\text{corrente\_max} - \text{corrente\_min}}{\text{canal\_max} - \text{canal\_min}}\right) \times \text{valor\_adc} \mathbf{+ \text{corrente\_min}}$ | `corrente_min` (Valor atual: 0.0) |
| **Vazão** | $\text{UE} = \left(\frac{\text{vazao\_max} - \text{vazao\_min}}{\text{canal\_max} - \text{canal\_min}}\right) \times \text{valor\_adc} \mathbf{+ \text{vazao\_min}}$ | `vazao_min` (Valor atual: 0.0) |
| **Pressão** | $\text{UE} = \left(\frac{\text{pressao\_max} - \text{pressao\_min}}{\text{canal\_max} - \text{canal\_min}}\right) \times \text{valor\_adc} \mathbf{+ \text{pressao\_min}}$ | `pressao_min` (Valor atual: 0.0) |

***

### 2. Leitura de Entradas Digitais

O código lê o estado dos pinos de entrada digital (DI0 a DI5) e inverte a lógica (`1` = `false`, `0` = `true`) antes de armazenar os valores nas variáveis booleanas que serão publicadas.

### 3. Montagem e Publicação JSON

O objeto `StaticJsonDocument<300> doc;` é usado para criar o *payload* da mensagem MQTT, que é publicado a cada **5 segundos** (`delay(5000)`).

| Campo JSON | Origem | Tipo |
| :--- | :--- | :--- |
| `entrada_digital_1` a `entrada_digital_6` | Estado das entradas digitais (String: "true" / "false"). | String |
| `Corrente` | Valor convertido em Amperes. | Float |
| `Pressao` | Valor convertido em bar. | Float |
| `vazao` | Valor convertido em $\text{m}^3/\text{h}$. | Float |
