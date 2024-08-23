# Nombre del ejecutable
TARGET = pf1

# Archivos fuente
SRCS = pf1.c

# Archivos objeto
OBJS = $(SRCS:.c=.o)

# Compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -pthread

# Regla por defecto
all: $(TARGET)

# Regla para compilar el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Regla para compilar archivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean