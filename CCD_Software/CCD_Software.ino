#include "CCD_Software.h"

uint16_t SH_Period = 0;

void setup()
{
    Serial.begin(9600);

    pinMode(MASTER_CLK, OUTPUT);
    pinMode(ICG, OUTPUT);
    pinMode(SH, OUTPUT);
    
    pinMode(CLK_IN, INPUT);
    pinMode(CCD_IN, INPUT);
    pinMode(DATA_CLK_IN, INPUT);

    pinMode(CONTROL_LED_265, OUTPUT);
    pinMode(CONTROL_LED_275, OUTPUT);
    pinMode(CONTROL_LED_280, OUTPUT);
    pinMode(CONTROL_LED_310, OUTPUT);
    pinMode(CONTROL_LED_365, OUTPUT);

    analogWriteResolution(8);
    analogWriteFrequency(MASTER_CLK, MC_FREQ);
    analogWriteFrequency(DATA_CLK, MC_FREQ / 4);
    analogWrite(MASTER_CLK, 128);
    analogWrite(DATA_CLK, 32);

    attachInterrupt(digitalPinToInterrupt(CLK_IN), ccd_isr, RISING);

    Serial.println("RoveComm Initializing...");
    RoveComm.begin(RC_SCIENCESENSORSBOARD_FIRSTOCTET, RC_SCIENCESENSORSBOARD_SECONDOCTET, RC_SCIENCESENSORSBOARD_THIRDOCTET, RC_SCIENCESENSORSBOARD_FOURTHOCTET, &TCPServer);
    Serial.println("Complete");
}

void loop()
{
    rovecomm_packet packet = RoveComm.read();
    switch (packet.data_id)
    {

    // Turn on/off LEDs
    case RC_SCIENCESENSORSBOARD_FLUOROMETERLEDS_DATA_ID:
    {
        uint8_t data = ((uint8_t *)packet.data)[0];
        digitalWrite(CONTROL_LED_265, data & (1 << 0) ? HIGH : LOW);
        digitalWrite(CONTROL_LED_275, data & (1 << 1) ? HIGH : LOW);
        digitalWrite(CONTROL_LED_280, data & (1 << 2) ? HIGH : LOW);
        digitalWrite(CONTROL_LED_310, data & (1 << 3) ? HIGH : LOW);
        digitalWrite(CONTROL_LED_365, data & (1 << 4) ? HIGH : LOW);
        break;
    }

    // Read and transmit CCD buffer
    case RC_SCIENCESENSORSBOARD_REQFLUOROMETER_DATA_ID:
    {
        SHPeriod = ((uint16_t *)packet.data)[0];
        
        Serial.println("Reading...");
        readCCD();

        Serial.println("Writing...");
        RoveComm.write(RC_SCIENCESENSORSBOARD_FLUOROMETERDATA1_DATA_ID, RC_SCIENCESENSORSBOARD_FLUOROMETERDATA1_DATA_COUNT, (uint16_t)(&ccd_buff[0]));
        RoveComm.write(RC_SCIENCESENSORSBOARD_FLUOROMETERDATA2_DATA_ID, RC_SCIENCESENSORSBOARD_FLUOROMETERDATA2_DATA_COUNT, (uint16_t)(&ccd_buff[500]));
        RoveComm.write(RC_SCIENCESENSORSBOARD_FLUOROMETERDATA3_DATA_ID, RC_SCIENCESENSORSBOARD_FLUOROMETERDATA3_DATA_COUNT, (uint16_t)(&ccd_buff[1000]));
        RoveComm.write(RC_SCIENCESENSORSBOARD_FLUOROMETERDATA4_DATA_ID, RC_SCIENCESENSORSBOARD_FLUOROMETERDATA4_DATA_COUNT, (uint16_t)(&ccd_buff[1500]));
        RoveComm.write(RC_SCIENCESENSORSBOARD_FLUOROMETERDATA5_DATA_ID, RC_SCIENCESENSORSBOARD_FLUOROMETERDATA5_DATA_COUNT, (uint16_t)(&ccd_buff[2000]));
        RoveComm.write(RC_SCIENCESENSORSBOARD_FLUOROMETERDATA6_DATA_ID, RC_SCIENCESENSORSBOARD_FLUOROMETERDATA6_DATA_COUNT, (uint16_t)(&ccd_buff[2500]));
        RoveComm.write(RC_SCIENCESENSORSBOARD_FLUOROMETERDATA7_DATA_ID, RC_SCIENCESENSORSBOARD_FLUOROMETERDATA7_DATA_COUNT, (uint16_t)(&ccd_buff[3000]));
        RoveComm.write(RC_SCIENCESENSORSBOARD_FLUOROMETERDATA8_DATA_ID, RC_SCIENCESENSORSBOARD_FLUOROMETERDATA8_DATA_COUNT, (uint16_t)(&ccd_buff[3500]));

        break;
    }

    // Default
    default:
    {
        break;
    }
    }
}

void readCCD()
{
    while (!digitalRead(DATA_CLK_IN));

    digitalWrite(ICG, HIGH);
    digitalWrite(SH, LOW);
    delayMicroseconds(2);
    digitalWrite(SH, HIGH);
    delayMicroseconds(5);
    //while (!digitalRead(CLK_IN));
    digitalWrite(ICG, LOW);

    ccd_buff_index = 0;
    attachInterrupt(digitalPinToInterrupt(DATA_CLK_IN), ccd_isr, RISING);
    delay((1/(float)(MC_FREQ/4))*NUM_CCD_ELEMENTS*1000);
    detachInterrupt(digitalPinToInterrupt(DATA_CLK_IN));
    
    digitalWrite(ICG, HIGH);
    digitalWrite(SH, LOW);
    delayMicroseconds(2);
    digitalWrite(SH, HIGH);

    delay(10);
}

void ccd_isr()
{
    if (ccd_buff_index >= 32 && ccd_buff_index < (32 + 3648))
    {
        ccd_buff[ccd_buff_index - 32] = analogRead(CCD_IN);
    }
    ccd_buff_index++;
}
