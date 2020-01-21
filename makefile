OBJS = codegen.o tokenizer.o parser.o compiler.o debug.o
compiler: $(OBJS)
	cc -o compiler $(OBJS)

$(OBJS): compiler.h

.PHONY: clean
clean:
	rm -f *.o