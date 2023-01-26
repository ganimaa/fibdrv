CFLAGS = -O1 -g -Wall -Idudect -I.
N = 1000
# Control the build verbosity
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

OBJS := bign.o bn_test.o

deps := $(OBJS:%.o=.%.o.d)

bntest: $(OBJS)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

.PHONY: check clean

check: bntest
	./bntest $(N) > out
	python3 fib_check.py $(N)
	

clean:
	rm -f $(OBJS) $(deps)
	rm -f out
	$(RM) bntest