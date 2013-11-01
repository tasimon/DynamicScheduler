#Knapsack makefile
CC := cc

DEFS := 
CFLAGS :=  -O3 -D DEBUG=0


INCLUDES := 
LDFLAGS := -lm 


all:knapsack knapsack2

knapsack: knapsack.c
	$(CC) $(CFLAGS) $(INCLUDES) knapsack.c -o $@ $(LDFLAGS)

knapsack2: knapsack2.c
	$(CC) $(CFLAGS) $(INCLUDES) knapsack2.c -o $@ $(LDFLAGS)

clean:
	rm -f knapsack knapsack2


