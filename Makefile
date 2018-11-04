NAME = krpsim

NAME_VERIF = krpsim_verif

SRC = main.cpp Lexer.class.cpp Parser.class.cpp DNA.class.cpp

SRC_VERIF = mainVerif.cpp Lexer.class.cpp Parser.class.cpp DNA.class.cpp \
			LexerVerif.class.cpp ParserVerif.class.cpp

OBJ = $(addprefix $(O_DIR)/,$(SRC:.cpp=.o))

OBJ_VERIF = $(addprefix $(O_DIR_VERIF)/,$(SRC_VERIF:.cpp=.o))

O_DIR = ./objs

O_DIR_VERIF = ./objs_verif

S_DIR = ./srcs

CFLAGS = -std=c++11 -stdlib=libc++ -g -Wall -Wextra -Werror -ferror-limit=2 -O3 -I includes

CC = g++

RM = rm -rf

$(NAME): $(OBJ)
	@$(CC) -o $(NAME) $(CFLAGS) $(OBJ)

$(NAME_VERIF): $(OBJ_VERIF)
	@$(CC) -o $(NAME_VERIF) $(CFLAGS) $(OBJ_VERIF)

$(O_DIR)/%.o: $(S_DIR)/%.cpp
	@mkdir -p $(O_DIR)
	@$(CC) $(CFLAGS) -o $@ -c $<

$(O_DIR_VERIF)/%.o: $(S_DIR)/%.cpp
	@mkdir -p $(O_DIR_VERIF)
	@$(CC) $(CFLAGS) -o $@ -c $<

all: $(NAME)

verif: $(NAME_VERIF)

clean:
	@$(RM) $(OBJ)
	@$(RM) $(OBJ_VERIF)
	@$(RM) $(O_DIR)
	@$(RM) $(O_DIR_VERIF)

fclean: clean
	@$(RM) $(NAME) $(NAME_VERIF)

re: fclean all verif

.PHONY: all clean fclean re
