
GREEN	=	\033[38;5;154m
RED		=	\033[38;5;197m
PURPLE	=	\033[38;5;171m
BLUE	=	\033[38;5;14m
NC		=	\033[0m
YELLOW	=	\033[38;5;190m

NAME = webserv

FLAGS = -std=c++98 -Wall -Wextra -Werror -fsanitize=address
DEPFLAGS = -MMD -MP

FILES_SRC = Input.cpp Utils.cpp Server.cpp Request.cpp RequestUtils.cpp \
			Response.cpp Location.cpp StartServer.cpp StartServerUtils.cpp \
			Signals.cpp Cgi.cpp ResponseGen.cpp
MAIN = main.cpp
DIR_OBJ = .tmp/

DIR_SRC = src/

SRC = $(addprefix $(DIR_SRC), $(FILES_SRC))

OBJ_SRC = $(addprefix $(DIR_OBJ), $(SRC:.cpp=.o))
OBJ_MAIN = $(addprefix $(DIR_OBJ), $(MAIN:.cpp=.o))

OBJ = $(OBJ_SRC) $(OBJ_MAIN)
DEP = $(OBJ:.o=.d)

all: $(DIR_OBJ) $(NAME)

d: all
	@echo "$(GREEN)Launch Webserv!$(NC)"
	@echo "$(GREEN)========================================$(NC)\n"
	@./webserv ./conf/test.conf
	@echo "$(GREEN)========================================$(NC)"

$(NAME): $(OBJ)
	@c++ $(FLAGS) $(DEPFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)Webserv compiled!$(NC)"

$(DIR_OBJ):
	@mkdir -p $(DIR_OBJ)

$(DIR_OBJ)%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	@c++ $(FLAGS) -I./inc $(DEPFLAGS) -c $< -o $@
	@echo "$(YELLOW)Compiling $<$(NC)"

clean:
	@rm -fr $(DIR_OBJ)
	@echo "$(RED)Deleted dir obj$(NC)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)Deleted exec$(NC)"

re: fclean all

.PHONY: all clean fclean re

-include $(DEP)