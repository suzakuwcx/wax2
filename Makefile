PHONY := __all
__all:

# Do not use make's built-in rules and variables
# (this increases performance and avoids hard-to-debug behaviour)
MAKEFLAGS += -rR
MAKEFLAGS += --no-print-directory

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
READELF		= $(CROSS_COMPILE)readelf
STRIP		= $(CROSS_COMPILE)strip

WAXINCLUDE = include/

CFLAGS:= -I$(WAXINCLUDE)

PHONY += all
__all: all FORCE
	@true # supress the print "Nothing to be done for '__all'"

ifeq ($(obj),)
  all: wax2
  obj-y += wax2.o
else
  all: $(obj)/built-in.o
  include $(obj)/Makefile
  obj-y := $(addprefix $(obj)/,$(obj-y))
  -include $(patsubst %.o,%.d,$(obj-y))
endif

# strip all the '/' suffix for folder object
# depending on folder object is depending on the 'built-in.o' under the folder
obj-y := $(patsubst %/,%,$(obj-y))
obj-y := $(filter %.o,$(obj-y)) $(patsubst %,%/built-in.o,$(filter-out %.o,$(obj-y)))

# 'built-in.o' is depending on all the object 
# written in 'Makefile' under this folder
$(obj)/built-in.o: $(obj-y)
	$(LD) -r -o $@ $^

%/built-in.o: % FORCE
	$(MAKE) obj=$<

# Due to that the 'xxx.h' will become an prerequisites after including '%.d',
# so only take the first prerequisites by '$<'
%.o: %.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

rm-files += wax2 System.map
wax2: $(obj-y)
	$(CC) -o $@ $^ 
	$(NM) -n $@ > System.map

PHONY += clean
clean:
	@rm -vf $(rm-files)
	@find ./ -name "*.[od]" | xargs rm -vf

PHONY += FORCE
FORCE:

.PHONY: $(PHONY)
