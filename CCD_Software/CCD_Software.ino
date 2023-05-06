#include <RoveComm.h>

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
//#include <MIMXRT1062.h>

/******************************************************************
 *                        DEFINITIONS
 ******************************************************************/
#define DEMO_ADC_BASE           ((ADC_Type *)(0x400C4000u)) // ADC1     ((ADC_Type *)ADC1_BASE)


// Pins
#define CCD_IN A10
#define SH 2
#define ICG 3
#define MASTER_CLK 1
#define DATA_CLK 5
#define CLK_IN 4

uint16_t ccd_buff[3648];

int i = 0;


RoveCommEthernet RoveComm;
EthernetServer TCPServer(RC_ROVECOMM_ETHERNET_TCP_PORT);



void setup() {
  Serial.begin(9600);

  pinMode(SH, OUTPUT);
  pinMode(ICG, OUTPUT);
  pinMode(MASTER_CLK, OUTPUT);
  pinMode(DATA_CLK, OUTPUT);
  pinMode(CLK_IN, INPUT);

  //ADC_Configuration();
  //XBARA_Configuration();

  /*
  if (ARM_DWT_CYCCNT == ARM_DWT_CYCCNT) {
    // Enable CPU Cycle count
    ARM_DEMCR |= ARM_DEMCR_TRCENA;
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
  }
  */

  
  //analogWriteResolution(12);
  analogWriteFrequency(MASTER_CLK, 1000000);
  analogWriteFrequency(DATA_CLK, 1000000/4);
  analogWrite(MASTER_CLK, 128);
  analogWrite(DATA_CLK, 32);
  
  //analogWriteFrequency(DATA_CLK, 500000); // FlexPWM2_1_A       EMC_08

  /*
  analogWriteFrequency(ICG, 133.33333);
  analogWrite(ICG, 5.46);
  analogWriteFrequency(SH, 100000);
  analogWrite(SH, 1638);
  */

  /*
  Serial.println("A");
  RoveComm.begin(RC_SCIENCESENSORSBOARD_FIRSTOCTET, RC_SCIENCESENSORSBOARD_SECONDOCTET, RC_SCIENCESENSORSBOARD_THIRDOCTET, RC_SCIENCESENSORSBOARD_FOURTHOCTET, &TCPServer);
  Serial.println("B");
  */
  
}

void loop() {
  // analogread takes about 2 us to complete.
  // digitalRead takes about 800 ns to complete.
  /*
  static uint32_t cntLast = ARM_DWT_CYCCNT;
  uint32_t cnt = ARM_DWT_CYCCNT;
  float ns = cnt * 1E9f/F_CPU;
  Serial.printf("Cycles in analogRead %10u\tCYCCNT: %10u (%0.6g ns)\n", cnt - cntLast, cnt, ns);
  cntLast = cnt;
  analogRead(CLK_IN);
  */
  
  /*
  Serial.print("FLEXPWM STATUS: ");
  Serial.println(FLEXPWM2_SM1STS, BIN);
  Serial.print("ADC result register:");
  Serial.println(ADC1_R0, BIN);
  Serial.print("ADC ETC result register: ");
  Serial.println(ADC_ETC_TRIG0_RESULT_1_0, BIN);
  */

  /*
  rovecomm_packet packet = RoveComm.read();
  switch (packet.data_id) {

    case RC_SCIENCESENSORSBOARD_REQFLUOROMETER_DATA_ID:
    {
      Serial.println("Received.");
    
      readCCD();
      RoveComm.write(10100, 500, (&ccd_buff[0]));
      RoveComm.write(10100, 500, (&ccd_buff[500]));
      RoveComm.write(10100, 500, (&ccd_buff[1000]));
      RoveComm.write(10100, 500, (&ccd_buff[1500]));
      RoveComm.write(10100, 500, (&ccd_buff[2000]));
      RoveComm.write(10100, 500, (&ccd_buff[2500]));
      RoveComm.write(10100, 500, (&ccd_buff[3000]));
      RoveComm.write(10100, 148, (&ccd_buff[3500]));
      break;
    }

    default:
      break;
  }
  */
  
  
 /*
  readCCD();
  
  for(int j = 500; j < 1000; j++) {
    Serial.println(ccd_buff[j]);
  }
  
  while(1);*/
  
}

