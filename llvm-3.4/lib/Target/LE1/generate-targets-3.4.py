for width in [1, 2, 4] :
  issue_width = str(width)

  for alus in range (1, width+1):
    if ((issue_width != 1) & (alus < (width / 2))) :
      continue
      
    num_alus = str(alus)
      
    for muls in [1, 2] :
      if (muls > width) :
        continue
          
      num_muls = str(muls)

      for lsus in [1, 2] :
        if (lsus > width) :
          continue

        num_lsus = str(lsus)

        config = issue_width + "w"
        config += num_alus + "a" + num_muls + "m" + num_lsus + "ls"

        model = "LE1Model" + config

        print "Config = " + config
        print "Model = " + model

        file_content = "def " + model + " : SchedMachineModel {\n"
        file_content += "  let IssueWidth = " + issue_width + ";\n"
        file_content += "  let LoadLatency = 3;\n}\n"

        file_content += "let SchedModel = " + model + " in {\n"
        file_content += "  def : WriteRes<WriteP, [PRED" + num_alus + "]> {\n"
        file_content += "    let NumMicroOps = 0;\n  }"

        file_content += "  def : WriteRes<WriteA, [ALU" + num_alus + "]> {\n"
        file_content += "    let Latency = 2;\n  }"

        file_content += "  def : WriteRes<WriteAI, [ALU" + num_alus + "]> {\n"
        file_content += "    let Latency = 2;\n"
        file_content += "    let NumMicroOps = 2;\n  }"
        
        file_content += "  def : WriteRes<WriteM, [MULT" + num_muls + "]> {\n"
        file_content += "    let Latency = 2;\n  }"

        file_content += "  def : WriteRes<WriteMI, [MULT" + num_muls + "]> {\n"
        file_content += "    let Latency = 2;\n"
        file_content += "    let NumMicroOps = 2;\n  }"

        file_content += "  def : WriteRes<WriteLS, [LSU" + num_lsus + "]> {\n"
        file_content += "    let Latency = 3;\n  }"

        file_content += "  def : WriteRes<WriteLSI, [LSU" + num_lsus + "]> {\n"
        file_content += "    let Latency = 3;\n"
        file_content += "    let NumMicroOps = 2;\n  }"

        file_content += "  def : WriteRes<WriteB, [BRU]> {\n"
        file_content += "    let Latency = 5;\n  }"

        file_content += "  def : WriteRes<WriteBI, [BRU]> {\n"
        file_content += "    let Latency = 5;\n"
        file_content += "    let NumMicroOps = 2;\n  }"

        file_content += "}\n"

        #print "Definition = " + file_content

        output_file = open(("MachineModels/LE1Schedule" + config + ".td"), 'w')
        output_file.write(str(file_content))
        output_file.close()

        output_file = open("LE1Schedule.td", 'a')
        output_file.write("include \"MachineModels/LE1Schedule" + config
        + ".td\"\n")
        output_file.close()

        output_file = open("LE1.td", 'a')
        output_file.write("def : ProcessorModel<\"" + config + "\", " + model + ", []>;\n")
        output_file.close()
