#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  if (Serial2.available() >= 4) { // wait for 4 bytes to arrive
    uint32_t buffer = Serial2.read() << 24; // read the first byte and shift it into the highest byte of buffer
    buffer |= Serial2.read() << 16; // read the second byte and shift it into the next byte of buffer
    buffer |= Serial2.read() << 8; // read the third byte and shift it into the next byte of buffer
    buffer |= Serial2.read(); // read the fourth byte and put it in the lowest byte of buffer

    uint16_t x = (buffer >> 16) & 0x07FF; // shift right to remove padding and mask to keep lower 11 bits
    uint16_t y = buffer & 0x07FF; // mask the lower 11 bits to get y

    Serial.print("x: ");
    Serial.print(x, HEX); 
    Serial.print(", y: ");
    Serial.println(y, HEX); 
  }
}
