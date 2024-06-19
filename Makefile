SHELL = bash

BUTOOL_PATH?=/opt/BUTool
CACTUS_ROOT?=/opt/cactus
EMP_ROOT?=/opt/emp-toolbox
IPBUS_REG_HELER_PATH=/opt/BUTool/include/IPBusIO

UHAL_VER_MAJOR ?= 2
UHAL_VER_MINOR ?= 8

SRCPATH=src
OBJPATH=obj

# set VPATH to look in source path
VPATH = $(SRCPATH)

CXXFILES := $(notdir $(wildcard $(SRCPATH)/*.cxx))
OBJFILES := $(CXXFILES:.cxx=.o)
OBJFILES := $(addprefix $(OBJPATH)/,$(OBJFILES))

CXX?=g++

INCLUDE_PATH += \
							-I$(BUTOOL_PATH)/include 
LIBRARY_PATH += \
							-L$(BUTOOL_PATH)/lib \
							-L$(EMP_ROOT)/logger/lib \
							-L$(EMP_ROOT)/core/lib \
							-L$(CACTUS_ROOT)/lib

ifdef BOOST_INC
INCLUDE_PATH +=-I$(BOOST_INC)
endif
ifdef BOOST_LIB
LIBRARY_PATH +=-L$(BOOST_LIB)
endif

Includes = \
	include \
	pybind11/include \
	$(PYTHON_INCLUDE_PREFIX) \
	$(EMP_ROOT)/logger/include \
	$(EMP_ROOT)/core/include \
	$(CACTUS_ROOT)/include

INCLUDE_PATH += $(addprefix -I,$(Includes))

LIBRARIES =     -Wl,-rpath=$(BUTOOL_PATH)/lib \
		-lToolException	\
		-lBUTool_IPBusIO \
		-lBUTool_IPBusRegHelpers \
		-lBUTool_IPBusStatus \
		-lBUTool_BUTextIO \
		-lboost_regex \
		-lboost_filesystem \
		-lboost_program_options \
		-lcactus_emp_logger \
		-lcactus_emp


CXX_FLAGS = -std=c++11 -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} -Werror -Wno-literal-suffix -Wno-error=clobbered

CXX_FLAGS +=-fno-omit-frame-pointer -Wno-ignored-qualifiers -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs -DUHAL_VER_MAJOR=${UHAL_VER_MAJOR} -DUHAL_VER_MINOR=${UHAL_VER_MINOR} ${COMPILETIME_ROOT} ${FALLTHROUGH_FLAGS}

ifdef MAP_TYPE
CXX_FLAGS += ${MAP_TYPE}
endif

LINK_EXE_FLAGS     = -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES} \
			-lBUTool_Helpers \
			-Wl,-rpath=$(RUNTIME_LDPATH)/lib ${COMPILETIME_ROOT} 



# ------------------------
# IPBUS stuff
# ------------------------
UHAL_LIBRARIES = -lcactus_uhal_log 		\
                 -lcactus_uhal_grammars 	\
                 -lcactus_uhal_uhal 		

UHAL_INCLUDE_PATH = \
	         					-isystem$(CACTUS_ROOT)/include 

UHAL_LIBRARY_PATH = \
							-L$(CACTUS_ROOT)/lib -Wl,-rpath=$(CACTUS_ROOT)/lib 

UHAL_INCLUDE_PATH += -isystem$(UIO_UHAL_PATH)/include
UHAL_LIBRARY_PATH += -Wl,-rpath=$(UIO_UHAL_PATH)/lib


UHAL_CXX_FLAGHS = ${UHAL_INCLUDE_PATH}

UHAL_LIBRARY_FLAGS = ${UHAL_LIBRARY_PATH}




test_stand : $(OBJFILES)
	${CXX} ${LINK_EXE_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} -lBUTool_ApolloSM -lboost_system -lpugixml ${LIBRARIES}  -o $@ $^

clean:
	rm -f test_stand 
	rm -f $(OBJFILES)

obj: 
	mkdir obj

$(OBJPATH)/%.o : %.cxx uhalspeedtest.hh obj 
	${CXX} ${CXX_FLAGS} ${UHAL_CXX_FLAGHS} -c $< -o $@

#-include $(LIBRARY_OBJECT_FILES:.o=.d)

