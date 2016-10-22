/**************************************************************************
* Number of pixels that are sent out to read appears to be related to how *
* fast the SH cycle is to the MCLK cycle. If I am understanding the       *
* documentation correctly, to get all the pixels you would need to be     *
* running the SH at 1/4 the MCLK speed. You still however get data out    *
* when running at a lower speed. Just you get fewer effective pixels      *
**************************************************************************/

/*
tcd1304ap:
pin 1 -> 5 VDC
pin 2 -> 5 VDC
pin 3 -> pin 13 Mega (IGC 0x80 below)
pin 4 -> pin 10 Mega (MCLK 0x10 below)
pin 5 -> pin 12 Mega (SH 0x40 below)
pin 21 -> pin A0 Mega (analog input)
pin 22 -> GND
*/


#define SH 0x40
#define ICG 0x80
#define MCLK 0x10

#define CLOCK PORTB

uint16_t buffer[1847];

int exposureTime = 1; // number of micro seconds for a half cycle.

void setup()
{
  // Initialize the clocks.
  DDRB |= (SH | ICG | MCLK);    	// Set the clock lines to outputs
  CLOCK |= ICG;				// Set the integration clear gate high.

  // Enable the serial port.
  Serial.begin(115200);


  /***************************************************
  * At present this bit is largely still magic to me *
  ***************************************************/
  // Setup timer2 to generate a 470kHz frequency on D11
  TCCR2A =  (0 << COM2A1) | (1 << COM2A0) | (1 << WGM21) | (0 << WGM20); 
  TCCR2B = (0 << WGM22) | (1 << CS20);  // this and the line above seem to define some kind of voodoo
  OCR2A = 32;   // this line seems to mean switch every 6 cycles
  TCNT2 = 1;  // I have no idea what this line does

  // Set the ADC clock to sysclk/32
  ADCSRA &= ~((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0);
}

void readCCD(void)
{
  int x;
  int y;
  int result;


  // starting with IGC high then cycling SH a few times seems to get things started
  CLOCK |= ICG;  // set IGC HIGH
  for (y = 0; y < 7; y++)
  {
    delayMicroseconds(exposureTime);
    CLOCK |= SH;  // set SH HIGH
    delayMicroseconds(exposureTime);  
    CLOCK &= ~SH;  // set SH LOW
  }
  delayMicroseconds(300); // putting the exposure delay here seems to work

  /******************************************************
  * To start the chip the following magic appears to be *
  * needed. IGC LOW, SH HIGH, wait a half cycle, SH LOW *
  * wait a half cycle then ICG HIGH. Then start cycling *
  * SH HIGH/LOW to get pixels out                       *
  ******************************************************/
  delayMicroseconds(exposureTime);  
  CLOCK &= ~ICG;  // set IGC LOW.

  CLOCK |= SH; 
  delayMicroseconds(exposureTime);  
  CLOCK &= ~SH;
  delayMicroseconds(exposureTime);
  CLOCK |= ICG;

  delayMicroseconds(exposureTime);  
  CLOCK |= SH;
  // 3694 pixels, a few at the ends are dummy/dark
  // at this point we can only see on the scope. However,
  // while the analogRead isn't accurate, preforming it
  // does not seem to interrupt the scope output. As such
  // the delay caused by the read doesn't appear to be an issue.
  // Writing however does appear to make a mess. It is possible
  // that this is why I have seen most people save to an array
  // then write later.
//  for (x = 0; x < 3694; x++)  // seems that one gets a new pixel on both the high and the low...?
    for (x = 0; x < 1847; x++)  // if one gets a new pixel on both high and low this is all that is necessary. This may not be true.
  {
    delayMicroseconds(exposureTime);
    // for the moment just read here; reading after the SH goes high as well results in too much slowing
    buffer[x] = analogRead(A0); 
    delayMicroseconds(exposureTime);  
  }
    Serial.println();
}


void sendData(void)
{
  int x;

  for (x = 0; x < 1847; ++x)
  {
    if ( x % 60 == 0 || x == 1847) {
// I couldn't get printf to work to serial, the below works
// but feels wrong...Though I'm told it has a smaller code
// footprint due to the lack of stdio.h
      if (buffer[x] < 10)
      {
              Serial.print('00');
      }
      else if (buffer[x] < 100)
      {
              Serial.print('0');
      }
      Serial.print(buffer[x]);
      Serial.print(' ');
    }
  }
  Serial.print('a'); // using this to track the end of the data stream
}

void loop()
{
  readCCD(); // always read the data (quasi-realtime)
  // only send when input recieved, any input, letter number whatever,
  // such that the computer can control when it recieves data.
  if (Serial.available())
  {
    sendData();
  }
}
