# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dlippelt <dlippelt@student.codam.nl>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/27 11:02:13 by dlippelt          #+#    #+#              #
#    Updated: 2025/11/20 11:26:30 by dlippelt         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME =			ircserv
NAME_BOT =		ircbot

CXX =			c++
RM =			rm -rf
MKDIR =			mkdir -p
VALG =			valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes

CXXSTD =		-std=c++17
CXXFLAGS =		-Wall -Werror -Wextra -g3 -DDEBUG
IFLAGS =		-I$(INCDIR) -I$(BOTDIR)/$(INCDIR)
DEPFLAGS =		-MMD -MP
COMPILE =		-c
OUTPUT =		-o

BOTDIR =		./Bot

SRCDIR =		./src
INCDIR =		./inc
OBJDIR =		./obj

SRCS =			$(shell find $(SRCDIR) -name "*.cpp" | sed 's|$(SRCDIR)/||')
SRCS_BOT =		$(shell find $(BOTDIR)/$(SRCDIR) -name "*.cpp" | sed 's|$(BOTDIR)/$(SRCDIR)/||')

OBJS =			$(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))
OBJS_BOT =		$(addprefix $(BOTDIR)/$(OBJDIR)/, $(SRCS_BOT:.cpp=.o))

DEPS =			$(OBJS:.o=.d)
DEPS_BOT =		$(OBJS_BOT:.o=.d)

OBJDIRS =		$(sort $(dir $(OBJS)))
OBJDIRS_BOT =	$(sort $(dir $(OBJS_BOT)))

all: $(NAME) $(NAME_BOT)

$(OBJDIRS):
	@$(MKDIR) $@

$(OBJDIRS_BOT):
	@$(MKDIR) $@

$(NAME): $(OBJDIRS) $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) $(OUTPUT) $@

$(NAME_BOT): $(OBJDIRS_BOT) $(OBJS_BOT)
	@$(CXX) $(CXXFLAGS) $(OBJS_BOT) $(OUTPUT) $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(CXX) $(CXXSTD) $(CXXFLAGS) $(DEPFLAGS) $(IFLAGS) $(COMPILE) $< $(OUTPUT) $@

$(BOTDIR)/$(OBJDIR)/%.o: $(BOTDIR)/$(SRCDIR)/%.cpp
	@$(CXX) $(CXXSTD) $(CXXFLAGS) $(DEPFLAGS) $(IFLAGS) $(COMPILE) $< $(OUTPUT) $@

clean:
	@$(RM) $(OBJDIR)
	@$(RM) $(BOTDIR)/$(OBJDIR)
	@$(RM) *.xml

fclean: clean
	@$(RM) $(NAME)
	@$(RM) $(NAME_BOT)

re: fclean all

valg: $(NAME)
	@$(VALG) ./$(NAME) $(ARGS)

valg_bot: $(NAME_BOT)
	@$(VALG) ./$(NAME_BOT) $(ARGS)

memory: CXXFLAGS += -fsanitize=address
memory: re

-include $(DEPS) $(DEPS_BOT)

.PHONY: all clean fclean re valg valg_bot memory
