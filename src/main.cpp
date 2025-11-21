/*
 * ESP32 CiufCiuf - Controller Trenino Natalizio
 *
 * Features:
 * - Controllo motore tramite relè
 * - Riproduzione audio tramite MAX98357A
 * - Web app per controllo remoto via WiFi
 * - Controllo volume software
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "config.h"

// ============================================
// VARIABILI GLOBALI
// ============================================
AsyncWebServer server(80);
bool motorRunning = false;
bool audioPlaying = false;
int currentVolume = DEFAULT_VOLUME;

// ============================================
// GESTIONE MOTORE
// ============================================
void motorStart()
{
    digitalWrite(RELAY_PIN, HIGH);
    motorRunning = true;
    Serial.println("[MOTOR] Started");
}

void motorStop()
{
    digitalWrite(RELAY_PIN, LOW);
    motorRunning = false;
    Serial.println("[MOTOR] Stopped");
}

// ============================================
// GESTIONE AUDIO MP3
// ============================================
AudioGeneratorMP3 *mp3 = NULL;
AudioFileSourceSPIFFS *file = NULL;
AudioOutputI2S *out = NULL;

// Inizializza il sistema audio
void setupAudio()
{
    // Inizializza output I2S
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetGain((float)currentVolume / 100.0);

    Serial.println("[AUDIO] I2S Output initialized");
}

void audioStart()
{
    if (!audioPlaying)
    {
        // Verifica che il file esista
        if (!SPIFFS.exists("/music.mp3"))
        {
            Serial.println("[AUDIO] Error: /music.mp3 not found in SPIFFS!");
            Serial.println("[AUDIO] Please upload your MP3 file to the data/ folder");
            return;
        }

        file = new AudioFileSourceSPIFFS("/music.mp3");
        mp3 = new AudioGeneratorMP3();

        if (mp3->begin(file, out))
        {
            audioPlaying = true;
            Serial.println("[AUDIO] Playing music.mp3");
        }
        else
        {
            Serial.println("[AUDIO] Error starting MP3 playback");
            delete mp3;
            delete file;
            mp3 = NULL;
            file = NULL;
        }
    }
}

void audioStop()
{
    if (audioPlaying && mp3)
    {
        mp3->stop();
        delete mp3;
        delete file;
        mp3 = NULL;
        file = NULL;
        audioPlaying = false;
        Serial.println("[AUDIO] Stopped");
    }
}

void audioLoop()
{
    // Continua a riprodurre l'MP3
    if (audioPlaying && mp3)
    {
        if (mp3->isRunning())
        {
            if (!mp3->loop())
            {
                // Fine del file, riavvia dall'inizio (loop)
                mp3->stop();
                delete mp3;
                delete file;

                file = new AudioFileSourceSPIFFS("/music.mp3");
                mp3 = new AudioGeneratorMP3();
                mp3->begin(file, out);
            }
        }
        else
        {
            audioPlaying = false;
        }
    }
}

void setVolume(int vol)
{
    currentVolume = constrain(vol, 0, 100);
    if (out)
    {
        out->SetGain((float)currentVolume / 100.0);
    }
    Serial.printf("[AUDIO] Volume set to: %d%%\n", currentVolume);
}

// ============================================
// PAGINA WEB HTML
// ============================================
const char *HTML_PAGE = R"rawliteral(
<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CiufCiuf Controller</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 400px;
            width: 100%;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 10px;
            font-size: 28px;
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }
        .section {
            margin-bottom: 30px;
        }
        .section-title {
            color: #555;
            font-size: 14px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 15px;
        }
        .button-group {
            display: flex;
            gap: 10px;
        }
        button {
            flex: 1;
            padding: 15px;
            font-size: 16px;
            font-weight: 600;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s;
            color: white;
        }
        .btn-start {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .btn-start:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        .btn-stop {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
        }
        .btn-stop:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(245, 87, 108, 0.4);
        }
        .btn-start:active, .btn-stop:active {
            transform: translateY(0);
        }
        .volume-control {
            margin-top: 15px;
        }
        .volume-slider {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: #ddd;
            outline: none;
            -webkit-appearance: none;
        }
        .volume-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #667eea;
            cursor: pointer;
        }
        .volume-slider::-moz-range-thumb {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #667eea;
            cursor: pointer;
            border: none;
        }
        .volume-display {
            text-align: center;
            margin-top: 10px;
            color: #666;
            font-size: 14px;
        }
        .status {
            background: #f8f9fa;
            border-radius: 10px;
            padding: 15px;
            margin-top: 20px;
        }
        .status-item {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
            font-size: 14px;
        }
        .status-item:last-child {
            margin-bottom: 0;
        }
        .status-label {
            color: #666;
        }
        .status-value {
            font-weight: 600;
            color: #333;
        }
        .status-on {
            color: #28a745;
        }
        .status-off {
            color: #dc3545;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>CiufCiuf Controller</h1>
        <div class="subtitle">Controllo Trenino Natalizio</div>

        <div class="section">
            <div class="section-title">Motore</div>
            <div class="button-group">
                <button class="btn-start" onclick="motorStart()">Parti</button>
                <button class="btn-stop" onclick="motorStop()">Ferma</button>
            </div>
        </div>

        <div class="section">
            <div class="section-title">Musica</div>
            <div class="button-group">
                <button class="btn-start" onclick="audioStart()">Play</button>
                <button class="btn-stop" onclick="audioStop()">Stop</button>
            </div>

            <div class="volume-control">
                <input type="range" min="0" max="100" value="50"
                       class="volume-slider" id="volumeSlider"
                       oninput="updateVolume(this.value)">
                <div class="volume-display">
                    Volume: <span id="volumeValue">50</span>%
                </div>
            </div>
        </div>

        <div class="status">
            <div class="status-item">
                <span class="status-label">Motore:</span>
                <span class="status-value" id="motorStatus">OFF</span>
            </div>
            <div class="status-item">
                <span class="status-label">Audio:</span>
                <span class="status-value" id="audioStatus">OFF</span>
            </div>
        </div>
    </div>

    <script>
        // Funzioni per controllare il motore
        function motorStart() {
            fetch('/motor/start')
                .then(response => response.json())
                .then(data => {
                    updateStatus();
                    console.log('Motor started');
                })
                .catch(error => console.error('Error:', error));
        }

        function motorStop() {
            fetch('/motor/stop')
                .then(response => response.json())
                .then(data => {
                    updateStatus();
                    console.log('Motor stopped');
                })
                .catch(error => console.error('Error:', error));
        }

        // Funzioni per controllare l'audio
        function audioStart() {
            fetch('/audio/start')
                .then(response => response.json())
                .then(data => {
                    updateStatus();
                    console.log('Audio started');
                })
                .catch(error => console.error('Error:', error));
        }

        function audioStop() {
            fetch('/audio/stop')
                .then(response => response.json())
                .then(data => {
                    updateStatus();
                    console.log('Audio stopped');
                })
                .catch(error => console.error('Error:', error));
        }

        // Funzione per aggiornare il volume
        function updateVolume(value) {
            document.getElementById('volumeValue').textContent = value;
            fetch('/audio/volume?value=' + value)
                .then(response => response.json())
                .then(data => console.log('Volume updated to ' + value + '%'))
                .catch(error => console.error('Error:', error));
        }

        // Aggiorna lo stato visualizzato
        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    const motorStatus = document.getElementById('motorStatus');
                    const audioStatus = document.getElementById('audioStatus');

                    motorStatus.textContent = data.motor ? 'ON' : 'OFF';
                    motorStatus.className = data.motor ? 'status-value status-on' : 'status-value status-off';

                    audioStatus.textContent = data.audio ? 'ON' : 'OFF';
                    audioStatus.className = data.audio ? 'status-value status-on' : 'status-value status-off';

                    document.getElementById('volumeSlider').value = data.volume;
                    document.getElementById('volumeValue').textContent = data.volume;
                })
                .catch(error => console.error('Error:', error));
        }

        // Aggiorna lo stato ogni 2 secondi
        setInterval(updateStatus, 2000);

        // Aggiorna lo stato al caricamento
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

// ============================================
// CONFIGURAZIONE API REST
// ============================================
void setupWebServer()
{
    // Pagina principale
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", HTML_PAGE); });

    // API - Start motore
    server.on("/motor/start", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    motorStart();
    request->send(200, "application/json", "{\"status\":\"ok\",\"motor\":true}"); });

    // API - Stop motore
    server.on("/motor/stop", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    motorStop();
    request->send(200, "application/json", "{\"status\":\"ok\",\"motor\":false}"); });

    // API - Start audio
    server.on("/audio/start", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    audioStart();
    request->send(200, "application/json", "{\"status\":\"ok\",\"audio\":true}"); });

    // API - Stop audio
    server.on("/audio/stop", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    audioStop();
    request->send(200, "application/json", "{\"status\":\"ok\",\"audio\":false}"); });

    // API - Set volume
    server.on("/audio/volume", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    if (request->hasParam("value")) {
      int vol = request->getParam("value")->value().toInt();
      setVolume(vol);
      request->send(200, "application/json", "{\"status\":\"ok\",\"volume\":" + String(currentVolume) + "}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
    } });

    // API - Get status
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    String json = "{\"motor\":" + String(motorRunning ? "true" : "false") +
                  ",\"audio\":" + String(audioPlaying ? "true" : "false") +
                  ",\"volume\":" + String(currentVolume) + "}";
    request->send(200, "application/json", json); });

    server.begin();
}

// ============================================
// SETUP
// ============================================
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("========================================");
    Serial.println("  ESP32 CiufCiuf - Trenino Natalizio");
    Serial.println("========================================");

    // Configura pin relè
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Motore spento all'avvio
    Serial.println("[INIT] Relay configured on pin " + String(RELAY_PIN));

    // Monta SPIFFS per accedere ai file MP3
    if (!SPIFFS.begin(true))
    {
        Serial.println("[SPIFFS] Mount failed!");
        return;
    }
    Serial.println("[SPIFFS] Mounted successfully");

    // Lista i file presenti in SPIFFS
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    Serial.println("[SPIFFS] Files:");
    while (file)
    {
        Serial.print("  - ");
        Serial.print(file.name());
        Serial.print(" (");
        Serial.print(file.size());
        Serial.println(" bytes)");
        file = root.openNextFile();
    }

    // Inizializza sistema audio
    setupAudio();

    // Connessione WiFi
    Serial.println();
    Serial.print("[WIFI] Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.println("[WIFI] Connected!");
        Serial.print("[WIFI] IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.println();
        Serial.println("========================================");
        Serial.println("  Apri il browser e vai su:");
        Serial.print("  http://");
        Serial.println(WiFi.localIP());
        Serial.println("========================================");
    }
    else
    {
        Serial.println();
        Serial.println("[WIFI] Connection failed!");
        Serial.println("[WIFI] Please check credentials in config.h");
    }

    // Avvia web server
    setupWebServer();
    Serial.println("[WEB] Server started");

    Serial.println();
    Serial.println("[INIT] System ready!");
    Serial.println();
}

// ============================================
// LOOP
// ============================================
void loop()
{
    // Gestisce la riproduzione MP3
    audioLoop();

    // Il web server gira in background
    delay(1);
}