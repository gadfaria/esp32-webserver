#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/* Put your SSID & Password */
const char *ssid = "";
const char *password = "";

/* Structure with expected information */
struct infos
{
    String country;
    String cases;
    String confirmed;
    String deaths;
    String recovered;
    String updated_at;
};

/* Put yout web server port */
WebServer server(80);
infos infos;
HTTPClient http;
StaticJsonDocument<500> doc;

void setup()
{
    Serial.begin(115200);
    delay(4000);

    http.begin("https://covid19-brazil-api.now.sh/api/report/v1/brazil"); //API to get infos from covid-19 in Brazil

    /* Connect to Wi-Fi network with SSID and password */
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); // Print local IP address

    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);

    server.begin(); // Start web server
}

void loop()
{
    server.handleClient();
}

void handle_OnConnect()
{
    int httpCode = http.GET();
    if (httpCode == 200) // Successful request code = 200
    {
        Serial.println("Successful request");
        String payload = http.getString();
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.print("Fail! Error: ");
            Serial.println(error.c_str());
        }
        else
        {
            infos = {doc["data"]["country"], doc["data"]["cases"], doc["data"]["confirmed"], doc["data"]["deaths"], doc["data"]["recovered"], doc["data"]["updated_at"]};
        }
    }
    else
    {
        Serial.println("Request failed");
    }
    Serial.println("On Connect");
    server.send(200, "text/html", SendHTML());
}

void handle_NotFound()
{
    server.send(404, "text/plain", "Not found");
}

String SendHTML()
{
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr += "<title>ESP32 Web Server</title>\n";
    ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
    ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
    ptr += "</style>\n";
    ptr += "</head>\n";
    ptr += "<body>\n";
    ptr += "<h1>COVID-19</h1>\n";
    ptr += "<h3>ESP32 Web Server - PETTEC</h3>\n";
    ptr += ("<p>Country: " + infos.country + "</p>");
    ptr += ("<p>Cases: " + infos.cases + "</p>");
    ptr += ("<p>Confirmed: " + infos.confirmed + "</p>");
    ptr += ("<p>Deaths: " + infos.deaths + "</p>");
    ptr += ("<p>Recovered: " + infos.recovered + "</p>");

    ptr += "</body>\n";
    ptr += "</html>\n";
    return ptr;
}
