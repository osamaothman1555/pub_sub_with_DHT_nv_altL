/* ESP32 AWS IoT
 *  
 * Simplest possible example (that I could come up with) of using an ESP32 with AWS IoT.
 *  
 * Author: Anthony Elder 
 * License: Apache License v2
 * Sketch Modified by Stephen Borsay for www.udemy.com
 * https://github.com/sborsay
 * Add in Char buffer utilizing sprintf to dispatch JSON data to AWS IoT Core
 * Use and replace your own SID, PW, AWS Account Endpoint, Client cert, private cert, x.509 CA root Cert
 */
#include <WiFiClientSecure.h>
#include <PubSubClient.h> // install with Library Manager, I used v2.6.0
#include <dht.h>

#define dht_pin 27

char incoming_Msg[500];

const char* awsEndpoint = "a327fhhb7dw2gm-ats.iot.us-east-1.amazonaws.com";

/* Update the two certificate strings below. Paste in the text of your AWS 
 * device certificate and private key. Add a quote character at the start
 * of each line and a backslash, n, quote, space, backslash at the end 
 * of each line. 
 * For the rest start with quote and end with quote, space and backslash:
 */

// xxxxxxxxxx-certificate.pem.crt
const char* certificate_pem_crt = \

"-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVANjI6x4fOROftMFTvEVL3z6ZDchdMA0GCSqGSIb3DQEB" \
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t" \
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMTA3MTQxNDU0" \
"NDdaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh" \
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC28IOeaOcnIz97yTed" \
"f0IUyEfQAgXDc60XO2l2uS4DwYUx0eIw/5rkhD38vAqVn1XTQKT+EymMtF5UdrcN" \
"B0sblTKPOxSZNFUelGhnf+m/Lt9ZxMYm6PhHb2uQGWkzyHhysSTE82sEVo8vRlFR" \
"HkMoy/9kURrq3QDf/A3o502LeYLIWAWllc8xmLZL1M6VQfWE0ztZ9cYYowq4O2ns" \
"ZA9v7KpD+L3yNlT/8phIRCJ5EQB07DEdT810H+MMNVtZsBTEUxY7R+mlFROnQlRg" \
"vi0pPKYKAjcuBVxQ3rDKPQ4zAFgcEQdOSc+NyFzd2lq2hAtlBZUOU+UR5kyuJrhL" \
"Sjb1AgMBAAGjYDBeMB8GA1UdIwQYMBaAFM1rGvomQZjq5BtbZlDuE5E+s3zzMB0G" \
"A1UdDgQWBBTYLou+qu0aNvh1lujnnFy8llaX1zAMBgNVHRMBAf8EAjAAMA4GA1Ud" \
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAmuFgC49OVuPvpI7hHJdL1JuE" \
"ugc9Yfofox8bHLRdy02tF8b0qZgzfKkVetY+Ay5h8WgsWYm0/GF5hWR1tAgJwVXg" \
"m0l7jU7ns1coJCB/Y01rUDMwD0EIpLAUam/vICkVr5Wd5BfjE2Jk5YiFLEQdO4E8" \
"L4/GzpLMs17NCTXbFn0ycy3cjL52mgEOYLlgwY2O5GFXhJ6/DzvgPpdv9QsJWw4C" \
"6Dr2R7imngPoqQc7IvpMSJtZQq1vpGtAZxcC2Gc2UGpSQkmIAomxPl1HrIciAT+i" \
"wDWg1ARtTuKnOwgMGPqQqa41lyxby6ULUv2FTCmwxfEP6DoaJlkoVE+7imL1aA==" 
"-----END CERTIFICATE-----\n";

// xxxxxxxxxx-private.pem.key
const char* private_pem_key = \

