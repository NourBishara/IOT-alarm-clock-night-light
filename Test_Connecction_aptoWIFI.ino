//Describtion: use AP and server to allow user to connect and enter home wifi address and password. 
//Connect to Wifi


#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

const char* ssid = "Nours'_AP_esp";
const char* password = NULL;

WebServer server(80);

global bool isWifi=false;

void handleLogin() {
  String msg;
  
  
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    {

      Serial.println("wifi name:" );
      Serial.println(server.arg("USERNAME"));
      Serial.println("password::");
      Serial.println( server.arg("PASSWORD"))  ;    
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);


      //Serial.begin(115200);
       WiFi.mode(WIFI_STA);
       WiFi.begin(server.arg("USERNAME").c_str(), server.arg("PASSWORD").c_str());
       Serial.println("Attempting connection, is in station mode");

        // Wait for connection
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
         Serial.print(".");
        }
       Serial.println("");
       Serial.print("Connected to ");
       Serial.println(server.arg("USERNAME"));
       Serial.print("IP address: ");
       Serial.println(WiFi.localIP());
      isWifi=true;
      return;
    }
    
  }
  String content = "<html><body><form action='/login' method='POST'>To Connect to wifi, please enter Network and password:<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='wifi network'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "You also can go <a href='/inline'>here</a></body></html>";
  server.send(200, "text/html", content);
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");

  //server.on("/", handleRoot);
  server.on("/", handleLogin);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works without need of authentification");
  });

   
  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  // put your main code here, to run repeatedly:
  if(!isWifi){
    server.handleClient();
    handleLogin();
    delay(500);
  }
  else{

  }
}
