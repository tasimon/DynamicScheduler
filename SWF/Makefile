OBJS := sched_sim_05.o

EXECUTABLE := sched_sim_05

DEFS := 
CFLAGS :=  -g -O3 -D DEBUG=0 -D IO=1

INCLUDES := 
LDFLAGS := -lm 

CC := cc

%.o: %.c %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(EXECUTABLE)


