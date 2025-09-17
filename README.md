# DataLogger-UneDuneTe

- Grupo 6:
- Murilo Umbelino Oliveira dos Santos - RA: 082230013
- Victor Mendes de Andrade Ferreira - RA: 082230015
- Lucas Junqueira Gonçalves - RA: 082230029

# Link de apresentação do projeto

- https://youtu.be/mJ0166kbTA8

## 📌 Descrição do Projeto
Este projeto consiste em um **Data Logger com Arduino** capaz de monitorar:
- **Temperatura** (sensor DHT11)  
- **Umidade relativa do ar** (sensor DHT11)  
- **Luminosidade** (sensor LDR)  

O sistema exibe os valores em um display LCD I2C 16x2 e registra **anomalias** (valores fora da faixa aceitável) na **EEPROM** do Arduino, incluindo data/hora do evento, temperatura, umidade e luminosidade.  

### 🔔 Funcionalidades principais
- Exibição de **data e hora em tempo real** (RTC DS1307).  
- Exibição de **telas individuais** para cada sensor através de **botões físicos**:  
  - Botão **HOME** → mostra Data e Hora.  
  - Botão **TEMP** → mostra Temperatura.  
  - Botão **UMID** → mostra Umidade.  
  - Botão **LUMI** → mostra Luminosidade.  
- **Alerta visual e sonoro**:  
  - LED vermelho pisca em caso de anomalia.  
  - Buzzer emite sinal sonoro.  
- **Registro em memória EEPROM**:  
  - Cada anomalia salva **timestamp, temperatura, umidade e luminosidade**.  
  - Os registros podem ser lidos via monitor serial.  

⚠️ Observações:  
- Para trocar de tela, o botão correspondente precisa ser **pressionado e segurado**.  
- Os **logs não são exibidos no LCD**, apenas gravados na EEPROM e acessados via Serial Monitor.  

---

## 🛠️ Especificações Técnicas

- **Microcontrolador**: Arduino Uno  
- **Sensores**:
  - DHT11 → Temperatura e Umidade  
  - LDR → Luminosidade (via porta analógica A0)  
- **Módulos**:
  - RTC DS1307 (com bateria CR2032)  
  - Display LCD 16x2 com interface I2C (endereço 0x27)  
- **Atuadores**:
  - LED vermelho (alerta)  
  - Buzzer (alerta sonoro)  
- **Botões**:
  - 4 botões (HOME, TEMP, UMID, LUMI) conectados com resistores internos `INPUT_PULLUP`.  
- **Memória**:
  - EEPROM interna do Arduino (100 registros, 10 bytes cada).  

---

## 📖 Manual de Operação

1. **Inicialização**  
   - Ao ligar o dispositivo, é exibida uma animação de inicialização no LCD.  
   - Em seguida, a tela **HOME** mostra a **data e hora atuais**.  

2. **Navegação entre telas**  
   - Pressione e **segure** o botão correspondente para mudar de tela:  
     - **HOME**: Data e Hora  
     - **TEMP**: Temperatura (°C)  
     - **UMID**: Umidade (%)  
     - **LUMI**: Luminosidade (%)  

3. **Alertas e Anomalias**  
   - Se algum valor estiver fora da faixa pré-definida, o LED vermelho acende e o buzzer toca.  
   - No LCD, aparece a tela de **ANOMALIA** indicando qual parâmetro saiu da faixa.  
   - O evento é gravado na EEPROM com data/hora e valores dos sensores.  

4. **Consulta de Logs**  
   - Conecte o Arduino ao computador.  
   - Abra o **Monitor Serial** (9600 baud).  
   - Os logs armazenados na EEPROM serão exibidos em formato tabular.  

---

## 📂 Estrutura do Código
- `setup()` → inicializa sensores, RTC, LCD, EEPROM e mostra splash screen.  
- `loop()` → lê sensores, verifica anomalias, gerencia telas e salva logs.  
- `showScreen()` → exibe a tela correspondente no LCD.  
- `get_log()` → imprime os registros da EEPROM no Serial Monitor.  
- `getNextAddress()` → controla o ponteiro de gravação da EEPROM.  
- `showSplash()` → mostra animação de inicialização.  

---

## 🚀 Melhorias Futuras
- Implementar **debounce** nos botões para evitar necessidade de segurar.  
- Exibir logs de anomalia diretamente no LCD.  
- Exportar registros para **cartão SD** ou envio via **Serial/Bluetooth/Wi-Fi**.  
- Interface de configuração para ajuste dinâmico dos limites de alerta.  
- Implementar um case para o data logger.

---
