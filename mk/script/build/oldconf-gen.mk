#
#   Date: Jun 22, 2012
# Author: Eldar Abusalimov
#

include mk/script/script-common.mk

HOSTCC  = gcc
HOSTCPP = $(HOSTCC) -E

HOSTCC_IQUOTE_SUPPORT:=

GCC_VERSION := \
	$(word 3,$(shell $(HOSTCC) -v 2>&1 | grep -e "^gcc"))

ifneq ($(GCC_VERSION),)
GCC_VERSION_MAJOR := $(word 1,$(subst ., ,$(GCC_VERSION)))
ifeq ($(GCC_VERSION_MAJOR),4)
HOSTCC_IQUOTE_SUPPORT:=true
endif
endif

CLANG := $(shell $(HOSTCC) -v 2>&1 | grep -e "clang")

ifneq ($(CLANG),)
# Assuming that any llvm clang have -iquote
HOSTCC_IQUOTE_SUPPORT:=true
endif

ifneq ($(HOSTCC_IQUOTE_SUPPORT),)
HOSTCC_CPPFLAGS := -iquote $(CONF_DIR)
else
HOSTCC_CPPFLAGS := -I $(CONF_DIR) -I-
endif

build_conf   := $(CONF_DIR)/build.conf
lds_conf     := $(CONF_DIR)/lds.conf

build_mk     := $(MKGEN_DIR)/build.mk
config_lds_h := $(SRCGEN_DIR)/config.lds.h

all : $(build_mk) $(config_lds_h)

$(build_mk)     : $(build_conf)
$(config_lds_h) : $(lds_conf)

$(build_mk)     : DEFS := __BUILD_MK__
$(config_lds_h) : DEFS := __CONFIG_LDS_H__

$(build_mk) :
	@$(call cmd_notouch_stdout,$@, \
		$(HOSTCPP) -P -undef -nostdinc $(HOSTCC_CPPFLAGS) $(DEFS:%=-D%) \
		-MMD -MP -MT $@ -MF $@.d mk/confmacro.S \
			| $(AWK) '{ gsub("\\$$N","\n"); print }')


$(config_lds_h) :
	@$(call cmd_notouch_stdout,$@, \
		$(HOSTCPP) -P -undef -nostdinc $(HOSTCC_CPPFLAGS) $(DEFS:%=-D%) \
		-MMD -MT $@ -MF $@.d mk/confmacro.S \
			| $(AWK) '{ gsub("\\$$N","\n"); gsub("\\$$","#"); print }'; \
	echo '#define CONFIG_ROOTFS_IMAGE "$(ROOTFS_IMAGE)"') # XXX =/

$(AUTOCONF_DIR)/start_script.inc: $(CONF_DIR)/start_script.inc
	@$(call cmd_notouch_stdout,$@,cat $<)

-include $(addsuffix .d,$(build_mk) $(config_lds_h))

%/. :
	@$(MKDIR) $*

.SECONDEXPANSION :
$(build_mk) $(config_lds_h): mk/script/build/oldconf-gen.mk | $$(@D)/.

