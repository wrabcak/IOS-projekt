CC= gcc

CFLAGS= -std=gnu99 -Wall  -Werror -Wextra -pedantic

LFLAGS=  -lrt -lpthread

NAME=santa

$(NAME): main.c
	$(CC) $(CFLAGS) main.c -g -o $(NAME) $(LFLAGS)
	
clean: 
	rm -f *.o santa santa.out

