#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32-CAR";
const char* password = "Papilagustativa";

WebServer server(80);

enum Mode {REPOSO, LINEFOLLOWER, MANUAL};
Mode currentMode = REPOSO;

const int motor1Pin1 = 18;
const int motor1Pin2 = 19;
const int motor2Pin1 = 4;
const int motor2Pin2 = 16;

const int motor1Speed = 245;
const int motor2Speed = 255;

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>

<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>ESP32 Car Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background-color: #f0f0f0;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        h1,
        h2 {
            color: #333;
        }

        .button {
            display: inline-block;
            padding: 15px 25px;
            font-size: 24px;
            cursor: pointer;
            text-align: center;
            text-decoration: none;
            outline: none;
            color: #fff;
            background-color: #007BFF;
            border: none;
            border-radius: 15px;
            box-shadow: 0 9px #999;
            margin: 5px;
            transition: background-color 0.3s ease, transform 0.1s ease;
        }

        .button-large {
            font-size: 24px;
            padding: 20px 30px;
        }

        .button:hover {
            background-color: #0056b3;
        }

        .button:active {
            background-color: #004494;
            box-shadow: 0 5px #666;
            transform: translateY(4px);
        }

        .button-disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }

        .info-message {
            display: none;
            font-size: 24px;
            color: #333;
        }

        #manual-controls {
            margin-top: 30px;
            display: flex;
            justify-content: space-between;
            align-items: flex-start;
            width: 90%;
        }

        #manual-buttons {
            display: none;
            justify-content: center;
            align-items: center;
            flex-direction: column;
        }

        .button-control {
            width: 80px;
            height: 80px;
            margin: 5px;
            font-size: 20px;
            cursor: pointer;
            outline: none;
            transition: background-color 0.3s ease, transform 0.1s ease;
            background-color: #ccc;
            box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.3);
            border: none;
            border-radius: 5px;
            padding: 5px;
        }

        .button-control:hover {
            background-color: #ddd;
        }

        .button-control:active {
            background-color: #bbb;
            transform: translateY(4px);
        }

        .button-inner {
            background-color: #fff;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            margin: auto;
        }

        .modo-reposo {
            background-color: red !important;
        }

        .modo-manual {
            background-color: green !important;
        }

        .modo-activo {
            box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.3);
        }

        #control-container {
            display: grid;
            grid-template-areas:
                "up-left up up-right"
                "left stop right"
                "down-left down down-right";
            gap: 10px;
        }

        #forward-btn {
            grid-area: up;
        }

        #left-btn {
            grid-area: left;
        }

        #stop-btn {
            grid-area: stop;
        }

        #right-btn {
            grid-area: right;
        }

        #backward-btn {
            grid-area: down;
        }

        #up-left-btn {
            grid-area: up-left;
        }

        #up-right-btn {
            grid-area: up-right;
        }

        #down-left-btn {
            grid-area: down-left;
        }

        #down-right-btn {
            grid-area: down-right;
        }

        #serial-monitor {
            width: 300px;
            height: 300px;
            border: 1px solid #333;
            border-radius: 5px;
            background-color: #fff;
            padding: 10px;
            overflow-y: scroll;
            text-align: left;
        }

        #mode-buttons-container {
            display: flex;
            justify-content: center;
            align-items: center;
        }
    </style>
</head>

