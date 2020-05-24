/*
 * PLL_1PHA_SOGI.h
 *
 *  Created on: 22 thg 5, 2020
 *      Author: HP
 */

#ifndef PLL_1PHA_SOGI_H_
#define PLL_1PHA_SOGI_H_
#define SPLL_SOGI_Q float32
//*********** Structure Definition ********//
typedef struct{
    float32  osg_k;
    float32   osg_x;
    float32  osg_y;
    float32   osg_b0;
    float32   osg_b2;
    float32  osg_a1;
    float32 osg_a2;
    float32   osg_qb0;
    float32   osg_qb1;
    float32   osg_qb2;
}SPLL_1ph_SOGI_IQ_OSG_COEFF;
typedef struct{
    int32   B1_lf;
    int32   B0_lf;
    int32   A1_lf;
}SPLL_1ph_SOGI_IQ_LPF_COEFF;

typedef struct{
    long int    u[3];  // Ac Input
    long int   osg_u[3];
    long int   osg_qu[3];
    long int   u_Q[2];
    long int   u_D[2];
    long int   ylf[2];
    long int   fo; // output frequency of PLL
    long int   fn; //nominal frequency
    long int   theta[2];
    long double   cos;
    long double   sin;
    long double   delta_T;
    SPLL_1ph_SOGI_IQ_OSG_COEFF osg_coeff;
    SPLL_1ph_SOGI_IQ_LPF_COEFF lpf_coeff;
}SPLL_1ph_SOGI_IQ;
//SPLL_1ph_SOGI_IQ.fo



//*********** Function Declarations *******//
void SPLL_1ph_SOGI_IQ_init(int Grid_freq, long DELTA_T, SPLL_1ph_SOGI_IQ *spll);
void SPLL_1ph_SOGI_IQ_coeff_update(float delta_T, float wn, SPLL_1ph_SOGI_IQ *spll);
void SPLL_1ph_SOGI_IQ_FUNC(SPLL_1ph_SOGI_IQ *spll1);



#endif /* PLL_1PHA_SOGI_H_ */
