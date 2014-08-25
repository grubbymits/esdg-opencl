
path_to_driver = "/home/sam/src/esdg-opencl/"
path_to_llvm_backend = path_to_driver + "llvm-3.2/lib/Target/LE1/"
path_to_simulator_models = path_to_driver + "install-dir/machines/"

final_device_array = ""
driver_devices = ""
total_devices = 0

sim_output = True;
compiler_output = False;
mod_driver = False;

for context in [1, 2, 4, 8] :
    
    for width in [1, 2, 4]:
        issue_width = str(width)
    
        for alus in range(1, width+1):
            if ((issue_width != 1) & (alus < (width / 2))):
                continue

            num_alus = str(alus)

            for muls in [1, 2] :
            # for muls in range(1, width+1):
                if (muls > width) :
                    continue
                
                num_muls = str(muls)

                for lsus in [1, 2] :
                # for lsus in range(1, width+1):
                    if (lsus > width) :
                        continue
                    
                    num_lsus = str(lsus)
                
                    for banks in [1, 2, 4, 8]:
                        if (banks > (lsus * context)):
                            continue
                
                        num_banks = str(banks)
                        #print "Num Banks = " + num_banks

                        # Define string simulator model
                        simulator_target = """
                        <galaxy>
                            <systems>1</systems>
                            <type>homogeneous</type>
                            <system>
                                <contexts>""" + str(context) + """</contexts>
                                <SCALARSYS_PRESENT>1</SCALARSYS_PRESENT>
                                <PERIPH_PRESENT>0</PERIPH_PRESENT>
                                <DARCH>DRAM_SHARED</DARCH>
                                <DRAM_BLK_SIZE>16</DRAM_BLK_SIZE>
                                <DRAM_SIZE>0x1000</DRAM_SIZE>
                                <STACK_SIZE>0x100</STACK_SIZE> <!-- 8 KiB -->
                                <DRAM_BANKS>""" + num_banks + """</DRAM_BANKS>
                                <context>
                                    <ISSUE_WIDTH_MAX>""" + issue_width + """</ISSUE_WIDTH_MAX>
                                    <ISA_PRSPCTV>VT32PP</ISA_PRSPCTV>
                                    <IARCH>IFE_SIMPLE_IRAM_PRIV</IARCH>
                                    <CLUST_TEMPL>1</CLUST_TEMPL>
                                    <HYPERCONTEXTS>1</HYPERCONTEXTS>
                                    <IFETCH_WIDTH>""" + issue_width + """</IFETCH_WIDTH>
                                    <IRAM_SIZE>0x100</IRAM_SIZE>

                                    <clusterTemplate>
                                        <name>Cluster0</name>
                                        <SCORE_PRESENT>1</SCORE_PRESENT>
                                        <VCORE_PRESENT>0</VCORE_PRESENT>
                                        <FPCORE_PRESENT>0</FPCORE_PRESENT>
                                        <CCORE_PRESENT>0</CCORE_PRESENT>

                                            <INSTANTIATE>1</INSTANTIATE>
                                            <INSTANCES>1</INSTANCES>
                                            <ISSUE_WIDTH>""" + issue_width + """</ISSUE_WIDTH>

                                            <S_GPR_FILE_SIZE>64</S_GPR_FILE_SIZE>
                                            <S_FPR_FILE_SIZE>0</S_FPR_FILE_SIZE>
                                            <S_VR_FILE_SIZE>0</S_VR_FILE_SIZE>
                                            <S_PR_FILE_SIZE>8</S_PR_FILE_SIZE>

                                            <IALUS>""" + num_alus + """</IALUS>
                                            <IMULTS>""" + num_muls + """</IMULTS>
                                            <LSU_CHANNELS>""" + num_lsus + """</LSU_CHANNELS>
                                            <BRUS>1</BRUS>
                                    </clusterTemplate>

                                    <hypercontext>
                                        <name>HyperContext0</name>
                                        <cluster>0_0</cluster>
                                    </hypercontext>
                                </context>
                            </system>
                        </galaxy>"""

                        if (mod_driver) :
                            driver_devices += "  Coal::LE1Device( " + str(context) + ",      " + issue_width + ",        " + num_alus + ",   "
                            driver_devices += num_muls + ",     " + num_lsus + ",   " + num_banks + ")"
                            total_devices += 1

                            #print "Context = " + str(context) + ", width = " + issue_width + ", alus = " + num_alus + ", muls = " + num_muls
                            #print "lsus = " + num_lsus + ", banks = " + num_banks
                            if ((context == 8) & (width == 4) & (alus == width) & (muls == 2) & (lsus == 2) & (banks == 8)):
                                driver_devices += "     // " + str(total_devices) + "\n};"
                                final_device_array = "static Coal::LE1Device LE1Devices[" + str(total_devices) + "] = {\n"
                                final_device_array += "//                cores,  width,  alus, muls, lsus, banks\n"
                                
                                final_device_array += driver_devices
                                output_file = open("devices.h", 'w')
                                output_file.write(str(final_device_array))
                                output_file.close()
                            else :
                                driver_devices += ",    // " + str(total_devices) + "\n"

                        config_name = issue_width + "w_" + num_alus + "a_" + num_muls + "m_" + num_lsus + "ls"
                    
                        # Write XML file for simulator
                        if (sim_output) : 
                            simulator_target_filename = path_to_simulator_models + str(context) + "-core/"
                            simulator_target_filename += config_name + "_" + num_banks + "b.xml"
                            output_file = open(simulator_target_filename, 'w')
                            output_file.write(str(simulator_target))
                            output_file.close()
                            #print "Create new simulator model: " + simulator_target_filename

                        if not compiler_output :
                            continue
                    
                        # Don't create a compiler target for scalar device
                        if ((width == 1) | (banks != 1) | (context != 1)):
                            continue
                    
                        # Define LLVM schedule target
                        compiler_target = """// --------------  LE1 Itinerary for """ + config_name + " -------------- // \n\n"

                        # Define functional units
                        for compiler_alu in range(alus) :
                            alu = str(compiler_alu)
                            #print "def ALU"
                            compiler_target += "def ALU_" + alu + "_" + config_name + "  : FuncUnit;\n"

                        for compiler_mul in range(muls) :
                            mul = str(compiler_mul)
                            #print "def MUL"
                            compiler_target += "def MUL_" + mul + "_" + config_name + "  : FuncUnit;\n"

                        for compiler_lsu in range(lsus) :
                            #print "def LSU"
                            lsu = str(compiler_lsu)
                            compiler_target += "def LSU_" + lsu + "_" + config_name + "  : FuncUnit;\n"

                        compiler_target += "def BRU_0_" + config_name + "   : FuncUnit;\n\n"

                        # Define the itineraries
                        compiler_target += "def LE1" + config_name + "Itineraries : ProcessorItineraries< [\n"
                        for compiler_alu in range(alus) :
                            alu = str(compiler_alu)
                            compiler_target += "    ALU_" + alu + "_" + config_name + ", \n"

                        for compiler_mul in range(muls) :
                            mul = str(compiler_mul)
                            compiler_target += "    MUL_" + mul + "_" + config_name + ", \n"

                        for compiler_lsu in range(lsus) :
                            lsu = str(compiler_lsu)
                            compiler_target += "    LSU_" + lsu + "_" + config_name + ", \n"

                        compiler_target += "    BRU_0_" + config_name + " ], [/*ByPass*/], [\n\n"
                    
                        # Define IIAlu
                        compiler_target += "    InstrItinData<IIAlu, [InstrStage<1, [\n     "
                        for compiler_alu in range(alus) :
                            alu = str(compiler_alu)
                            compiler_target += "ALU_" + alu + "_" + config_name
                        
                            if (compiler_alu != alus-1) :
                                compiler_target += ", "
                            else :
                                compiler_target += "]>], [2, 1]>,\n\n"

                        # Define IIMul
                        compiler_target += "    InstrItinData<IIMul, [InstrStage<1, [\n     "
                        for compiler_mul in range(muls) :
                            mul = str(compiler_mul)
                            compiler_target += "MUL_" + mul + "_" + config_name
                        
                            if (compiler_mul != muls-1) :
                                compiler_target += ", "
                            else :
                                compiler_target += "]>], [2, 1]>,\n\n"

                        # Define IILoadStore
                        compiler_target += "    InstrItinData<IILoadStore, [InstrStage<1, [\n   "
                        for compiler_lsu in range(lsus) :
                            lsu = str(compiler_lsu)
                            compiler_target += "LSU_" + lsu + "_" + config_name
                        
                            if (compiler_lsu != lsus-1) :
                                compiler_target += ", "
                            else :
                                compiler_target += "]>], [3, 1]>,\n\n"

                        # Define IIBranch
                        compiler_target += "    InstrItinData<IIBranch, [InstrStage<1, [BRU_0_" + config_name + "]>], [6, 1]> \n]>;\n\n"

                        # Define Model
                        compiler_target += "def LE1Model" + config_name + " : SchedMachineModel {\n"
                        compiler_target += "    let IssueWidth = " + issue_width + ";\n"
                        compiler_target += "    let Itineraries = LE1" + config_name + "Itineraries;\n}\n"

                        # Write LLVM machine model
                        compiler_target_filename = path_to_llvm_backend + "MachineModels/LE1" + config_name + ".td"
                        output_file = open(compiler_target_filename, 'w')
                        output_file.write(str(compiler_target))
                        output_file.close()
                    
                        # Update main LE1.td schedule file to define the configs
                        output_file = open((path_to_llvm_backend + "LE1.td"), 'a')
                        output_file.write("def : Processor< \"" + config_name + "\", LE1" + config_name + "Itineraries, []>;\n")
                        output_file.close()

                        # Update main LE1Schedule to include the new schedules
                        output_file = open((path_to_llvm_backend + "LE1Schedule.td"), 'a')
                        output_file.write("include \"MachineModels/LE1" + config_name + ".td\"\n")
                        output_file.close()

                        #print "Created new LLVM machine model: " + config_name + " at " + path_to_llvm_backend + "MachineModels/"

