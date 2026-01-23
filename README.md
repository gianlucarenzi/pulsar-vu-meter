# Pulsar VU

Un progetto flessibile per creare un **Pulsar VU** (un VU-meter a 12 LED), capace di funzionare sia come indicatore di carico della CPU di un computer, sia come VU-meter per un segnale audio mono. Il progetto è basato sull'ecosistema **PlatformIO** e supporta diverse schede e configurazioni.

![Placeholder](https://via.placeholder.com/600x300.png?text=Immagina+qui+il+tuo+Pulsar+VU+in+azione!)

---

## 🌟 Caratteristiche Principali

-   **Doppia Modalità**:
    1.  **Modalità CPU**: Visualizza il carico complessivo della CPU di un computer (Linux, Windows, macOS) tramite uno script Python (`pulsar_host.py`) che comunica via seriale.
    2.  **Modalità Audio**: Analizza un segnale audio mono da un pin analogico e ne visualizza il volume.
-   **Multi-Piattaforma (Hardware)**: Supporta nativamente sia **Arduino Nano** che **ESP32 WROVER**, con configurazioni pronte per entrambi.
-   **Multi-Striscia**: In modalità audio, può pilotare fino a 3 strisce LED identiche in parallelo per un effetto più luminoso o distribuito.
-   **Display a Segmenti di Colore o Colore Fisso**: Puoi scegliere se l'indicatore si riempie progressivamente con colori diversi (verde, giallo, arancione, rosso) oppure con un colore fisso (verde, giallo, arancione, rosso, o personalizzato RGB) tramite configurazione nel codice (#define LED_FIXED_COLOR).
-   **Configurazione Semplice**: Gestito interamente tramite **PlatformIO** in Visual Studio Code, con ambienti pre-configurati per ogni combinazione di scheda e modalità.

---

## 🔦 Configurazione Colore LED

### Firmware

Per ottenere una barra di colore fisso (verde, giallo, arancione, rosso o personalizzato RGB) durante il funzionamento normale:

1. Apri il file `platformio.ini`.
2. Nella sezione `[env:<tuo_ambiente>]`, aggiungi tra le `build_flags` la definizione desiderata:

```ini
-D LED_FIXED_COLOR=LED_COLOR_GREEN    ; tutti verdi
-D LED_FIXED_COLOR=LED_COLOR_YELLOW   ; tutti gialli
-D LED_FIXED_COLOR=LED_COLOR_ORANGE   ; tutti arancioni
-D LED_FIXED_COLOR=LED_COLOR_RED      ; tutti rossi
-D LED_FIXED_COLOR=LED_COLOR_CUSTOM   ; colore RGB custom
-D LED_CUSTOM_R=128                   ; rosso (0-255)
-D LED_CUSTOM_G=0                     ; verde (0-255)
-D LED_CUSTOM_B=128                   ; blu (0-255)
```
Esempio per una barra viola:
```ini
-D LED_FIXED_COLOR=LED_COLOR_CUSTOM
-D LED_CUSTOM_R=128
-D LED_CUSTOM_G=0
-D LED_CUSTOM_B=128
```
3. Compila e carica il firmware sull'hardware.

Se non definisci nulla, il comportamento sarà "a segmenti" (verde/giallo/arancione/rosso progressivi).

### Qt Simulator

Puoi scegliere il colore dei LED da CLI:
- `--green` (tutti verdi)
- `--yellow` (tutti gialli)
- `--orange` (tutti arancioni)
- `--red` (tutti rossi)
- `--custom R G B` (valori RGB personalizzati)
Se nessuna opzione è specificata, il comportamento è a segmenti.

## 🛠️ Hardware Necessario

-   **Microcontrollore**: Uno tra:
    -   Arduino Nano
    -   ESP32 WROVER Kit (o simile)
-   **Striscia LED**: Una o più strisce di LED indirizzabili **WS2812B** da 12 LED.
-   **Cavi** per i collegamenti.
-   **(Solo per Modalità Audio)**: Componenti per un semplice **circuito di biasing** per il segnale audio (es. 2 resistori da 10kΩ, 1 condensatore da 10µF).

---

## ⚙️ Software e Setup

1.  **Visual Studio Code**: Installa l'editor.
2.  **Estensione PlatformIO IDE**: Cerca e installa `PlatformIO IDE` dal pannello Estensioni di VS Code.
3.  **Python 3**: Assicurati di avere Python installato sul tuo computer.
4.  **Clona il Repository**: Ottieni i file di questo progetto sul tuo computer.
5.  **Installa le Dipendenze Python**: Apri un terminale nella cartella del progetto ed esegui:
    ```bash
    pip install psutil pyserial
    ```
6.  **Apri il Progetto**: Apri la cartella del progetto in VS Code (`File > Open Folder...`). PlatformIO rileverà automaticamente la configurazione.

---

## 🚀 Come si Usa

### 1. Firmware (Microcontrollore)

Il cuore della configurazione è il file `platformio.ini`. Puoi scegliere cosa caricare semplicemente selezionando l'ambiente desiderato dalla barra di stato di VS Code.

**Ambienti Disponibili:**

| Ambiente            | Scheda       | Modalità | N. Strisce |
| ------------------- | ------------ | -------- | ---------- |
| `nano`              | Arduino Nano | Seriale  | 1          |
| `nano_audio`        | Arduino Nano | Audio    | 3          |
| `esp32_rover`       | ESP32        | Seriale  | 1          |
| `esp32_rover_audio` | ESP32        | Audio    | 3          |

-   **Per caricare**:
    1.  Collega la scheda al PC.
    2.  Scegli l'ambiente desiderato dalla barra di stato in basso.
    3.  Clicca sull'icona della freccia (→) "Upload" nella stessa barra.

### 2. Script Host (Solo Modalità CPU/Seriale)

Per usare il **Pulsar VU** come indicatore di carico CPU:

1.  **Modifica lo script**: Apri il file `pulsar_host.py` e imposta la porta seriale corretta per il tuo sistema operativo.
    ```python
    # pulsar_host.py

    # ...
    # Modifica questo valore in base al tuo sistema operativo:
    SERIAL_PORT = '/dev/ttyACM0'  # Esempio per Linux
    # SERIAL_PORT = 'COM3'          # Esempio per Windows
    # ...
    ```
2.  **Esegui lo script**: Dopo aver caricato il firmware in modalità `nano` o `esp32_rover`, lancia lo script da un terminale:
    ```bash
    python3 pulsar_host.py [PORTA_SERIAL] [reverse]
    ```
    - `PORTA_SERIAL` (opzionale): la porta seriale a cui è collegato l'Arduino (es. `/dev/ttyACM0` su Linux, `COM3` su Windows). Se omesso, userà `COM1` su Windows o `./vCOM` su altri sistemi.
    - `reverse` (opzionale): se specificato, abilita la modalità REVERSE (visualizzazione invertita). Se omesso, la modalità normale è attiva.

---

## 🔌 Schemi di Cablaggio

### Arduino Nano

-   **Modalità CPU (1 Striscia)**:
    -   `D6` -> Dati Striscia 1
    -   `5V` / `GND` -> Alimentazione Striscia 1
-   **Modalità Audio (3 Strisce)**:
    -   `A0`  -> Output del circuito di biasing del segnale audio
    -   `D6` -> Dati Striscia 1
    -   `D7` -> Dati Striscia 2
    -   `D8` -> Dati Striscia 3
    -   `5V` / `GND` -> Alimentazione comune per tutte le strisce

### ESP32 WROVER

-   **Modalità CPU (1 Striscia)**:
    -   `GPIO 23` -> Dati Striscia 1
    -   `5V` / `GND` -> Alimentazione Striscia 1
-   **Modalità Audio (3 Strisce)**:
    -   `GPIO 36` -> Output del circuito di biasing del segnale audio
    -   `GPIO 23` -> Dati Striscia 1
    -   `GPIO 22` -> Dati Striscia 2
    -   `GPIO 21` -> Dati Striscia 3
    -   `5V` / `GND` -> Alimentazione comune per tutte le strisce

**Nota sull'alimentazione**: Pilotare 3 strisce LED può richiedere una corrente significativa. Assicurati che la tua fonte di alimentazione (che sia l'USB del PC o un alimentatore esterno) sia adeguata.

---

## 🔧 Dettagli di Configurazione

Il file `platformio.ini` permette di personalizzare facilmente i pin utilizzati senza modificare il codice sorgente. Ad esempio, la configurazione per la modalità audio su Nano definisce tutti i pin necessari:

```ini
[env:nano_audio]
; Ambiente per Nano con input audio analogico (3 strisce)
platform = atmelavr
board = nanoatmega328
build_flags =
    -D INPUT_MODE_ANALOG
    -D ANALOG_INPUT_PIN=A0
    -D PULSAR_VU_DATA_PIN_1=6
    -D PULSAR_VU_DATA_PIN_2=7
    -D PULSAR_VU_DATA_PIN_3=8
```

Questo approccio, basato su "build flags", rende il progetto pulito e facilmente estensibile.