void readCCD() {
  while (digitalRead(CLK_IN));
  
  digitalWrite(SH, LOW);
  digitalWrite(ICG, HIGH);
  delayMicroseconds(1);
  digitalWrite(SH, HIGH);
  delayMicroseconds(5);
  digitalWrite(ICG, LOW);

  i = 0;
  
  attachInterrupt(digitalPinToInterrupt(CLK_IN), ccd_isr, RISING);
  delay(20);
  detachInterrupt(digitalPinToInterrupt(CLK_IN));
  Serial.println(i);
}

void ccd_isr() {
  if (i >= 32 && i < (32+3648)) {
    ccd_buff[i-32] = analogRead(CCD_IN);
  }
  i++;
}

void ADC_Config()
{
  // --ADC Configuration register--

  // Bit 16:      Disable Data Overwrite
  // Bits 15-14:  Hardware average select (disabled because averaging is disabled)
  // Bit 13:      Hardware trigger selected
  // Bits 12-11:  Select VREFH/VREFL for voltage reference
  // Bit 10:      High Speed conversion selected
  // Bits 9-8:    ADSTS; total sample time set to 3 ADC clock cycles with Long sample time disabled
  // Bit 7:       ADLPC; Low-power mode disabled
  // Bits 6-5:    ADIV; Input clock not divided
  // Bit 4:       ADLSMP; Set to short sample mode
  // Bits 3-2:    MODE; ADC Resolution set to 8-bit conversion
  // Bits 1-0:    ADICLK; IPG clock divided by 1 selected as input clock source to generate ADCK
  ADC1_CFG = (0<<16) | (1<<13) | (0b00<<11) | (0<<10) | (0b00<<8) | (0<<7) | (0b00<<5) | (0<<4) | (0b10<<2) | (0b00<<0);

  // --General Control register--

  // Bit 7:       CAL; Calibration bit begins calibration when set
  // Bit 6:       ADCO; Continuous Conversion disabled
  // Bit 5:       AVGE; Hardware average disabled
  // Bit 4:       ACFE; Compare Function disabled
  // Bit 3:       ACFGT; Compare Function Greater than Disabled
  // Bit 2:       ACREN; Compare function range disabled
  // Bit 1:       DMAEN: DMA disabled
  // Bit 0:       ADACKEN: Disable asynchronous clock output. Asynchronous clock only enabled if selected by ADICLK and a conversion is active.
  ADC1_GC = (0<<7) | (0<<6) | (0<<5) | (0<<4) | (0<<3) | (0<<2) | (0<<1) | (0<<0);

  // --Trigger Control Register--
  
  // Disable COCO interrupts
  ADC1_HC0 &= (0<<7);
  ADC1_HC0 &= (0<<7);

  // Set hardware triggers as external channels 15 for HC0 and HC1
  ADC1_HC0 |= (0b01111<<0);
  ADC1_HC1 |= (0b01111<<0);
}

void ADC_Calibrate()
{
    // --Calibration--
  /*To complete calibration, the user must follow the below procedure:
   *  1. Configure ADC_CFG with actual operating values for maximum accuracy.
   *  2. Configure the ADC_GC values along with CAL bit.
   *  3. Check the status of CALF bit in ADC_GS and the CAL bit in ADC_GC.
   *  4. When CAL bit becomes '0' then check the CALF status and COCO[0] bit status.
   */

  bool bHWTrigger = false;

  if (0U != ((0x2000U) & ADC1_CFG))
  {
    bHWTrigger = true;
    
  }
}

