.PHONY: all
all:
	make -C asf/gcc -j4

.PHONY: clean
clean:
	make -C asf/gcc clean
