include ./flags.mk

OUT_DIR				?= ./out
FINAL_OUT_DIR			?= ../lib
CFG_TEE_CLIENT_LOG_LEVEL 	?= 2

.PHONY: all libteec clean

all: libteec
################################################################################
# Teec configuration
################################################################################
LIB_NAME	:= libobfuse.a

TEEC_SRCS	:= tee_client_api.c \
		   teec_trace.c \
		   obfuse.c \
		   install_agent.c

TEEC_SRC_DIR	:= src
TEEC_OBJ_DIR	:= $(OUT_DIR)
TEEC_OBJS 	:= $(patsubst %.c,$(TEEC_OBJ_DIR)/%.o, $(TEEC_SRCS))
TEEC_INCLUDES 	:= ./include

TEEC_CFLAGS	:= $(addprefix -I, $(TEEC_INCLUDES)) $(CFLAGS) -D_GNU_SOURCE \
		   -DDEBUGLEVEL_$(CFG_TEE_CLIENT_LOG_LEVEL) \
		   -DBINARY_PREFIX=\"TEEC\"

TEEC_LFLAGS     := -lpthread
TEEC_LIBRARY	:= $(OUT_DIR)/$(LIB_NAME)

libteec: $(TEEC_LIBRARY)
	$(CP) $(TEEC_LIBRARY) $(FINAL_OUT_DIR)/$(LIB_NAME)

$(TEEC_LIBRARY): $(TEEC_OBJS)
	$(RM) $@
	$(AR) $@ $(TEEC_OBJS)
	$(RANLIB) $@

$(TEEC_OBJ_DIR)/%.o: ${TEEC_SRC_DIR}/%.c
	@mkdir -p $(TEEC_OBJ_DIR)
	@echo "  CC      $<"
	@$(CC) $(TEEC_CFLAGS) -c $< -o $@

################################################################################
# Cleaning up configuration
################################################################################
clean:
	$(RM) $(OUT_DIR) $(FINAL_OUT_DIR)/$(LIB_NAME)
