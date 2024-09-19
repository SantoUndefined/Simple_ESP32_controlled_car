// -------------------------ESP 32 code---------------
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ToyCarAP";  // Name for the access point
const char* password = "toycar123";  // Password for the access point

WebServer server(80);

// Motor control pins
const int leftMotorPin1 = 26;
const int leftMotorPin2 = 27;
const int rightMotorPin1 = 32;
const int rightMotorPin2 = 33;

// Sprayer pin
const int sprayerPin = 25;

void setup() {
  Serial.begin(115200);

  // Set motor control pins as outputs
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);
  pinMode(sprayerPin, OUTPUT);

  // Set up the ESP32 as an access point
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Define server routes
  server.on("/", handleRoot);
  server.on("/control", HTTP_POST, handleControl);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Toy Car Control</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body {
          font-family: Arial, sans-serif;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
        }
        .control-panel {
          display: grid;
          grid-template-columns: repeat(3, 1fr);
          gap: 10px;
          width: 300px;
        }
        .control-button {
          width: 100%;
          height: 100px;
          font-size: 18px;
          border: none;
          border-radius: 10px;
          background-color: #4CAF50;
          color: white;
          cursor: pointer;
          user-select: none;
          touch-action: manipulation;
        }
        .control-button:active {
          background-color: #45a049;
        }
        #forwardBtn { grid-column: 2; }
        #leftBtn { grid-column: 1; grid-row: 2; }
        #sprayerToggle { grid-column: 2; grid-row: 2; }
        #rightBtn { grid-column: 3; grid-row: 2; }
        #backwardBtn { grid-column: 2; grid-row: 3; }
        .switch {
          position: relative;
          display: inline-block;
          width: 60px;
          height: 34px;
          margin: auto;
        }
        .switch input {
          opacity: 0;
          width: 0;
          height: 0;
        }
        .slider {
          position: absolute;
          cursor: pointer;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
          background-color: #ccc;
          transition: .4s;
          border-radius: 34px;
        }
        .slider:before {
          position: absolute;
          content: "";
          height: 26px;
          width: 26px;
          left: 4px;
          bottom: 4px;
          background-color: white;
          transition: .4s;
          border-radius: 50%;
        }
        input:checked + .slider {
          background-color: #2196F3;
        }
        input:checked + .slider:before {
          transform: translateX(26px);
        }
        .label {
          text-align: center;
          font-size: 14px;
          margin-top: 5px;
        }
      </style>
    </head>
    <body>
      <div class="control-panel">
        <button id="forwardBtn" class="control-button">Forward</button>
        <button id="leftBtn" class="control-button">Left</button>
        <div id="sprayerToggle">
          <label class="switch">
            <input type="checkbox" id="sprayerCheckbox">
            <span class="slider"></span>
          </label>
          <div class="label">Sprayer</div>
        </div>
        <button id="rightBtn" class="control-button">Right</button>
        <button id="backwardBtn" class="control-button">Backward</button>
      </div>
      <script>
        const buttons = {
          forward: document.getElementById('forwardBtn'),
          backward: document.getElementById('backwardBtn'),
          left: document.getElementById('leftBtn'),
          right: document.getElementById('rightBtn')
        };
        const sprayerToggle = document.getElementById('sprayerCheckbox');
        let currentDirection = '';
        let isButtonPressed = false;

        function sendControl(direction, sprayer) {
          const data = `direction=${direction}&sprayer=${sprayer}`;
          fetch('/control', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: data,
          });
        }

        function handleButtonPress(direction) {
          currentDirection = direction;
          isButtonPressed = true;
        }

        function handleButtonRelease() {
          currentDirection = '';
          isButtonPressed = false;
          sendControl('', sprayerToggle.checked);
        }

        function continuouslyCheckButtonPress() {
          if (isButtonPressed) {
            sendControl(currentDirection, sprayerToggle.checked);
          }
        }

        // Set up event listeners for buttons
        for (const [direction, button] of Object.entries(buttons)) {
          button.addEventListener('mousedown', () => handleButtonPress(direction));
          button.addEventListener('mouseup', handleButtonRelease);
          button.addEventListener('mouseleave', handleButtonRelease);
          button.addEventListener('touchstart', (e) => {
            e.preventDefault();
            handleButtonPress(direction);
          });
          button.addEventListener('touchend', (e) => {
            e.preventDefault();
            handleButtonRelease();
          });
        }

        sprayerToggle.addEventListener('change', () => {
          sendControl(currentDirection, sprayerToggle.checked);
        });

        // Check button state every 100ms
        setInterval(continuouslyCheckButtonPress, 100);
      </script>
    </body>
    </html>
  )";
  server.send(200, "text/html", html);
}

void handleControl() {
  String direction = server.arg("direction");
  bool sprayerState = (server.arg("sprayer") == "true");

  // Print current status to Serial Monitor
  Serial.print("Direction: ");
  Serial.print(direction);
  Serial.print(", Sprayer: ");
  Serial.println(sprayerState ? "ON" : "OFF");

  // Control motors based on direction
  if (direction == "forward") {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);
  } else if (direction == "backward") {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);
  } else if (direction == "left") {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);
  } else if (direction == "right") {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);
  } else {
    // Stop all motors if no direction
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, LOW);
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, LOW);
  }

  // Control sprayer
  digitalWrite(sprayerPin, sprayerState ? HIGH : LOW);

  server.send(200, "text/plain", "OK");
}
