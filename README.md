# Monitoramento IoT Industrial 

---

Este repositório contém o fluxo do **Node-RED** para as ferramentas de monitoramento de dados analógicos e digitais do dispositivo **NORVI GSM AE04 L**. O sistema coleta leituras em tempo real por meio do protocolo **MQTT**, processa os dados de sensores de Corrente, Pressão e Vazão, e armazena em um banco de dados **MySQL** para posterior visualização e análise.

## Arquitetura do Projeto

O projeto é baseado na seguinte arquitetura de IoT:

1. **Dispositivo de Borda:** NORVI GSM AE04 L.
2. **Protocolo de Comunicação:** MQTT (Tópico: `texto_norvi_wifi`).
3. **Processamento:** Node-RED (Fluxo: **"Fluxo 1"**).
4. **Banco de Dados:** MySQL (`INOVATEC_IOT`).

### Componentes Chave

| Componente | Tipo | Função |
| :--- | :---: | --- |
| **NORVI GSM AE04 L** | Hardware | Coleta de dados analógicos (Corrente, Pressão, Vazão) e digitais (DI0 a DI5). Pode ser qualquer dispositivo capaz de fornecer esses dados, desde um clp até um microcomputador.|
| **MQTT Broker** | Software | Intermediação da mensagem, ele gerencia os **subscibers** e os **publishers** do servidor MQTT (`test.mosquitto.org`). |
| **Node: `Direcionamento de dados`** | `function` (JS) | Mapeia os dados brutos da mensagem MQTT e armazena Corrente, Pressão e Vazão em variáveis de fluxo (`flow`). |
| **Node: `QUERY EFETIVO_MOTOR`** | `function` (JS) | Prepara a query SQL de `INSERT` para dados do motor (Corrente, frequencia, nome da bomba/motor e painél responsável pela alimentação). |
| **Node: `QUERY EFETIVO_HIDRAULICO`** | `function` (JS) | Prepara a query SQL de `INSERT` para dados hidráulicos (Pressão e Vazão). |

---

## Instalação e Configuração

### Pré-requisitos

Para replicar ou simular esse ambiente será necessário:

1. Uma instância do **Node-RED** em execução.
2. Um servidor **MySQL** configurado.
3. Um dispositivo publicando dados no tópico MQTT.

### Dependências do Node-RED

Este fluxo utiliza os seguintes *nodes* adicionais. Instale-os via Palette Manager:

* `node-red-contrib-ui-led2` (versão `0.4.19`): Para visualização das entradas digitais.
* `node-red-dashboard` (versão `3.6.6`): Para a interface de usuário (Dashboards).
* `node-red-node-mysql` (versão `2.0.0`): Para conexão com o banco de dados.

### Configuração do Banco de Dados

A conexão `MySQLdatabase` está configurada como `INOVATEC_IOT` e as *queries* de inserção esperam a existência das seguintes tabelas:

1. `EFETIVO_MOTOR` (Recebe `TIMESTAMP`, `ID_PNL`, `NOME_BP`, `CORRENTE_LEITURA`, `FREQUENCIA_LEITURA`).
2. `EFETIVO_HIDRAULICO` (Recebe `TIMESTAMP`, `ID_PNL`, `PRESSAO_LEITURA`, `VAZAO_LEITURA`).
3. `EFETIVO_MEDIDOR` (Recebe dados de outro *insert*).

---

## Arquivos JSON de Configuração

Arquivos JSON de configuração estão disponíveis neste repositório. Eles são cruciais para a inicialização e o teste do ambiente.

### Como usar Estes Arquivos

Estes arquivos servem primariamente para criar o ambiente sem a necessidade de fazer isso do completo zero. 

* **JSON NODE-RED:** Esse arquivo pode ser usado para iniciar o ambiente node red, basta acessar o menu no lado superior direito > importar > selecionar o arquivo ou basta copiar e colar ele. O node-red se encarregará de fazer a construção de todo o fluxo.
* **JSON GRAFANA** Essencial para estabelecer o ambiente de dasboards no grafana

---

## Detalhes do Fluxo

O fluxo principal (`a5975144ae9a0dd0` - **"Fluxo 1"**) orquestra a coleta de dados:

### 1. Ingestão de Dados (MQTT)

* **Node:** `mqtt in` (`2bc9537afb0fa4ae`)
* **Tópico:** `texto_norvi_wifi`
* **Broker:** `test.mosquitto.org` (Padrão: `1883`)

### 2. Processamento e Mapeamento

O nó **`Direcionamento de dados`** (uma `function`) realiza o principal trabalho de *parsing* (análise) e distribuição, convertendo a *payload* JSON recebida.

```javascript
// Exemplo de Mapeamento:
// Mapeamento das Entradas Digitais
entradas.DI0 = dados.entrada_digital_1; // ... até DI5

// Armazenamento em Flow Context e Mapeamento para Dashboard
if (dados.Corrente !== undefined) { 
    dashboardDados.corrente = dados.Corrente;
    flow.set("corrente", dashboardDados.corrente); // Variável de Fluxo
}
// ... lógica similar para Pressao e Vazao