<body>
    <h1>ESP32 Car Control</h1>
    <h2>Controla tu auto desde aquí</h2>
    <div id="mode-buttons-container">
        <button class='button button-large' id="reposo-btn" onclick='sendCommand("REPOSO")'>REPOSO</button>
        <button class='button button-large' id="manual-btn" onclick='sendCommand("MANUAL")'>MANUAL</button>
        <button class='button button-large' id="start-btn" onclick='sendCommand("START")'>INICIAR</button>
    </div>
    <div id="manual-controls">
        <div id="serial-monitor">
            <h2>Serial Monitor</h2>
            <div id="serial-content"></div>
        </div>
        <div id="manual-buttons">
            <div id="control-container">
                <button class="button button-control" id="forward-btn" onclick='sendControlCommand("FORWARD")'>
                    <div class="button-inner"></div>
                </button>
                <button class="button button-control" id="left-btn" onclick='sendControlCommand("LEFT")'>
                    <div class="button-inner"></div>
                </button>
                <button class="button button-control" id="stop-btn" onclick='sendControlCommand("STOP")'>
                    <div class="button-inner"></div>
                </button>
                <button class="button button-control" id="right-btn" onclick='sendControlCommand("RIGHT")'>
                    <div class="button-inner"></div>
                </button>
                <button class="button button-control" id="backward-btn" onclick='sendControlCommand("BACKWARD")'>
                    <div class="button-inner"></div>
                </button>
                <button class="button button-control" id="up-left-btn" onclick='sendControlCommand("UP-LEFT")'>
                    GIRO 45
                </button>
                <button class="button button-control" id="up-right-btn" onclick='sendControlCommand("UP-RIGHT")'>
                    GIRO 45
                </button>
                <button class="button button-control" id="down-left-btn" onclick='sendControlCommand("DOWN-LEFT")'>
                    GIRO 45
                </button>
                <button class="button button-control" id="down-right-btn" onclick='sendControlCommand("DOWN-RIGHT")'>
                    GIRO 45
                </button>
            </div>
        </div>
    </div>
    <div id='info-message-container'>
        <p class='info-message' id='reposo-message'>El auto no se puede mover en modo REPOSO. Por favor, cambia a otro
            modo.</p>
    </div>
    <script>
        let manualMode = false;
        let currentMode = "reposo";

        function sendCommand(command) {
            if (command === "MANUAL") {
                manualMode = true;
                enableManualButtons();
                currentMode = "manual";
                logCommand(`Ingresado al modo: ${command}`);
            } else {
                manualMode = false;
                disableManualButtons();
                currentMode = command.toLowerCase();
                logCommand(`Ingresado al modo: ${command}`);
            }
            fetch(`/command?cmd=${command}`)
                .then(response => response.text())
                .then(data => console.log(data))
                .catch(error => console.error('Error:', error));
            updateButtonStyles();
        }

        function sendControlCommand(command) {
            fetch(`/command?cmd=${command}`)
                .then(response => response.text())
                .then(data => console.log(data))
                .catch(error => console.error('Error:', error));
            logCommand(`Comando enviado: ${command}`);
        }

        function enableManualButtons() {
            document.getElementById('manual-buttons').style.display = 'flex';
            document.querySelectorAll('.info-message').forEach(message => message.style.display = 'none');
        }

        function disableManualButtons() {
            document.getElementById('manual-buttons').style.display = 'none';
            document.querySelectorAll('.info-message').forEach(message => message.style.display = 'none');
            if (currentMode === 'reposo') {
                document.getElementById('reposo-message').style.display = 'block';
            }
        }

        function updateButtonStyles() {
            document.getElementById('reposo-btn').classList.remove('modo-reposo', 'modo-activo');
            document.getElementById('manual-btn').classList.remove('modo-manual', 'modo-activo');
            if (currentMode === 'reposo') {
                document.getElementById('reposo-btn').classList.add('modo-reposo', 'modo-activo');
            } else if (currentMode === 'manual') {
                document.getElementById('manual-btn').classList.add('modo-manual', 'modo-activo');
            }
        }

        function logCommand(message) {
            const serialContent = document.getElementById('serial-content');
            const newMessage = document.createElement('p');
            newMessage.textContent = message;
            serialContent.appendChild(newMessage);
            serialContent.scrollTop = serialContent.scrollHeight;
        }

        document.querySelectorAll('.button-large').forEach(button => button.addEventListener('click', () => {
            document.querySelectorAll('.info-message').forEach(message => message.style.display = 'none');
        }));
    </script>
</body>

</html>

)rawliteral";

