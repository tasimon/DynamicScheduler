OBJS := sched_sim_04.o

EXECUTABLE := sched_sim_04

DEFS := 
CFLAGS :=  -O0 -D DEBUG=0

INCLUDES := 
LDFLAGS := -lm 

CC := cc

%.o: %.c %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(EXECUTABLE)


