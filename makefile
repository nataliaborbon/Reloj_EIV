MODULES = module/freertos
BOARD = edu-ciaa-nxp
MUJU = ./muju


include $(MUJU)/module/base/makefile

doc:
	doxygen Doxyfile
	