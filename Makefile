# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: dlippelt <dlippelt@student.codam.nl>         +#+                      #
#                                                    +#+                       #
#    Created: 2025/10/27 11:02:13 by dlippelt      #+#    #+#                  #
#    Updated: 2025/11/05 11:32:09 by spyun         ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

NAME =		ircserv

CXX =		c++
RM =		rm -rf
MKDIR =		mkdir -p
VALG =		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes

CXXSTD =	-std=c++17
CXXFLAGS =	-Wall -Werror -Wextra -g3 -DDEBUG
IFLAGS =	-I$(INCDIR)
DEPFLAGS =	-MMD -MP
COMPILE =	-c
OUTPUT =	-o

SRCDIR =	./src
INCDIR =	./inc
OBJDIR =	./obj

SRCS =		$(shell find $(SRCDIR) -name "*.cpp" | sed 's|$(SRCDIR)/||')

OBJS =		$(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

DEPS =		$(OBJS:.o=.d)

OBJDIRS =	$(sort $(dir $(OBJS)))

all: $(NAME)

$(OBJDIRS):
	@$(MKDIR) $@

$(NAME): $(OBJDIRS) $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) $(OUTPUT) $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(CXX) $(CXXSTD) $(CXXFLAGS) $(DEPFLAGS) $(IFLAGS) $(COMPILE) $< $(OUTPUT) $@

clean:
	@$(RM) $(OBJDIR)
	@$(RM) *.xml

fclean: clean
	@$(RM) $(NAME)

re: fclean all

valg: $(NAME)
	@$(VALG) ./$(NAME) $(ARGS)

memory: CXXFLAGS += -fsanitize=address
memory: re

-include $(DEPS)

.PHONY: all clean fclean re valg memory
