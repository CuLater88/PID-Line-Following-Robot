void setup() {
  // put your setup code here, to run once:
  Serial0.begin(115200);
  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial0.println("SERIAL BEGINS HERE");
  int bytes = Serial0.available();
  if(bytes > 0){
    Serial0.println("COMS ESTABLISHED");
  }
  delay(3000);
}