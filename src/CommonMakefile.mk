## Hossein Moein
## September 12 2017

LOCAL_LIB_DIR = ../lib/$(BUILD_PLATFORM)
LOCAL_BIN_DIR = ../bin/$(BUILD_PLATFORM)
LOCAL_OBJ_DIR = ../obj/$(BUILD_PLATFORM)
LOCAL_INCLUDE_DIR = ../include
PROJECT_LIB_DIR = ../../lib/$(BUILD_PLATFORM)
PROJECT_INCLUDE_DIR = ../../include

# -----------------------------------------------------------------------------

SRCS = ../test/thrpool_tester.cc \
       ../test/par_sort_tester.cc \
       ../test/par_accumulate_tester.cc \
       ../test/par_map_reduce.cc \
       ../test/par_partial_sum.cc \
       ../test/par_adjcent_diff.cc \
       ../test/par_dot_product.cc

HEADERS = $(LOCAL_INCLUDE_DIR)/Leopard/SharedQueue.h \
          $(LOCAL_INCLUDE_DIR)/Leopard/SharedQueue.tcc \
          $(LOCAL_INCLUDE_DIR)/Leopard/ThreadPool.h \
          $(LOCAL_INCLUDE_DIR)/Leopard/ThreadPool.tcc

LIB_NAME =
TARGET_LIB =

TARGETS += $(LOCAL_BIN_DIR)/thrpool_tester \
           $(LOCAL_BIN_DIR)/par_sort_tester \
           $(LOCAL_BIN_DIR)/par_accumulate_tester \
           $(LOCAL_BIN_DIR)/par_map_reduce \
           $(LOCAL_BIN_DIR)/par_partial_sum \
           $(LOCAL_BIN_DIR)/par_adjcent_diff \
           $(LOCAL_BIN_DIR)/par_dot_product

# -----------------------------------------------------------------------------

LFLAGS += -Bstatic -L$(LOCAL_LIB_DIR) -L$(PROJECT_LIB_DIR)

LIBS = $(LFLAGS) $(PLATFORM_LIBS)
INCLUDES += -I. -I$(LOCAL_INCLUDE_DIR) -I$(PROJECT_INCLUDE_DIR)
DEFINES = -Wall -D_REENTRANT -DHMTHRP_HAVE_CLOCK_GETTIME \
          -DP_THREADS -D_POSIX_PTHREAD_SEMANTICS -DDMS_$(BUILD_DEFINE)__

# -----------------------------------------------------------------------------

# object file
#
LIB_OBJS =

# -----------------------------------------------------------------------------

# set up C++ suffixes and relationship between .cc and .o files
#
.SUFFIXES: .cc

$(LOCAL_OBJ_DIR)/%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(LOCAL_OBJ_DIR)/%.o: ../benchmarks/%.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(LOCAL_OBJ_DIR)/%.o: ../examples/%.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(LOCAL_OBJ_DIR)/%.o: ../test/%.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.cc :
	$(CXX) $(CXXFLAGS) $< -o $@ -lm $(TLIB) -lg++

# -----------------------------------------------------------------------------

all: PRE_BUILD $(TARGETS)

PRE_BUILD:
	mkdir -p $(LOCAL_LIB_DIR)
	mkdir -p $(LOCAL_BIN_DIR)
	mkdir -p $(LOCAL_OBJ_DIR)
	mkdir -p $(PROJECT_LIB_DIR)
	mkdir -p $(PROJECT_INCLUDE_DIR)/Leopard

$(TARGET_LIB):

THRPOOL_TESTER_OBJ = $(LOCAL_OBJ_DIR)/thrpool_tester.o
$(LOCAL_BIN_DIR)/thrpool_tester: $(THRPOOL_TESTER_OBJ)
	$(CXX) -o $@ $(THRPOOL_TESTER_OBJ) $(LIBS)

PAR_SORT_TESTER_OBJ = $(LOCAL_OBJ_DIR)/par_sort_tester.o
$(LOCAL_BIN_DIR)/par_sort_tester: $(PAR_SORT_TESTER_OBJ)
	$(CXX) -o $@ $(PAR_SORT_TESTER_OBJ) $(LIBS)

PAR_ACCUMULATE_TESTER_OBJ = $(LOCAL_OBJ_DIR)/par_accumulate_tester.o
$(LOCAL_BIN_DIR)/par_accumulate_tester: $(PAR_ACCUMULATE_TESTER_OBJ)
	$(CXX) -o $@ $(PAR_ACCUMULATE_TESTER_OBJ) $(LIBS)

PAR_MAP_REDUCE_OBJ = $(LOCAL_OBJ_DIR)/par_map_reduce.o
$(LOCAL_BIN_DIR)/par_map_reduce: $(PAR_MAP_REDUCE_OBJ)
	$(CXX) -o $@ $(PAR_MAP_REDUCE_OBJ) $(LIBS)

PAR_PARTIAL_SUM_OBJ = $(LOCAL_OBJ_DIR)/par_partial_sum.o
$(LOCAL_BIN_DIR)/par_partial_sum: $(PAR_PARTIAL_SUM_OBJ)
	$(CXX) -o $@ $(PAR_PARTIAL_SUM_OBJ) $(LIBS)

PAR_ADJCENT_DIFF_OBJ = $(LOCAL_OBJ_DIR)/par_adjcent_diff.o
$(LOCAL_BIN_DIR)/par_adjcent_diff: $(PAR_ADJCENT_DIFF_OBJ)
	$(CXX) -o $@ $(PAR_ADJCENT_DIFF_OBJ) $(LIBS)

PAR_DOT_PRODUCT_OBJ = $(LOCAL_OBJ_DIR)/par_dot_product.o
$(LOCAL_BIN_DIR)/par_dot_product: $(PAR_DOT_PRODUCT_OBJ)
	$(CXX) -o $@ $(PAR_DOT_PRODUCT_OBJ) $(LIBS)

# -----------------------------------------------------------------------------

depend:
	makedepend $(CXXFLAGS) -Y $(SRCS)

clobber:
	rm -f $(LIB_OBJS) $(TARGETS) $(THRPOOL_TESTER_OBJ) $(PAR_SORT_TESTER_OBJ) \
          $(PAR_ACCUMULATE_TESTER_OBJ) $(PAR_MAP_REDUCE_OBJ) \
          $(PAR_PARTIAL_SUM_OBJ) $(PAR_ADJCENT_DIFF_OBJ) \
          $(PAR_DOT_PRODUCT_OBJ)

install_lib:
	cp -pf $(TARGET_LIB) $(PROJECT_LIB_DIR)/.

install_hdr:
	cp -rpf $(LOCAL_INCLUDE_DIR)/* $(PROJECT_INCLUDE_DIR)/.

# -----------------------------------------------------------------------------

## Local Variables:
## mode:Makefile
## tab-width:4
## End:
