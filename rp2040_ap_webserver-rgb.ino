#include <SPI.h>
#include <WiFiNINA.h>

// Network SSID and password
char ssid[] = "rp2040";
char pass[] = "12345678";

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  Serial.println("Access Point Web Server");
  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  // wait 10 seconds for connection:
  delay(10000);

  // start the web server on port 80
  server.begin();

  // you're connected now, so print out the status
  printWiFiStatus();
}

void loop() {
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      Serial.println("Device connected to AP");
    }
    else {
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {
    // if you get a client,
    Serial.println("new client");
    String currentLine = "";
    
    while (client.connected()) {
      // This is required for the Arduino Nano RP2040 Connect - otherwise it will loop so fast that SPI will never be served.
      delayMicroseconds(10);
                    
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        if (c == '\n') {
          // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // Create the webserver page 
            webServerPage(client);
            
            // break out of the while loop:
            break;
          }
          else {
            // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {
          // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check if the client request was "/RH", "/RL", "/GH", "/GL", "/BH" or "/BL"
        if (currentLine.endsWith("GET /red-on")) digitalWrite(LEDR, HIGH);
        else if (currentLine.endsWith("GET /red-off")) digitalWrite(LEDR, LOW);
        else if (currentLine.endsWith("GET /green-on")) digitalWrite(LEDG, HIGH);
        else if (currentLine.endsWith("GET /green-off")) digitalWrite(LEDG, LOW);
        else if (currentLine.endsWith("GET /blue-on")) digitalWrite(LEDB, HIGH);
        else if (currentLine.endsWith("GET /blue-off")) digitalWrite(LEDB, LOW);
      }
    }
    
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

/// @brief Print the SSID, the IP Address and webserver URL
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

/// @brief Create the HTML + CSS + Javascript page for the webserver 
void webServerPage(WiFiClient client) {
  client.print("<html>");
  client.print("  <head>");
  client.print("    <style>");
  client.print("      h1 {");
  client.print("        font-family:verdana,garamond,serif;");
  client.print("        color: black;");
  client.print("        font-size: 40px;");
  client.print("        text-align: center;");
  client.print("        font-weight:normal;");
  client.print("      }");
  client.print("      button {");
  client.print("        border: 1;");
  client.print("        border-radius: 20px;");
  client.print("        height: 40px;");
  client.print("        width:80px;");
  client.print("        cursor: pointer;");
  client.print("        margin: 0px 10px 0px 10px;");
  client.print("      }");
  client.print("      .red-off {background-color: #550000;}");
  client.print("      .red-on {background-color: #ff0000;}");
  client.print("      .green-off {background-color: #005500;}");
  client.print("      .green-on {background-color: #00ff00;}");
  client.print("      .blue-off {background-color: #000055;}");
  client.print("      .blue-on { background-color: #0000ff;}");
  client.print("    </style>");
  client.print("  </head>");
  client.print("  <body>");
  client.print("    <p><h1>Arduino Nano RP2040<h1></p>");
  client.print("    <p>Click the buttons below to turn on/off the RGB leds</p>");
  client.print("    <button class=\"red-off\" onclick=\"clickButton('red')\"></button>");
  client.print("    <button class=\"green-off\" onclick=\"clickButton('green')\"></button>");
  client.print("    <button class=\"blue-off\" onclick=\"clickButton('blue')\"></button>");
  client.print("    <script>");
  client.print("      function clickButton(color) {");
  client.print("        let button = document.querySelector('.' + color + '-off');");
  client.print("        button.classList.toggle(color + '-on');");
  client.print("        let xmlHttpRequest = new XMLHttpRequest();");
  client.print("        if(button.classList.contains(color + '-on')) xmlHttpRequest.open(\"GET\", \"/\" + color + \"-on\", true);");
  client.print("        else if(button.classList.contains(color + '-off')) xmlHttpRequest.open(\"GET\", \"/\" + color + \"-off\", true);");
  client.print("        xmlHttpRequest.send();");
  client.print("      }");
  client.print("  </script>");
  client.print("  </body>");
  client.print("</html>");

  // HTTP response ends with another blank line:
  client.println();
}
