#include "F28x_Project.h"
#include "math.h"
#include "PLL_1PHA_SOGI.h"
//void SPLL_1ph_SOGI_IQ_FUNC(SPLL_1ph_SOGI_IQ *spll_obj);

extern float sin_tab[];
void GPIOsetup(void);
void ConfigureDAC(void);
__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void cpu_timer2_isr(void);
void InitEPwm1Example(void);
void InitEPwm2Example(void);
void SetupADCSoftware(void);
void ConfigureADC(void);
int count=0;
int AdcaResult0 = 0, doc_adc = 0, AdcaResult1= 0, m = 0, index =0;
double sinfunc = 0.0, goc = 0.0, D_alpha = 0.0, tanso = 0.0;
int SINUP ;
int goc_input = 0 ;
int SINLOW;
double AC_input = 0;
int i =0;
//------------------------------------------------

 int ADC[3] ;

float PI      =     3.14159265359;
int Grid_freq    = 50;
long int DELTA_T  = 20000;
float delta_T  ;
float wn   ;
SPLL_1ph_SOGI_IQ spll;

//void heso_osg_intit(int FG ,int FS); 1234321

//=============================
void main(void)
{


InitSysCtrl();
GPIOsetup();
InitEPwm1Gpio();
InitEPwm2Gpio();
SetupADCSoftware();
ConfigureADC();
ConfigureDAC();
DINT;
   InitPieCtrl();
   IER = 0x0000;
   IFR = 0x0000;
   InitPieVectTable();
   EALLOW;  // This is needed to write to EALLOW protected registers
   PieVectTable.TIMER0_INT = &cpu_timer0_isr;
   PieVectTable.TIMER1_INT = &cpu_timer1_isr;
   PieVectTable.TIMER2_INT = &cpu_timer2_isr;
   EDIS;    // This is needed to disable write to EALLOW protected registers
   InitCpuTimers();
   InitEPwm1Example();
   InitEPwm2Example();
// 200MHz CPU Freq, 1 second Period (in uSeconds)
   ConfigCpuTimer(&CpuTimer0, 200, 50);
   ConfigCpuTimer(&CpuTimer1, 200, 10);
   ConfigCpuTimer(&CpuTimer2, 200, 1000000);
   CpuTimer0Regs.TCR.all = 0x4000;
   CpuTimer1Regs.TCR.all = 0x4000;
   CpuTimer2Regs.TCR.all = 0x4000;

   IER |= M_INT1;
   IER |= M_INT13;
   IER |= M_INT14;
// Enable TINT0 in the PIE: Group 1 interrupt 7 111
//
   PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
// Enable global Interrupts and higher priority real-time debug events:
//
   EINT;  // Enable Global interrupt INTM
   ERTM;  // Enable Global realtime interrupt DBGM
   SPLL_1ph_SOGI_IQ_init( Grid_freq,((float)(1.0/DELTA_T)),&spll);


while(1)
   {
    DacaRegs.DACVALS.all = spll.theta[0] * 1000 + 1500  ;
    DacbRegs.DACVALS.all = AC_input * 2000 + 2000 ;
//    AdcaRegs.ADCSOCFRC1.all = 0x0003; //SOC0 and SOC1
//    while(AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
//    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
//    AdcaResult0 = AdcaResultRegs.ADCRESULT0;
//    AdcaResult1 = AdcaResultRegs.ADCRESULT1;
    SPLL_1ph_SOGI_IQ_coeff_update( 1.0/20000.0 ,  2.0*3.14159265359*50, &spll);

//    SPLL_1ph_SOGI_IQ_coeff_update(((float)(1.0/ISR_FREQUENCY)),(float)(2*PI*GRID_FREQ),&spll1);
   }
}
//*********** Structure Init Function ****//
//void heso_osg_intit(int FG , int FS)
//{
//    float k = 0.5 ;
//    double x= 2*PI*k*FG/FS;
//   double y = 2*PI*FG/FS;
//    y=y*y;
//    double  b0 = x/(x+y+4);
//    double b2 = - b0;
//    double a1 = 2*(4-y)/(x+y+4) ;
//    double a2 = (x-y-4)/(x+y+4);
//    for(i=0 ;   i<= 2  ;i++)
//    {
//        ADC[i]= 0;
//    }
//
//}
void SPLL_1ph_SOGI_IQ_init(int Grid_freq, long DELTA_T, SPLL_1ph_SOGI_IQ *spll_obj)
{
    spll_obj->u[0]=0.0;
    spll_obj->u[1]=0.0;
    spll_obj->u[2]=0.0;

    spll_obj->osg_u[0]=0.0;
    spll_obj->osg_u[1]=0.0;
    spll_obj->osg_u[2]=0.0;

    spll_obj->osg_qu[0]=0.0;
    spll_obj->osg_qu[1]=0.0;
    spll_obj->osg_qu[2]=0.0;

    spll_obj->u_Q[0]=0.0;
    spll_obj->u_Q[1]=0.0;

    spll_obj->u_D[0]=0.0;
    spll_obj->u_D[1]=0.0;

    spll_obj->ylf[0]=0.0;
    spll_obj->ylf[1]=0.0;

    spll_obj->fo=0.0;
    spll_obj->fn=(Grid_freq);

    spll_obj->theta[0]=0.0;
    spll_obj->theta[1]=0.0;

    spll_obj->sin=0.0;
    spll_obj->cos=0.0;

    // loop filter coefficients for 20kHz
    spll_obj->lpf_coeff.B0_lf=(166.9743);
    spll_obj->lpf_coeff.B1_lf=(-166.266);
    spll_obj->lpf_coeff.A1_lf=(-1.0);

    spll_obj->delta_T=1.0/20000;// ???
}
void SPLL_1ph_SOGI_IQ_coeff_update(float delta_T, float wn, SPLL_1ph_SOGI_IQ *spll)
{
    float osgx,osgy,temp;
    spll->osg_coeff.osg_k=0.5;
    osgx = 0.0157079;
    spll->osg_coeff.osg_x=osgx;
//    osgy=(float)(wn*delta_T*wn*delta_T);
    osgy= 0.00024674;
    spll->osg_coeff.osg_y=osgy;
    temp=(float)1.0/(osgx+osgy+4.0);
    spll->osg_coeff.osg_b0=(float)osgx*temp;
    spll->osg_coeff.osg_b2=-1.0*spll->osg_coeff.osg_b0;
    spll->osg_coeff.osg_a1=(float)(2.0*(4.0-osgy)*temp);
    spll->osg_coeff.osg_a2=(float)(osgx-osgy-4)*temp;
    spll->osg_coeff.osg_qb0=(0.5*osgy)*temp;
    spll->osg_coeff.osg_qb1=(spll->osg_coeff.osg_qb0*2);
    spll->osg_coeff.osg_qb2=spll->osg_coeff.osg_qb0;
}
void SPLL_1ph_SOGI_IQ_FUNC(SPLL_1ph_SOGI_IQ *spll_obj)
{
    // Update the spll_obj->u[0] with the grid value before calling this routine

    //-------------------------------//
    // Orthogonal Signal Generator   //
    //-------------------------------//
    spll_obj->osg_u[0]= (spll_obj->osg_coeff.osg_b0*(spll_obj->u[0]- spll_obj->u[2]))+ (spll_obj->osg_coeff.osg_a1*spll_obj->osg_u[1])+ (spll_obj->osg_coeff.osg_a2*spll_obj->osg_u[2]);

    spll_obj->osg_u[2]=spll_obj->osg_u[1];
    spll_obj->osg_u[1]=spll_obj->osg_u[0];

    spll_obj->osg_qu[0]= (spll_obj->osg_coeff.osg_qb0*spll_obj->u[0])+ (spll_obj->osg_coeff.osg_qb1*spll_obj->u[1])+ (spll_obj->osg_coeff.osg_qb2*spll_obj->u[2])+ (spll_obj->osg_coeff.osg_a1*spll_obj->osg_qu[1])+ (spll_obj->osg_coeff.osg_a2*spll_obj->osg_qu[2]);

    spll_obj->osg_qu[2]=spll_obj->osg_qu[1];
    spll_obj->osg_qu[1]=spll_obj->osg_qu[0];

    spll_obj->u[2]=spll_obj->u[1];
    spll_obj->u[1]=spll_obj->u[0];

    //-------------------------------------------------------//
    // Park Transform from alpha beta to d-q axis            //
    //-------------------------------------------------------//
    spll_obj->u_Q[0]= (spll_obj->cos*spll_obj->osg_u[0]) + (spll_obj->sin*spll_obj->osg_qu[0]);
    spll_obj->u_D[0]= (spll_obj->cos*spll_obj->osg_qu[0]) - (spll_obj->sin*spll_obj->osg_u[0]);

    //---------------------------------//
    // Loop Filter                     //
    //---------------------------------//
    spll_obj->ylf[0]=spll_obj->ylf[1]+ (spll_obj->lpf_coeff.B0_lf*spll_obj->u_Q[0])+ (spll_obj->lpf_coeff.B1_lf*spll_obj->u_Q[1]);
    //spll_obj->ylf[0]=(spll_obj->ylf[0]>SPLL_SOGI_Q(20.0))?SPLL_SOGI_Q(20.0):spll_obj->ylf[0];
    //spll_obj->ylf[0]=(spll_obj->ylf[0]<SPLL_SOGI_Q(-20.0))?SPLL_SOGI_Q(-20.0):spll_obj->ylf[0];
    spll_obj->ylf[1]=spll_obj->ylf[0];

    spll_obj->u_Q[1]=spll_obj->u_Q[0];
    //spll_obj->u_D[1]=spll_obj->u_D[0];

    //---------------------------------//
    // VCO                             //
    //---------------------------------//
    spll_obj->fo=spll_obj->fn+spll_obj->ylf[0];

    spll_obj->theta[0]=spll_obj->theta[1]+ ( (spll_obj->fo*spll_obj->delta_T)* ((2.0)*(3.1415926)));
    if(spll_obj->theta[0]>(2*3.1415926))
       spll_obj->theta[0]=(0.0);

    spll_obj->theta[1]=spll_obj->theta[0];
    spll_obj->theta[0]=( int )spll_obj->theta[0]*2047 / (2*PI) ;
    spll_obj->sin=sin_tab[(spll_obj->theta[0])]; // sin

    if((spll_obj->theta[0])<=1535)

            spll_obj->cos=sin_tab[(spll_obj->theta[0]) + 512 ];


    else

            spll_obj->cos=sin_tab[spll_obj->theta[0]-1535];


}

