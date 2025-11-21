# ESP32 CiufCiuf - Controller Trenino Natalizio

Controller WiFi per trenino natalizio con ESP32, relè motore e riproduzione audio tramite MAX98357A.

[🇬🇧 English Version](README.md)

## Hardware Necessario

- **ESP32 DevKit** - Microcontrollore principale
- **Modulo Relè 5V 1 canale** - Controllo motore
- **MAX98357A amplificatore I2S** - Riproduzione audio
- **Speaker 8Ω 3W** - Uscita audio
- **Alimentazione** - 5V per ESP32 (power bank mini USB o portabatterie)
- **Cavi Dupont** - Collegamenti

## Schema Collegamenti

### Relè Motore
```
Modulo Relè    →    ESP32
-----------         -----
VCC            →    5V (VIN)
GND            →    GND
IN             →    GPIO 2

Modulo Relè    →    Locomotiva
-----------         ----------
COM            →    Filo tastino 1
NO             →    Filo tastino 2
```

### MAX98357A Audio
```
MAX98357A      →    ESP32
---------           -----
VIN            →    5V (VIN)
GND            →    GND
BCLK           →    GPIO 26
LRC (LRCLK)    →    GPIO 25
DIN            →    GPIO 22

MAX98357A      →    Speaker
---------           -------
Speaker+       →    Speaker+ (rosso)
Speaker-       →    Speaker- (nero)
```

## Configurazione

### 1. Configura WiFi
Copia il file di configurazione esempio e inserisci le tue credenziali WiFi:

```bash
cp src/config.h.example src/config.h
```

Poi modifica `src/config.h` e inserisci le tue credenziali WiFi:

```cpp
const char* WIFI_SSID = "IL_TUO_WIFI";
const char* WIFI_PASSWORD = "LA_TUA_PASSWORD";
```

**Nota:** Il file `config.h` è ignorato da git per mantenere private le tue credenziali.

### 2. Compila e Carica
In PlatformIO:
1. Clicca su "Build" (icona segno di spunta in basso)
2. Collega l'ESP32 via USB
3. Clicca su "Upload" (icona freccia destra in basso)

### 3. Apri Serial Monitor
Clicca sull'icona del monitor seriale in basso per vedere l'indirizzo IP dell'ESP32.

### 4. Apri la Web App
Nel browser del tuo telefono/PC (connesso allo stesso WiFi), vai all'indirizzo mostrato nel Serial Monitor:
```
http://192.168.x.x
```

## Funzionalità

### Web App (già implementata)
- **Controllo Motore**: pulsanti Parti/Ferma
- **Controllo Audio**: pulsanti Play/Stop musica
- **Volume**: slider 0-100%
- **Status**: visualizzazione stato in tempo reale

### API REST
Se vuoi controllare il trenino da script o altre app:

- `GET /motor/start` - Avvia motore
- `GET /motor/stop` - Ferma motore
- `GET /audio/start` - Avvia audio
- `GET /audio/stop` - Ferma audio
- `GET /audio/volume?value=50` - Imposta volume (0-100)
- `GET /status` - Ottieni stato corrente (JSON)

## Implementazione Audio

Il firmware attuale include un'interfaccia di base per il controllo audio. Per implementare la riproduzione audio completa:

1. Collega l'hardware MAX98357A come da schema
2. Aggiungi file audio nella memoria flash ESP32 (SPIFFS o LittleFS)
3. Implementa la riproduzione I2S nelle funzioni `audioStart()` e `audioStop()` in `main.cpp`
4. Utilizza librerie come ESP32-audioI2S per decodifica e riproduzione dei file audio

### Configurazione Pin (opzionale)
Se vuoi cambiare i pin, modifica il file `src/config.h`:

```cpp
#define RELAY_PIN 2      // Pin relè motore
#define I2S_BCLK 26      // Pin I2S
#define I2S_LRC  25
#define I2S_DOUT 22
```

## Troubleshooting

### ESP32 non si connette al WiFi
- Verifica le credenziali in `config.h`
- Controlla che il WiFi sia 2.4GHz (ESP32 non supporta 5GHz)
- Controlla il Serial Monitor per messaggi di errore

### Il relè non commuta
- Verifica i collegamenti (VCC, GND, GPIO 2)
- Alcuni moduli relè necessitano di segnale LOW per attivarsi (inverti HIGH/LOW nel codice)

### Errori di compilazione
- Aspetta che PlatformIO scarichi tutte le librerie
- Se persiste, prova: PlatformIO → Clean → Build

## Note

- L'ESP32 deve essere sempre acceso per ricevere comandi WiFi
- Il tastino fisico della locomotiva accende solo il motore (alimentazione diretta)
- Il relè controllato da ESP32 fa da interruttore tra il tastino e il motore
- Tensione locomotiva: 6V (4 batterie AA)
- Alimentazione ESP32: separata (5V USB)
