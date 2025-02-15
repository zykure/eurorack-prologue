# #############################################################################
# Oscillator Makefile
# #############################################################################

ifeq ($(OS),Windows_NT)
ifeq ($(MSYSTEM), MSYS)
    detected_OS := $(shell uname -s)
else
    detected_OS := Windows
endif
else
    detected_OS := $(shell uname -s)
endif

OSCILLATOR_UC := $(shell echo $(OSCILLATOR) | tr a-z A-Z)
ifeq ($(MANIFEST),)
	MANIFEST := manifest_$(OSCILLATOR).json
endif

PLATFORMDIR = logue-sdk/platform/$(PLATFORM)
PROJECTDIR = .
TOOLSDIR = $(PLATFORMDIR)/../../tools
EXTDIR = $(PLATFORMDIR)/../ext
TEMPLATEDIR = $(PLATFORMDIR)/osc

CMSISDIR = $(EXTDIR)/CMSIS/CMSIS

# #############################################################################
# configure archive utility
# #############################################################################

ZIP = /usr/bin/zip
ZIP_ARGS = -r -m -q -9

ifeq ($(OS),Windows_NT)
ifneq ($(MSYSTEM), MSYS)
  ZIP = $(TOOLSDIR)/zip/bin/zip
endif
endif

# #############################################################################
# Set platform specific options
# #############################################################################

ifeq ($(PLATFORM), prologue)
    PKGSUFFIX = prlgunit
	BLOCKSIZE = 16
endif

ifeq ($(PLATFORM), minilogue-xd)
    PKGSUFFIX = mnlgxdunit
	BLOCKSIZE = 16
endif

ifeq ($(PLATFORM), nutekt-digital)
    PKGSUFFIX = ntkdigunit
	BLOCKSIZE = 64
endif

# #############################################################################
# configure cross compilation
# #############################################################################

MCU = cortex-m4

GCC_TARGET = arm-none-eabi-
GCC_BIN_PATH = $(TOOLSDIR)/gcc/gcc-arm-none-eabi-5_4-2016q3/bin

CC   = $(GCC_BIN_PATH)/$(GCC_TARGET)gcc
CXXC = $(GCC_BIN_PATH)/$(GCC_TARGET)g++
LD   = $(GCC_BIN_PATH)/$(GCC_TARGET)gcc
#LD  = $(GCC_BIN_PATH)/$(GCC_TARGET)g++
CP   = $(GCC_BIN_PATH)/$(GCC_TARGET)objcopy
AS   = $(GCC_BIN_PATH)/$(GCC_TARGET)gcc -x assembler-with-cpp
AR   = $(GCC_BIN_PATH)/$(GCC_TARGET)ar
OD   = $(GCC_BIN_PATH)/$(GCC_TARGET)objdump
SZ   = $(GCC_BIN_PATH)/$(GCC_TARGET)size

HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

LDDIR = $(TEMPLATEDIR)/ld
RULESPATH = $(LDDIR)
LDSCRIPT = $(LDDIR)/userosc.ld
DLIBS = -lm

DADEFS = -DSTM32F401xC -DCORTEX_USE_FPU=TRUE -DARM_MATH_CM4
DDEFS = -DSTM32F401xC -DSTM32F401xx \
    -DCORTEX_USE_FPU=TRUE -DARM_MATH_CM4 -D__FPU_PRESENT -D_USE_MATH_DEFINES -DM_PI=3.14159265358979323846 \
    -DOSC_$(OSCILLATOR_UC) -DOSCILLATOR_TYPE=$(OSCILLATOR) -DBLOCKSIZE=$(BLOCKSIZE)

DDEFS += $(OSC_DDEFS)

COPT = -std=c11 -mstructure-size-boundary=8
CXXOPT = -std=c++11 -fno-rtti -fno-exceptions -fno-non-call-exceptions -fno-use-cxa-atexit -ffunction-sections -fdata-sections -funroll-loops

LDOPT = -Xlinker --just-symbols=$(LDDIR)/osc_api.syms -Wl,--gc-sections

CWARN = -W -Wall -Wextra
CXXWARN =

FPU_OPTS = -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -fcheck-new

OPT = -g -O3 -mlittle-endian
OPT += $(FPU_OPTS)
#OPT += -flto

TOPT = -mthumb -mno-thumb-interwork -DTHUMB_NO_INTERWORKING -DTHUMB_PRESENT


# #############################################################################
# set targets and directories
# #############################################################################

PKGDIR = $(PROJECT)
PKGARCH = $(PROJECT).$(PKGSUFFIX)
PAYLOAD = payload.bin
BUILDDIR = $(PROJECTDIR)/build
OBJDIR = $(BUILDDIR)/obj
LSTDIR = $(BUILDDIR)/lst

ASMSRC = $(UASMSRC)

ASMXSRC = $(UASMXSRC)

UCSRC = logue-sdk/platform/ext/CMSIS/CMSIS/DSP_Lib/Source/SupportFunctions/arm_fill_f32.c
CSRC = $(TEMPLATEDIR)/tpl/_unit.c $(UCSRC)

CXXSRC = $(UCXXSRC)

vpath %.s $(sort $(dir $(ASMSRC)))
vpath %.S $(sort $(dir $(ASMXSRC)))
vpath %.c $(sort $(dir $(CSRC)))
vpath %.cc $(sort $(dir $(CXXSRC)))