//==============================================================================================
void GPIOsetup(void)
{
 EALLOW;
      GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;// I/O
      GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0;// I/O
      GpioCtrlRegs.GPBDIR.bit.GPIO34  = 1;// Output
      GpioCtrlRegs.GPADIR.bit.GPIO31  = 1;// Output

  EDIS;
}

//===================================================================================================
__interrupt void cpu_timer0_isr(void)
{
   CpuTimer0.InterruptCount++;
   count++;
   if (count <=5000)
   {
       GpioDataRegs.GPBDAT.bit.GPIO34 = 0;
       GpioDataRegs.GPADAT.bit.GPIO31 = 1;
   }
   else
   {
       GpioDataRegs.GPBDAT.bit.GPIO34 = 1;
       GpioDataRegs.GPADAT.bit.GPIO31 = 0;
       if (count >=10000)
           count = 0;
   }
//
//
//
//   // dieu chinh tan so : 40 Hz -- > 80Hz
//           // 4096 - > 80Hz
//           //2048-- >40Hz
//           // 1024       20Hz
//           // chuyen dien ap doc ve ung voi tan so
//  doc_adc = AdcaResult0;
//   if (doc_adc <= 1024 )
//       doc_adc = 1024;
//
//  tanso = doc_adc*0.0195; // max = 80hz
//
//   D_alpha= 0.0001 * 2 * 3.14* tanso ;
//
//   if ( goc <= 6.28)
//       goc = goc + D_alpha;
//   else
//    goc = 0;
//   // chuyen sang so nguyen duong : 2pi <--> 2047 ; goc <----> index
//
//   index = goc*2047 / 6.28 ;
//
//
//   sinfunc = sin_tab[index];
//
//  // dieu chinh ty so dieu che
//   m = AdcaResult1;
//   if ( m < 1500 )
//       m = 1500;
//   if ( m > 2000 )
//       m = 2000;
//// Unipolar : 4 XUNG PWM :
//   SINUP = 2500 +  m * sinfunc;
//   SINLOW = 2500 -  m* sinfunc;
//   EPwm1Regs.CMPA.bit.CMPA =  SINUP;
//   EPwm2Regs.CMPA.bit.CMPA =  SINLOW;
//Bipollar : 2 xung PWM ; 1 carrier, 1 sine:
//  EPwm1Regs.CMPA.bit.CMPA =  SINUP; -> KHOA S1,S2/ EPWM 1B -> S3,S4;
   spll.u[0]=(long) index  ;
   SPLL_1ph_SOGI_IQ_FUNC(&spll);

   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void cpu_timer1_isr(void)
{
   CpuTimer1.InterruptCount++;
   D_alpha= 0.00001 * 2 * PI* 60 ;

     if ( goc <= 2*PI)
         goc = goc + D_alpha;
     else
      goc = 0;
     // chuyen sang so nguyen duong : 2pi <--> 2047 ; goc <----> index

     index = goc*2047 /(2*PI) ;
     goc_input = goc*2047 /(2*PI);
     AC_input = sin_tab[goc_input];
// index 0 - > 2047 ; 50hz

   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void cpu_timer2_isr(void)
{
   CpuTimer2.InterruptCount++;
}
void InitEPwm1Example()
{

    EPwm1Regs.TBPRD = 5000;                       // Set timer period; => 10KHz
    EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;           // Phase is 0
    EPwm1Regs.TBCTR = 0x0000;                     // Clear counter

    //
    // Setup TBCLK
    //
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       // Clock ratio to SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;          // Slow just to observe on
                                                   // the scope
    //
    // Setup compare
    //


    //
    // Set actions
    //
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;            // Set PWM2A on Zero
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;

    //
    // Active Low complementary PWMs - setup the deadband
    //
    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm1Regs.DBCTL.bit.IN_MODE = DBA_ALL;
    EPwm1Regs.DBRED.bit.DBRED = 250;
    EPwm1Regs.DBFED.bit.DBFED = 250;
}
void InitEPwm2Example()
{
    EPwm2Regs.TBPRD = 5000;                       // Set timer period; => 10KHz
       EPwm2Regs.TBPHS.bit.TBPHS = 0x0000;           // Phase is 0
       EPwm2Regs.TBCTR = 0x0000;                     // Clear counter
       //
       // Setup TBCLK
       //
       EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up
       EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
       EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       // Clock ratio to SYSCLKOUT
       EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;          // Slow just to observe on                                                     // the scope
       //
       // Setup compare
       //
       //
       // Set actions
       //
       EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;            // Set PWM2A on Zero
       EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;
       //
       // Active Low complementary PWMs - setup the deadband
       //
       EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
       EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
       EPwm2Regs.DBCTL.bit.IN_MODE = DBA_ALL;
       EPwm2Regs.DBRED.bit.DBRED = 250;
       EPwm2Regs.DBFED.bit.DBFED = 250;
   }
void ConfigureADC(void)
{
    EALLOW;
    //write configurations
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    //Set pulse positions to late
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    //power up the ADCs
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    //delay for 1ms to allow ADC time to power up
    DELAY_US(1000);
    EDIS;
}

// SetupADCSoftware - Setup ADC channels and acquisition window
void SetupADCSoftware(void)
{
    //Select the channels to convert and end of conversion flag ADCA
    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert pin A0
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19; //sample window is acqps +
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 19;

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
    EDIS;
}
void ConfigureDAC(void)
{
    EALLOW;
    DacbRegs.DACCTL.bit.DACREFSEL = 1;          // Use ADC references
    DacbRegs.DACCTL.bit.LOADMODE = 0;           // Load on next SYSCLK
                    // Set mid-range
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DAC

    DacaRegs.DACCTL.bit.DACREFSEL = 1;          // Use ADC references
    DacaRegs.DACCTL.bit.LOADMODE = 0;           // Load on next SYSCLK
                    // Set mid-range
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DAC

    DaccRegs.DACCTL.bit.DACREFSEL = 1;          // Use ADC references
    DaccRegs.DACCTL.bit.LOADMODE = 0;           // Load on next SYSCLK
                        // Set mid-range
    DaccRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DAC
    EDIS;
}


