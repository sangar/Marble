#include "MarbleMain.h"

#include "nRF24L01P.h"

#define TRANSFER_SIZE   4

const int CSN = 17;
const int CE = 18;

nRF24L01P my_nrf24l01p(MOSI, MISO, SCK, CSN, CE, 0);

char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
int txDataCnt = 0;
int rxDataCnt = 0;

char serialMsg[TRANSFER_SIZE];

//DigitalOut dle(9);
//DigitalOut led(10);
AnalogOut led(10);

/*
 * Main methods
 */
void MarbleMain::setup() {
	Serial.link();
	
	Serial.println("Serial up and running.");
	
	my_nrf24l01p.powerUp();
	_delay_ms(1000);

	Serial.println("nRF24L01+ powered up.");
	
	// Display the (default) setup of the nRF24L01+ chip
	Serial.printf("nRF24L01+ Frequency    : %d MHz\n\r", my_nrf24l01p.getRfFrequency());
	Serial.printf("nRF24L01+ Output power : %d dBm\n\r", my_nrf24l01p.getRfOutputPower());
	Serial.printf("nRF24L01+ Data Rate    : %d kbps\n\r", my_nrf24l01p.getAirDataRate());
	unsigned long long txaddr = my_nrf24l01p.getTxAddress();
	unsigned long long rxaddr = my_nrf24l01p.getRxAddress();
	Serial.printf("nRF24L01+ TX Address   : 0x%2lX%8lX\n\r", ((unsigned long *) &txaddr)[1], ((unsigned long *) &txaddr)[0]);
	Serial.printf("nRF24L01+ RX Address   : 0x%2lX%8lX\n\r", ((unsigned long *) &rxaddr)[1], ((unsigned long *) &rxaddr)[0]);

	Serial.printf("Type keys to test transfers:\r\n  (transfers are grouped into %d characters)\n\r", TRANSFER_SIZE);

	my_nrf24l01p.setTransferSize( TRANSFER_SIZE );

	my_nrf24l01p.setReceiveMode();
	my_nrf24l01p.enable();

	led = HIGH;
} // end setup


void MarbleMain::loop() {

	memset(serialMsg, 0, 4);
	
	// If we've received anything in the nRF24L01+...
	if ( my_nrf24l01p.readable() ) {

	    // ...read the data into the receive buffer
	    rxDataCnt = my_nrf24l01p.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );

		for (int i=0; i<rxDataCnt; i++) {
			Serial.printf("Got value %d\r\n", (byte)rxData[i]);
		}

	    memcpy(serialMsg, rxData, 4);

	    // to help debug nRF24L01+ -> Host communication
		Serial.printf("Received %d bytes.\n\r", rxDataCnt);
	}
	
	// Message received
	if (serialMsg[0] != 0) {
		switch (serialMsg[0]) {
			case 1:
				switch (serialMsg[1]) {
					case 1:
						switch (serialMsg[2]) {
							case 1:
								led = (byte) serialMsg[3];
						}
				}
		}
	}
} // end loop
