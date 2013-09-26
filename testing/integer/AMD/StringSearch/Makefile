DEPTH = ../../../../..

include $(DEPTH)/make/openclsdkdefs.mk 

####
#
#  Targets
#
####

OPENCL			= 1
SAMPLE_EXE		= 1
EXE_TARGET 		= StringSearch
EXE_TARGET_INSTALL   	= StringSearch

####
#
#  C/CPP files
#
####

FILES 	= StringSearch
CLFILES	= StringSearch_Kernels.cl
IMAGES  = StringSearch_Input.txt

LLIBS  	+= SDKUtil

include $(DEPTH)/make/openclsdkrules.mk 

