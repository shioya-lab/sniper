include ../../common/Makefile.common

QEMU_FRONTEND_CFLAGS=-fPIC -fvisibility=hidden $(shell pkg-config --cflags glib-2.0)
CFLAGS+=$(QEMU_FRONTEND_CFLAGS)
CPPFLAGS+=-I.. -I$(RV8_HOME)/src/asm
CXXFLAGS+=$(QEMU_FRONTEND_CFLAGS)
LD_FLAGS+=-shared

libqemu-frontend.so: bbv_count.o plugin.o qemu_frontend.o
	$(_MSG) '[LD    ]' $(subst $(shell readlink -f $(SIM_ROOT))/,,$(shell readlink -f $@))
	$(_CMD) $(CXX) $(LD_FLAGS) -o $@ $^ $(LD_LIBS) $(OPT_CFLAGS)

bbv_count.d: ../bbv_count.cc
	$(_MSG) '[DEP   ]' $(subst $(shell readlink -f $(SIM_ROOT))/,,$(shell readlink -f $@))
	$(_CMD) $(CXX) -MM -MG $(CPPFLAGS) $(CXXFLAGS) $< | sed -n "H;$$ {g;s@.*:\(.*\)@$*.o $@: \$$\(wildcard\1\)@;p}" >$@

bbv_count.o: ../bbv_count.cc
	$(_MSG) '[CXX   ]' $(subst $(shell readlink -f $(SIM_ROOT))/,,$(shell readlink -f $@))
	$(_CMD) $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	-$(RM) libqemu-frontend.so bbv_count.o plugin.o qemu_frontend.o

ifneq ($(CLEAN),clean)
-include bbv_count.d plugin.d qemu_frontend.d
endif
