CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude
LDFLAGS = -lz

SRC_DIR = src
OBJ_DIR = obj
TARGET = blur

# Fichiers sources : main.c + tous les .c et .s dans src/
SRCS = main.c $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.s)

# Conversion en fichiers objets dans obj/, en préservant la structure des dossiers
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$(SRCS))) $(patsubst %.s,$(OBJ_DIR)/%.o,$(filter %.s,$(SRCS)))

.PHONY: all clean fclean re

all: $(TARGET)

# Règle de compilation finale
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compilation des .c en .o
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation des .s en .o
$(OBJ_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(TARGET)

re: fclean all