/*
 * driver.h
 *
 *  Created on: 14 Dec 2010
 *      Author: elvc
 */

#ifndef DRIVER_H_
#define DRIVER_H_
#include <systemc.h>

typedef struct {
#ifdef SCTB	
					sc_signal<sc_logic> *vc_clk;
				    sc_signal<sc_logic> *vc_reset;
				    sc_signal<sc_lv<32> > *vc_addr;
				    sc_signal<sc_lv<32> > *vc_din;
				    sc_signal<sc_lv<32> > *vc_dout;
				    sc_signal<sc_logic> *vc_rd;
				    sc_signal<sc_logic> *vc_wr;
				    sc_signal<sc_logic> *vc_rdack;
				    sc_signal<sc_logic> *vc_wrack;
//#else

#endif				    
} dbgIfT;		

	void crap();
	//void driver2(void);
	void driver2(dbgIfT *);
	void driver1(dbgIfT *);
	//int vtApiInit (galaxyConfigT * galaxyConfig , hostE host , targetE target);

#endif /* DRIVER_H_ */
