#ifndef _SCGENMOD_vc_vt_top_esl_
#define _SCGENMOD_vc_vt_top_esl_

#include "systemc.h"

struct onedstregt {
    sc_lv<6> addr;
    sc_logic valid;
    sc_lv<4> cluster;
    sc_lv<32> res;
};

inline ostream& operator<<(ostream& os, const onedstregt& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const onedstregt&, const std::string &) {}

inline int operator== (const onedstregt& left, const onedstregt& right) {
    return 0;
}

/************************************/

struct onesrcregt {
    sc_lv<6> addr;
    sc_logic valid;
    sc_lv<32> val;
    int cluster;
};

inline ostream& operator<<(ostream& os, const onesrcregt& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const onesrcregt&, const std::string &) {}

inline int operator== (const onesrcregt& left, const onesrcregt& right) {
    return 0;
}

/************************************/

struct hctracepackett {
    sc_lv<32> pc;
    sc_lv<32> imm;
    onesrcregt rs1;
    onesrcregt rs2;
    onesrcregt rs3;
    onesrcregt rs4;
    onesrcregt rs5;
    onesrcregt rs6;
    onesrcregt rs7;
    onedstregt rd1;
    onedstregt rd2;
};

inline ostream& operator<<(ostream& os, const hctracepackett& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const hctracepackett&, const std::string &) {}

inline int operator== (const hctracepackett& left, const hctracepackett& right) {
    return 0;
}

/************************************/

class vc_vt_top_esl : public sc_foreign_module
{
public:
    sc_in<sc_logic> vc_clk;
    sc_in<sc_logic> vc_reset;
    sc_in<sc_lv<32> > vc_addr;
    sc_in<sc_lv<32> > vc_din;
    sc_out<sc_lv<32> > vc_dout;
    sc_in<sc_logic> vc_rd;
    sc_in<sc_logic> vc_wr;
    sc_out<sc_logic> vc_rdack;
    sc_out<sc_logic> vc_wrack;
    sc_out<hctracepackett> vc_gtraceo[1][4][1];


    vc_vt_top_esl(sc_module_name nm, const char* hdl_name)
     : sc_foreign_module(nm),
       vc_clk("vc_clk"),
       vc_reset("vc_reset"),
       vc_addr("vc_addr"),
       vc_din("vc_din"),
       vc_dout("vc_dout"),
       vc_rd("vc_rd"),
       vc_wr("vc_wr"),
       vc_rdack("vc_rdack"),
       vc_wrack("vc_wrack")
    {
        elaborate_foreign_module(hdl_name);
    }
    ~vc_vt_top_esl()
    {}

};

#endif

