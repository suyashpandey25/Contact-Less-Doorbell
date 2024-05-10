#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>

#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 465
#define SENDER_EMAIL "xxxxxxxxxxxxxxxx" // Mention Senders email
#define SENDER_PASSWORD "xxxxxxxxxxxx"  // mention app password created on gamil
#define RECIPIENT_EMAIL "xxxxxxxxxxxxx" // mention recievers email
#define RECIPIENT_NAME "DOOEBELL"

const int trigPin = D1;
const int echoPin = D2;
const int buzzerPin = D3; // Define buzzer pin
const int buttonPin = D6;  // Push button pin
const int relayPin = D7;   // Relay pin

int buttonState = 0;       // Current state of the button
int lastButtonState = 0;

const char* ssid = "xxxxxxx"; // mention the hotspot name 
const char* password = "xxxxxx"; // mention the hotspot password

enum Transfer_Encoding { enc_7bit, enc_base64 }; // Renamed enum

SMTPSession smtp;

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi");  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
    // Set the button pin as input
  pinMode(buttonPin, INPUT);
  
  // Set the relay pin as output
  pinMode(relayPin, OUTPUT);
}

void loop() {
  long duration, distance;
  
  buttonState = digitalRead(buttonPin);
  
  // Check if the button state has changed
  if (buttonState != lastButtonState) {
    // If the button is pressed (LOW state)
    if (buttonState == LOW) {
      // Toggle the state of the relay
      digitalWrite(relayPin, !digitalRead(relayPin));
      
      // Print the current state of the relay
      if (digitalRead(relayPin) == HIGH) {
        Serial.println("Relay ON");
      } else {
        Serial.println("Relay OFF");
      }
    }
    
    // Update the last button state
    lastButtonState = buttonState;
  }

  digitalWrite(trigPin, LOW);  
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  
  Serial.print("Distance: ");
  Serial.println(distance);
  
  // Check if distance is less than a threshold to consider it as motion detected
  if (distance < 10) { // Adjust threshold as needed
    Serial.println("Motion detected!");
    sendEmail();
    digitalWrite(buzzerPin, HIGH);
    delay(100); // Keep buzzer on for a brief moment
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
  }
  
  delay(1000);
}

void sendEmail() {
  ESP_Mail_Session session;
  session.server.host_name = SMTP_SERVER;
  session.server.port = SMTP_PORT;
  session.login.email = SENDER_EMAIL;
  session.login.password = SENDER_PASSWORD;
  session.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "DOOR BELL";
  message.sender.email = SENDER_EMAIL;
  message.subject = "IOT DOORBELL";
  message.addRecipient(RECIPIENT_NAME, RECIPIENT_EMAIL);
  
  String body = "Motion Detected";
  
  message.text.content = body.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Transfer_Encoding::enc_7bit; // Updated enum usage

  if (!smtp.connect(&session))
    return;

  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
  else
    Serial.println("Email sent successfully!");
}
