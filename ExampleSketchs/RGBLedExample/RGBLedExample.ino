int ledRojo = 0;
int ledVerde = 1;
int ledAzul = 2;
 
void setup() {
  pinMode(ledRojo,OUTPUT);
  pinMode(ledVerde,OUTPUT);
  pinMode(ledAzul,OUTPUT);
  }
   
  void loop() {
  //Hacer color rojo
  digitalWrite(ledRojo,255);
  digitalWrite(ledVerde,0);
  digitalWrite(ledAzul,0);
  delay(1500);
   
  //Hacer color verde
  digitalWrite(ledRojo,0);
  digitalWrite(ledVerde,255);
  digitalWrite(ledAzul,0);
  delay(1500);
   
  //Hacer color azul
  digitalWrite(ledRojo,0);
  digitalWrite(ledVerde,0);
  digitalWrite(ledAzul,255);
  delay(1500);
   
  //Hacer color blanco
  digitalWrite(ledRojo,255);
  digitalWrite(ledVerde,255);
  digitalWrite(ledAzul,255);
  delay(1500);
   
  //Hacer color amarillo
  digitalWrite(ledRojo,255);
  digitalWrite(ledVerde,255);
  digitalWrite(ledAzul,0);
  delay(1500);
   
  //Hacer color magenta
  digitalWrite(ledRojo,255);
  digitalWrite(ledVerde,0);
  digitalWrite(ledAzul,255);
  delay(1500);
   
  //Hacer color cian
  digitalWrite(ledRojo,0);
  digitalWrite(ledVerde,255);
  digitalWrite(ledAzul,255);
  delay(1500);
   
   
  //Hacer color rosa
  analogWrite(ledRojo,255);
  analogWrite(ledVerde,0);
  analogWrite(ledAzul,128);
  delay(1500);
}
