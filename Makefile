RED=\033[1;31m
GREEN=\033[1;32m
YELLOW=\033[1;33m
BLUE=\033[1;34m
MAGENTA=\033[1;35m
CYAN=\033[1;36m
END=\033[0m

NAME = woody_woodpacker

CC = cc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lssl -lcrypto

ifdef DEBUG
CFLAGS += -g -fsanitize=address
endif

DIR_S = srcs
DIR_I = incs
DIR_O = obj

INCS = -I $(DIR_I)
SRCS_C = $(shell find $(DIR_S) -name '*.c')
SRCS_S = $(shell find $(DIR_S) -name '*.s')
HEADERS = $(shell find $(DIR_I) -name '*.h')
OBJS_C = $(SRCS_C:$(DIR_S)/%.c=$(DIR_O)/%.o)
OBJS_S = $(SRCS_S:$(DIR_S)/%.s=$(DIR_O)/%.o)
OBJS = $(OBJS_C) $(OBJS_S)

all: $(NAME)

$(NAME): $(OBJS) stub
	@echo "Creating $@"
	@$(CC) $(CFLAGS) $(INCS) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Done!"

$(DIR_O)/%.o: $(DIR_S)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

$(DIR_O)/%.o: $(DIR_S)/%.s
	@mkdir -p $(dir $@)
	@echo "Assembling $(notdir $<)"
	@nasm -f elf64 -g -F dwarf $< -o $@

stub: stub.s
	@echo "Assembling stub"
	@nasm -f bin stub.s -o stub
	@echo "Done!"

clean: 
	@echo "Removing objs"
	@rm -rf $(DIR_O)
	@rm -rf stub

fclean: clean
	@echo "Removing $(NAME)"
	@rm -rf $(NAME)

re: fclean all

debug: fclean
	@$(MAKE) all DEBUG=1

debug_stub: debug_stub.s
	@echo "Assembling debug stub"
	@nasm -f bin -g debug_stub.s -o stub

.PHONY: all clean fclean re debug