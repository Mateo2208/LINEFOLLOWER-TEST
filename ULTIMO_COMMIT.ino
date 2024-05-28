#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "ESP32-CAR";
const char* password = "Papilagustativa";

WebServer server(80);

enum Mode {REPOSO, LINEFOLLOWER, MANUAL, MAPPING};
Mode currentMode = REPOSO;

const int motor1Pin1 = 18;
const int motor1Pin2 = 19;
const int motor2Pin1 = 4;
const int motor2Pin2 = 16;

const int motor1Speed = 255;
const int motor2Speed = 255;

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
    ...
</head>
<body>
    ...
</body>
</html>
)rawliteral";

// Function declarations
void handleRoot();
void handleCommand();
void handleExecuteMapping();
void processCommand(String command);
void stopAllActivities();
void startManualMode();
void startMappingMode();
void processManualCommand(String command);
void executeMappingSequence(String sequence);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMotors();
void turnUpLeft();
void turnUpRight();
void turnDownLeft();
void turnDownRight();

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.on("/executeMapping", handleExecuteMapping);
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

void handleExecuteMapping() {
  String sequence = server.arg("sequence");
  Serial.println("Received mapping sequence: " + sequence);
  executeMappingSequence(sequence);
  server.send(200, "text/plain", "Mapping sequence received: " + sequence);
}

void processCommand(String command) {
  if (command == "REPOSO") {
    currentMode = REPOSO;
    stopAllActivities();
  } else if (command == "MANUAL") {
    currentMode = MANUAL;
    startManualMode();
  } else if (command == "MAPPING") {
    currentMode = MAPPING;
    startMappingMode();
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

void startMappingMode() {
  Serial.println("Starting mapping mode...");
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

void executeMappingSequence(String sequence) {
  StaticJsonDocument<500> doc;
  deserializeJson(doc, sequence);

  for (JsonObject movement : doc.as<JsonArray>()) {
    String command = movement["command"];
    int duration = movement["duration"];
    processManualCommand(command);
    delay(duration * 1000);
    stopMotors();
  }
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
  ledcWrite(0, motor1Speed / 2);
  ledcWrite(1, 0);
  ledcWrite(2, motor2Speed);
  ledcWrite(3, 0);
}

void turnUpRight() {
  Serial.println("GIRO 45 DERECHA ARRIBA");
  ledcWrite(0, motor1Speed);
  ledcWrite(1, 0);
  ledcWrite(2, motor2Speed / 2);
  ledcWrite(3, 0);
}

void turnDownLeft() {
  Serial.println("GIRO 45 IZQUIERDA ABAJO");
  ledcWrite(0, 0);
  ledcWrite(1, motor1Speed / 2);
  ledcWrite(2, 0);
  ledcWrite(3, motor2Speed);
}

void turnDownRight() {
  Serial.println("GIRO 45 DERECHA ABAJO");
  ledcWrite(0, 0);
  ledcWrite(1, motor1Speed);
  ledcWrite(2, 0);
  ledcWrite(3, motor2Speed / 2);
}

void stopMotors() {
  Serial.println("STOP");
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
}