// Declaraciones de funciones
void handleRoot();
void handleCommand();
void processCommand(String command);
void stopAllActivities();
void startManualMode();
void processManualCommand(String command);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMotors();
void turnUpLeft();
void turnUpRight();
void turnDownLeft();
void turnDownRight();
void executePattern();

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.begin();
  Serial.println("HTTP Server started, waiting for connections...");

  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcSetup(3, 5000, 8);

  ledcAttachPin(motor1Pin1, 0);
  ledcAttachPin(motor1Pin2, 1);
  ledcAttachPin(motor2Pin1, 2);
  ledcAttachPin(motor2Pin2, 3);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);

  Serial.println("Setup complete, waiting for commands...");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleCommand() {
  String command = server.arg("cmd");
  Serial.println("Received command: " + command);
  processCommand(command);
  server.send(200, "text/plain", "Command received: " + command);
}

void processCommand(String command) {
  if (command == "REPOSO") {
    currentMode = REPOSO;
    stopAllActivities();
  } else if (command == "MANUAL") {
    currentMode = MANUAL;
    startManualMode();
  } else if (command == "START") {
    executePattern();
  } else if (currentMode == MANUAL) {
    processManualCommand(command);
  }
}

void stopAllActivities() {
  Serial.println("Stopping all activities...");
  stopMotors();
}

void startManualMode() {
  Serial.println("Starting manual mode...");
}

void processManualCommand(String command) {
  if (command == "FORWARD") {
    moveForward();
  } else if (command == "BACKWARD") {
    moveBackward();
  } else if (command == "LEFT") {
    turnLeft();
  } else if (command == "RIGHT") {
    turnRight();
  } else if (command == "STOP") {
    stopMotors();
  } else if (command == "UP-LEFT") {
    turnUpLeft();
  } else if (command == "UP-RIGHT") {
    turnUpRight();
  } else if (command == "DOWN-LEFT") {
    turnDownLeft();
  } else if (command == "DOWN-RIGHT") {
    turnDownRight();
  } else {
    Serial.println("Unknown manual command");
  }
}

void executePattern() {
  Serial.println("Starting pattern sequence...");
  moveForward();
  delay(1700);
  turnRight();
  delay(480);
  moveForward();
  delay(1700);
  turnRight();
  delay(490);
  moveForward();
  delay(1200);
  turnRight();
  delay(500);
  moveForward(
  turnRight();
  delay(500);
  );
  delay(1700);
  stopMotors();
  Serial.println("Pattern sequence completed.");
}

void moveForward() {
  Serial.println("ADELANTE");
  ledcWrite(0, motor1Speed);
  ledcWrite(1, 0);
  ledcWrite(2, motor2Speed);
  ledcWrite(3, 0);
}

void moveBackward() {
  Serial.println("ATRAS");
  ledcWrite(0, 0);
  ledcWrite(1, motor1Speed);
  ledcWrite(2, 0);
  ledcWrite(3, motor2Speed);
}

void turnLeft() {
  Serial.println("IZQUIERDA");
  ledcWrite(0, motor1Speed);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
}

void turnRight() {
  Serial.println("DERECHA");
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, motor2Speed);
  ledcWrite(3, 0);
}

void turnUpLeft() {
  Serial.println("GIRO 45 IZQUIERDA ARRIBA");
  ledcWrite(0, motor1Speed);
  ledcWrite(1, 0);
  ledcWrite(2, motor2Speed / 2);
  ledcWrite(3, 0);
}

void turnUpRight() {
  Serial.println("GIRO 45 DERECHA ARRIBA");
  ledcWrite(0, motor1Speed / 2);
  ledcWrite(1, 0);
  ledcWrite(2, motor2Speed);
  ledcWrite(3, 0);
}

void turnDownLeft() {
  Serial.println("GIRO 45 IZQUIERDA ABAJO");
  ledcWrite(0, 0);
  ledcWrite(1, motor1Speed);
  ledcWrite(2, 0);
  ledcWrite(3, motor2Speed / 2);
}

void turnDownRight() {
  Serial.println("GIRO 45 DERECHA ABAJO");
  ledcWrite(0, 0);
  ledcWrite(1, motor1Speed / 2);
  ledcWrite(2, 0);
  ledcWrite(3, motor2Speed);
}

void stopMotors() {
  Serial.println("STOP");
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
}
