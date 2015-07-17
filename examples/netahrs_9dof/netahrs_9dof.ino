#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_Simple_AHRS.h>

// Create sensor instances.
Adafruit_LSM303_Accel_Unified accel(30301);
Adafruit_LSM303_Mag_Unified   mag(30302);

// Create simple AHRS algorithm using the above sensors.
Adafruit_Simple_AHRS          ahrs(&accel, &mag);

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

const char SSID[]   = "xxxxxxxx";
const char PASS[]   = "yyyyyyyy";

WiFiServer tcpServer(9999);
WiFiClient tcpClient;

void setup()
{
    Serial.begin(115200);
    Serial.println(F("Adafruit 9 DOF Board AHRS ESP8266 WiFi Example"));
    Serial.println("");

    // Initialize the sensors.
    accel.begin();
    mag.begin();

    // Connect to WiFi access point
    Serial.print("\nConnecting to ");
    Serial.println(SSID);
    int i = 20;
    while ((WiFi.status() != WL_CONNECTED) && i--) {
        Serial.print(".");
        delay(500);
    }
    if (i == 0) {
        Serial.print("Could not connect to ");
        Serial.println(SSID);
        while(1) continue;
    }
    // start TCP server listening on port 9999
    tcpServer.begin();
    tcpServer.setNoDelay(true);

    // Advertise using an easy to remember name instead of IP address
    if (MDNS.begin("esp_ahrs")) {
        Serial.println("\nConnect to esp_ahrs.local:9999 for AHRS data");
    }
    else {
        Serial.println("mDNS fail");
        Serial.print("\nConnect to ");
        Serial.print(WiFi.localIP());
        Serial.println(":9999 for AHRS data");
    }
}

void loop(void)
{
    // check if there are any new clients
    // only 1 client supported at this time.
    if (tcpServer.hasClient()) {
        if (!tcpClient || !tcpClient.connected()) {
            if(tcpClient) tcpClient.stop();
            tcpClient = tcpServer.available();
            Serial.println("AHRS client connected");
        }
        else {
            tcpServer.available().stop();
        }
    }
    if (tcpClient && tcpClient.connected()) {
        if (tcpClient.available()){
            // Discard incoming data for now.
            while (tcpClient.available()) tcpClient.read();
        }
        // write AHRS data to connected TCP client
        sensors_vec_t   orientation;

        // Use the simple AHRS function to get the current orientation.
        if (ahrs.getOrientation(&orientation)) {
            String aLine;
            aLine = "Orientation: " + 
                String(orientation.roll) + " " +
                String(orientation.pitch) + " " +
                String(orientation.heading);
            tcpClient.println(aLine);
        }
    }
    delay(50);
}
