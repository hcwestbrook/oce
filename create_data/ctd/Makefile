all: create_ctd.out create_ctdraw.out
%.out: %.R
	R --no-save < $< > $@
clean:
	-rm *~ *.rda *.out
install:
	cp ctd.rda ctdRaw.rda ../../data
	