ASMOBJS := $(addprefix $(OBJDIR)/, $(notdir $(ASMSRC:.s=.o)))
ASMXOBJS := $(addprefix $(OBJDIR)/, $(notdir $(ASMXSRC:.S=.o)))
COBJS := $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o)))
CXXOBJS := $(addprefix $(OBJDIR)/, $(notdir $(CXXSRC:.cc=.o)))

OBJS := $(ASMXOBJS) $(ASMOBJS) $(COBJS) $(CXXOBJS)

DINCDIR = $(TEMPLATEDIR)/inc \
	  $(TEMPLATEDIR)/inc/api \
	  $(PROJECTDIR)/eurorack \
	  $(PROJECTDIR)/eurorack/stmlib/third_party/STM \
	  $(PROJECTDIR)/eurorack/stmlib/third_party/STM/CMSIS/CM3_f4xx \
	  $(PROJECTDIR)/eurorack/stmlib/third_party/STM/STM32F4xx_StdPeriph_Driver/inc \
          $(PLATFORMDIR)/inc \
	  $(PLATFORMDIR)/inc/dsp \
	  $(PLATFORMDIR)/inc/utils \
          $(CMSISDIR)/Include

INCDIR := $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))

DEFS := $(DDEFS) $(UDEFS)
ADEFS := $(DADEFS) $(UADEFS)

LIBS := $(DLIBS) $(ULIBS)

LIBDIR := $(patsubst %,-I%,$(DLIBDIR) $(ULIBDIR))


# #############################################################################
# compiler flags
# #############################################################################

MCFLAGS   := -mcpu=$(MCU)
ODFLAGS	  = -x --syms
ASFLAGS   = $(MCFLAGS) -g $(TOPT) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.s=.lst)) $(ADEFS)
ASXFLAGS  = $(MCFLAGS) -g $(TOPT) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.S=.lst)) $(ADEFS)
CFLAGS    = $(MCFLAGS) $(TOPT) $(OPT) $(COPT) $(CWARN) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.c=.lst)) $(DEFS)
CXXFLAGS  = $(MCFLAGS) $(TOPT) $(OPT) $(CXXOPT) $(CXXWARN) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.cc=.lst)) $(DEFS)
LDFLAGS   = $(MCFLAGS) $(TOPT) $(OPT) -nostartfiles $(LIBDIR) -Wl,-Map=$(BUILDDIR)/$(PROJECT).map,--cref,--no-warn-mismatch,--library-path=$(RULESPATH),--script=$(LDSCRIPT) $(LDOPT)

OUTFILES := $(BUILDDIR)/$(PROJECT).elf \
	    $(BUILDDIR)/$(PROJECT).hex \
	    $(BUILDDIR)/$(PROJECT).bin \
	    $(BUILDDIR)/$(PROJECT).dmp \
	    $(BUILDDIR)/$(PROJECT).list

###############################################################################
# targets
###############################################################################

all: PRE_ALL $(OBJS) $(OUTFILES) POST_ALL

PRE_ALL:

POST_ALL: package

$(OBJS): | $(BUILDDIR) $(OBJDIR) $(LSTDIR)

$(BUILDDIR):
	@echo Compiler Options
	@echo $(CC) -c $(CFLAGS) -I. $(INCDIR)
	@echo
	@mkdir -p $(BUILDDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(LSTDIR):
	@mkdir -p $(LSTDIR)

$(ASMOBJS) : $(OBJDIR)/%.o : %.s Makefile
	@echo Assembling $(<F)
	@$(AS) -c $(ASFLAGS) -I. $(INCDIR) $< -o $@

$(ASMXOBJS) : $(OBJDIR)/%.o : %.S Makefile
	@echo Assembling $(<F)
	@$(CC) -c $(ASXFLAGS) -I. $(INCDIR) $< -o $@

$(COBJS) : $(OBJDIR)/%.o : %.c Makefile
	@echo Compiling $(<F)
	@$(CC) -c $(CFLAGS) -I. $(INCDIR) $< -o $@

$(CXXOBJS) : $(OBJDIR)/%.o : %.cc Makefile
	@echo Compiling $(<F)
	@$(CXXC) -c $(CXXFLAGS) -I. $(INCDIR) $< -o $@

$(BUILDDIR)/%.elf: $(OBJS) $(LDSCRIPT)
	@echo Linking $@
	@$(LD) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

%.hex: %.elf
	@echo Creating $@
	@$(HEX) $< $@

%.bin: %.elf
	@echo Creating $@
	@$(BIN) $< $@

%.dmp: %.elf
	@echo Creating $@
	@$(OD) $(ODFLAGS) $< > $@
	@echo
	@$(SZ) $<
	@echo

%.list: %.elf
	@echo Creating $@
	@$(OD) -S $< > $@

clean:
	@echo Cleaning
	-rm -fR .dep $(BUILDDIR) $(PKGARCH)
	@echo
	@echo Done

package: $(BUILDDIR)/$(PROJECT).bin
	@echo Packaging to ./$(PKGARCH)
	@mkdir -p $(PKGDIR)
	@cat $(MANIFEST) | jq '.header.platform="$(PLATFORM)"|.header.version="$(VERSION)"' > $(PKGDIR)/manifest.json
	@cp -a $(BUILDDIR)/$(PROJECT).bin $(PKGDIR)/$(PAYLOAD)
	@$(ZIP) $(ZIP_ARGS) $(PROJECT).zip $(PKGDIR)
	@mv $(PROJECT).zip $(PKGARCH)
	@echo
	@echo Done