void ADC_ETC_Config()
{
  /* Initialization:
   *  1. Configure the Global Control bits, include selecting DMA mode, 
   *     enabling triggers which will be used, and setting pre-division factor.
   *  2. Set DMA enable bits as needed.
   *  3. Configure control bits for each enabled trigger, include enabling or 
   *     disabling synchronization mode, assigning trigger priority, setting the 
   *     length of trigger chain, selecting trigger mode and setting the initial 
   *     and interval delay of this trigger.
   *  4. According to the length of trigger chain, several segments of the chain 
   *     should be configured. The configurations of each segment contain:
   *      a. setting the DONE interrupt for this chain, enabling or disabling B2B mode;
   *      b. selecting which trigger of ADC will be triggered, setting the ADC command.
   */


  // --Global Control Register--
  // Bit 31:      SOFTRST:            Software synchronous reset disabled (active high)
  // Bit 30:      TSC_BYPASS:         TSC Not bypassed. This should be bypassed to use ADC2
  // Bit 29:      DMA_MODE_SEL:       Trigger DMA_REQ with latched signal
  // Bits 23-16:  PRE_DIVIDER:        Pre-divider for trig delay and interval
  // Bits 15-13:  EXT1_TRIG_PRIORITY: External TSC1 (touch source control) trigger priority (7 highest, 0 lowest). Set to 0.
  // Bit 12:      EXT1_TRIG_ENABLE:   Disable external TSC1 trigger.
  // Bits 11-9:   EXT0_TRIG_PRIORITY: External TSC0 trigger priority 0
  // Bit 8:       EXT0_TRIG_ENABLE:   Disable external TSC0 trigger.
  // Bits 7-0:    TRIG_ENABLE:        Enable external XBAR trigger0.
  ADC_ETC_CTRL &= (0<<31);
  ADC_ETC_CTRL = (0<<30) | (0<<29) | (0b00000000<<16) | (0b000<<13)|(0<<12) | (0b000<<9)|(0<<8) | (0b00000001<<0);

  // --Trigger Control Register--
  // Bit 16:      SYNC_MODE:          Synchronization mode disabled
  // Bits 14-12:  TRIG_PRIORITY:      Set external trigger priority to 7
  // Bits 10-8:   TRIG_CHAIN:         Set trigger chain length to 1.
  // Bit 4:       TRIG_MODE:          Hardware trigger mode. Software trigger will be ignored.
  // Bit 0:       SW_TRIG:            No software trigger even generated.
  ADC_ETC_TRIG0_CTRL = (0<<16) | (0b111<<12) | (0b000<<8) | (0<<4) | (0<<0);

  // --Trigger Counter Register--
  // Bits 31-16:  SAMPLE_INTERVAL: TRIGGER sampling interval counter. When B2B is unset: Interval_delay = (SAMPLE_INTERVAL+1)*(PRE_DIVIDER+1)*ipg_clk
  // Bits 15-0:   INIT_DELAY:      TRIGGER initial delay counter. Initial_delay = (INT_DELAY+1)*(PRE_DIVIDER+1)*ipg_clk
  ADC_ETC_TRIG0_COUNTER = (0b0000000000000001<<16) | (0b0000000000000001<<0);

  // --Trigger Chain 0/1 Register
  // Bits 30-29:  IE1: Segment 1 done interrupt selected to no interrupt when finished
  // Bit 28:      B2B1: Segment 1 B2B disabled
  // Bits 27-20:  HWTS1: Segment 1 Hawrdware trigger selection. No trigger selected.
  // Bits 19-16:  CSEL1: ADC Channel 0 selected.
  // Bits 14-13:  IE0: Segment 0 done interrupt selected to no interrupt when finished.
  // Bit 12:      B2B0: Segment 0 B2B disabled
  // Bits 11-4:   HWTS0: Segment 0 Hardware trigger selection selected to ADC TRIG0.
  // Bits 3-0:    CSEL0: ADC Channel Selection.
  ADC_ETC_TRIG0_CHAIN_1_0 = (0b00<<29) | (0<<28) | (0b00000000<<20) | (0b0000<<16) | (0b00<<13) | (0<<12) | (0b00000001<<4) | (0b0000<<0);

  // Set PWM module to send a hardware trigger to XBAR
  // Bit 15:      PWAOT0:   Output Trigger 0 Source Selected to route PWMA signal to PWM_OUT_TRIG0 port
  FLEXPWM2_SM1TCTRL = (1<<15);

  // XBAR configure to link PWM trigger and ADC trigger
  // FLEXPWM2_PWM1_OUT_TRIG0  = XBAR1_IN44
  // ADC_ETC_TRIG00           = XBAR1_OUT103

  // Bits 14-8:   Input to be muxed to XBAR_OUT103. Set to 44 for FLEXPWM2_PWM1_OUT_TRIG0.
  XBARA1_SEL51 = (0b00101100<<8);


}
