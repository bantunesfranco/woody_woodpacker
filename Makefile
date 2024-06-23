RED=\033[1;31m
GREEN=\033[1;32m
YELLOW=\033[1;33m
BLUE=\033[1;34m
MAGENTA=\033[1;35m
CYAN=\033[1;36m
END=\033[0m

NAME = woody_woodpacker

CC = gcc
CFLAGS = -Wall -Wextra -Werror

ifndef DEBUG
CFLAGS += -g -fsanitize=address
endif

DIR_S = srcs
DIR_I = incs
DIR_O = obj

INCS = -I $(DIR_I)
SRCS = $(shell find $(DIR_S) -name '*.c')
HEADERS = $(shell find $(DIR_I) -name '*.h')
OBJS = $(patsubst $(DIR_S)/%.s,$(DIR_O)/%.o,$(SRCS))

$(NAME): $(OBJS)
	@echo "$(MAGENTA)Creating $@$(END)"
	@$(CC) $(CFLAGS) $(INCS) $(OBJS) -o $@
	@echo "$(GREEN)Done!$(END)"

$(DIR_O)/%.o: $(DIR_S)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	@echo "$(BLUE)Compiling $(notdir $<)$(END)"
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

all: $(NAME)

clean: 
	@echo "$(RED)Removing objs$(END)"
	@rm -rf $(DIR_O)

fclean: clean
	@echo "$(RED)Removing $(NAME)$(END)"
	@rm -rf $(NAME)

re: fclean all

debug:
	@$(MAKE) all DEBUG=1

.PHONY: all clean fclean re debug