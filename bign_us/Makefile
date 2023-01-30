CFLAGS = -O2 -g -fno-omit-frame-pointer -Wall -Idudect -I.
N = 10000
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
	$(Q)$(CC) $(LDFLAGS) -o $@ $^ -lm

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

.PHONY: check clean

check: bntest
	./bntest $(N) > out
	python3 fib_check.py $(N)
	

clean:
	rm -f $(OBJS) $(deps)
	rm -f out *.c.* *.log *.png
	$(RM) bntest 