"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpQIBAAKCAQEAtvCDnmjnJyM/e8k3nX9CFMhH0AIFw3OtFztpdrkuA8GFMdHi" \
"MP+a5IQ9/LwKlZ9V00Ck/hMpjLReVHa3DQdLG5UyjzsUmTRVHpRoZ3/pvy7fWcTG" \
"Juj4R29rkBlpM8h4crEkxPNrBFaPL0ZRUR5DKMv/ZFEa6t0A3/wN6OdNi3mCyFgF" \
"pZXPMZi2S9TOlUH1hNM7WfXGGKMKuDtp7GQPb+yqQ/i98jZU//KYSEQieREAdOwx" \
"HU/NdB/jDDVbWbAUxFMWO0fppRUTp0JUYL4tKTymCgI3LgVcUN6wyj0OMwBYHBEH" \
"TknPjchc3dpatoQLZQWVDlPlEeZMria4S0o29QIDAQABAoIBAFmIrzI5b2BlTO29" \
"ll2b6fCX8SWpx75YABANwNkdWpr1/MVcTpLilqFe46OtOJTwDaY9zkKXiE9X+47l" \
"J0PWHPlb9QnKxZMR3NMBmXOGVBWA0El0F/L1NIZpzsAQ3787T0/6yN8nr2nJH3zo" \
"smW8JOxzowQq+byW+5WkG/6wLfCjbNkZlCMJ+C/oAgdJTNbB/p+OY6EjkbwiF1t6" \
"B5wS0ZdfmVzFEz+c1qRiUOXo2rE3u5UHAn23xpxWXc1SpmdPWBIfUHdv25kUbzHr" \
"RuENhiDHsjW96YW622aPNaBuQWcwA/hJgXQBciR3mKX+Q2bH8htlymzLCgdX/rRm" \
"ikitRiECgYEA4GlIPnYCAFje7C4SG1NDH8gEWoQgVLUcyR8t5jpixEI66WOJLwSq" \
"b7wtq0Kx7BFrn1wjbI5GP+eAb+5C3A1r2khaxFBztgl74BWVxdjYAfqSuzMiFRZS" \
"69aLHinP639UjRkbXs5m1HEVDKuKOKzA7lSa18I8cpwgHRlPajBYog0CgYEA0LDJ" \
"Y59Mdh6ZtIpcAQDMLr2TAEkM1YhNupLjXVII2L29PZjxnw+Dn/oysLqxT5G+cztz" \
"RJ7q/3clNw1f3fr2jSy6iDdNlrxoxthUJD/TJdciYQv7aT+oBdILQsC0H1y0G7dL" \
"C03L/lx3D8JDq8dpMG6WzC+H45V3urdVkE7v9okCgYEA00hsm1oxwi/qGoW5yH5Y" \
"+WgxKSOy6O3oHze62ENqUAtnOevb8ie7bB3JBbByoac5I731IDTH0Uzd8QRrcJE2" \
"VTrAMorolqBRQAYykCb8IMjRM20ODrqI8cB6FLkbKEcmzP2xdk5wJF4fimFLbta/" \
"guSnMLKQhPuXhK4axgikulUCgYEAn6ykanUYK8h+EoVKn/ncQGcMcstx5m/ECV0a" \
"WezKmAuVH+xF804Lh/wHjPFLeXYqIkD6kSaG57Bh6R1ynIXI33u5vT+TPiIqiPo8" \
"Zv/urqVMyLRCSOVLyihMNWb8aoKjzBESejOsQZK+BnC9FlIdSdFT0CcN8jQKx/I+" \
"pYvATwECgYEAn5kiCR/P3iM54sD/xcfHG7iwFP3y2OZn+AtwWj7cPD3DFu2xbzcM" \
"9lcXRdZkceKaTeVpekBb+DHQg6wIFkksY/XRv/UNe9wUWezt4LGbSh+r1uS2FvAV" \
"aWDvY8t7bnVSNxONB3osDicRrLdSfBnfL9oflqp2dhv+RRUsk3H+KtI="
"-----END RSA PRIVATE KEY-----\n";

/* root CA can be downloaded in:
  https://www.symantec.com/content/en/us/enterprise/verisign/roots/VeriSign-Class%203-Public-Primary-Certification-Authority-G5.pem
*/

const char* rootCA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy" \
"rqXRfboQnoZsG4q5WTP468SQvvG5"
"-----END CERTIFICATE-----\n";

WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 

void setup() {
  Serial.begin(115200); delay(50); Serial.println();
  Serial.println("ESP32 AWS IoT Example");
  Serial.printf("SDK version: %s\n", ESP.getSdkVersion());

  Serial.println("Please input SSID (wifi name): ");
  while (Serial.available() == 0) 
  {/*Wait for User Input*/ }
  const char* ssid = (Serial.readString()).c_str();
  Serial.println("Please input wifi password: ");
  while (Serial.available() == 0) 
  {/*Wait for User Input*/ }
  const char* password = (Serial.readString()).c_str();

  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  wiFiClient.setCACert(rootCA);
  wiFiClient.setCertificate(certificate_pem_crt);
  wiFiClient.setPrivateKey(private_pem_key);

  pinMode(27,INPUT);
  
}

unsigned long lastPublish;
int msgCount;
dht DHT_0;
char* subTopic = "mytemptopic";
char* pubTopic = "inTopic";

void loop() {

  pubSubCheckConnect(pubTopic);

   //If you need to increase buffer size, you need to change MQTT_MAX_PACKET_SIZE in PubSubClient.h
   char realData[128];

  float temperature = 0; 
  float humidity =  0;
  
  if (millis() - lastPublish > 900000)   //Checks temperature and humidity every 15 minutes
  {
    DHT_0.read11(dht_pin);
    temperature = DHT_0.temperature;
    humidity = DHT_0.humidity;
    sprintf(realData,  "{\"uptime\":%lu,\"temperature\":%f,\"humid\":%f}", millis() / 1000, temperature, humidity);   //Packages data into a JSON string
    boolean rc = pubSubClient.publish(subTopic, realData);
    Serial.print("Published, rc="); Serial.print( (rc ? "OK: " : "FAILED: ") );
    Serial.println(realData);
    lastPublish = millis();  
  }
}

/*
 * Prints out message payload received from topic subscribed
 * @param topic The topic to subscribe to receive message.
 * @return The area of the circle.
*/
void msgReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    incoming_Msg[i] = payload[i];
  }
  Serial.println();
}

/*
 * Connects ESP32 device to AWSIoT. 
 * Also checks for incoming payloads from subcribed topic
 * @param topic The topic to subscribe to receive message.
*/

void pubSubCheckConnect(char* topic) {
  if ( ! pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("ESPthingXXXX");
      delay(1000);
    }
    Serial.println(" connected");
    pubSubClient.subscribe(topic);
  }
  pubSubClient.loop();
}
