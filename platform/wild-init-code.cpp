
  modem_port_in->begin(SerialModule_Baudrate);
    gnss_port_in->begin(SerialGNSS_BAUDRATE);
    debug_port_in->enableBlockingTx();
  // Seed the random
  pinMode(ANALOG_RND_PIN, INPUT);
  randomSeed(analogRead(ANALOG_RND_PIN));


  modem_port_in->begin(BG96_Baudrate);
