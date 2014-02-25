/*
 * sctb.h
 *
 *  Created on: 13 Dec 2010
 *      Author: elvc
 */

#ifndef SCTB_H_
#define SCTB_H_
#include <systemc.h>
#include <scv.h>
#include "vc_vt_top_esl.h"
#include "driver.h"

// The dbgIft struct is passed (ptr) to the driver code where the signals are assigned
dbgIfT dbgIf;		    

SC_MODULE(sctb)
{
    // Clock period set to 100MHz
  	sc_clock vclk;//Virtual clock;
    sc_event reset_deactivation_event;
	// interface ports
    sc_signal<sc_logic> vc_clk_i;
    sc_signal<sc_logic> vc_reset_i;
    sc_signal<sc_lv<32> > vc_addr_i;
    sc_signal<sc_lv<32> > vc_din_i;
    sc_signal<sc_lv<32> > vc_dout_i;
    sc_signal<sc_logic> vc_rd_i;
    sc_signal<sc_logic> vc_wr_i;
    sc_signal<sc_logic> vc_rdack_i;
    sc_signal<sc_logic> vc_wrack_i;



    void reset_generator();
    void clk_assign();
    void stimulus();


    vc_vt_top_esl* I_VC_VT_TOP_ESL;
    ///////////////////////////////////////////////////    
    // Constructor
	 SC_CTOR(sctb) :
    	vclk("vclk", 10, SC_NS, 0.5, 0.0, SC_NS, false),
      	//vc_clk_i("vc_clk_i", 10, SC_NS, 0.5, 0.0, SC_NS, false),
      	vc_clk_i("vc_clk_i"),
      	vc_reset_i("vc_reset_i"),
      	vc_addr_i("vc_addr_i"),
      	vc_din_i("vc_din_i"),
      	vc_dout_i("vc_dout_i"),
      	vc_rd_i("vc_rd_i"),
      	vc_wr_i("vc_wr_i"),
      	vc_rdack_i("vc_rdack_i"),
      	vc_wrack_i("vc_wrack_i")
 
	{
	  //*************************************************
	  // I_VC_VT_TOP
	  //*************************************************
	  I_VC_VT_TOP_ESL = new vc_vt_top_esl("I_VC_VT_TOP_ESL", "work.vc_vt_top_esl");
	  // Connect ports
	  I_VC_VT_TOP_ESL->vc_clk(vc_clk_i);
	  I_VC_VT_TOP_ESL->vc_reset(vc_reset_i);
	  I_VC_VT_TOP_ESL->vc_addr(vc_addr_i);
	  I_VC_VT_TOP_ESL->vc_din(vc_din_i);
	  I_VC_VT_TOP_ESL->vc_dout(vc_dout_i);
	  I_VC_VT_TOP_ESL->vc_rd(vc_rd_i);
	  I_VC_VT_TOP_ESL->vc_wr(vc_wr_i);
	  I_VC_VT_TOP_ESL->vc_rdack(vc_rdack_i);
	  I_VC_VT_TOP_ESL->vc_wrack(vc_wrack_i);

	  //*************************************************
	  SC_METHOD(reset_generator);
	  sensitive << reset_deactivation_event;
	  
	  //*************************************************
	  SC_METHOD(clk_assign);
	  sensitive << vclk.signal();
	  dont_initialize();

	  //*************************************************
	  SC_THREAD(stimulus);  
	  //*************************************************
	  //SC_THREAD(driver);  
	}

    ~sctb()
      {
        delete I_VC_VT_TOP_ESL; I_VC_VT_TOP_ESL = 0;
      }
};



//*************************************************
// Reset pulse generator.
// The first time it runs at initialization and sets reset low.
// It schedules a wakeup at time 400 ns, and at that time sets
// reset high (inactive).
//
inline void sctb::reset_generator()
{
  static bool first = true;
  if (first)
    {
      first = false;
      vc_reset_i.write(SC_LOGIC_1);
      reset_deactivation_event.notify(400, SC_NS);
    }
  else
    vc_reset_i.write(SC_LOGIC_0);
}


inline void sctb::clk_assign()
{
    sc_logic clock_tmp(vclk.signal().read());
    vc_clk_i.write(clock_tmp);
}
 
extern dbgIfT dbgIfG;
inline void sctb::stimulus(void)
{
	dbgIf.vc_clk=&vc_clk_i;
	dbgIf.vc_reset=&vc_reset_i;
	dbgIf.vc_addr=&vc_addr_i;
	dbgIf.vc_din=&vc_din_i;
	dbgIf.vc_dout= &vc_dout_i;
	dbgIf.vc_rd=&vc_rd_i;
	dbgIf.vc_wr=&vc_wr_i;
	dbgIf.vc_rdack=&vc_rdack_i;
	dbgIf.vc_wrack=&vc_wrack_i;

	dbgIfG.vc_clk=&vc_clk_i;
	dbgIfG.vc_reset=&vc_reset_i;
	dbgIfG.vc_addr=&vc_addr_i;
	dbgIfG.vc_din=&vc_din_i;
	dbgIfG.vc_dout= &vc_dout_i;
	dbgIfG.vc_rd=&vc_rd_i;
	dbgIfG.vc_wr=&vc_wr_i;
	dbgIfG.vc_rdack=&vc_rdack_i;
	dbgIfG.vc_wrack=&vc_wrack_i;

	//while ((dbgIfG.vc_reset->read() == SC_LOGIC_1))   	wait(10, SC_NS); 
	dbgIfG.vc_addr->write("0x0");
	dbgIfG.vc_din->write("0x0");
	dbgIfG.vc_rd->write(SC_LOGIC_0);
	dbgIfG.vc_wr->write(SC_LOGIC_0);
	
	wait (506 , SC_NS);
	cout << "Finished sctb::stimulus(void)\n";
	
	//while (true) {
	/*	
	cout <<"here!\n";
  	wait(10, SC_NS); 		
  	vc_rd_i.write(SC_LOGIC_0);
  	vc_addr_i.write("11110000111100001111000011110000");
  	vc_din_i.write("0xdeadbeef");
  	wait(10, SC_NS); 
  	vc_wr_i.write(SC_LOGIC_0);
  	wait(10, SC_NS); 
  	vc_wr_i.write(SC_LOGIC_1);
  	wait(10, SC_NS); 
  	//wait();
	//driver1( &dbgIf );  
	 * */
	driver2( &dbgIf);  
sc_stop();	
	//	crap();
	//}	
}
#endif /* SCTB_H_ */
