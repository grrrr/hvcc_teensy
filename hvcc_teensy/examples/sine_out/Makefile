.PHONY: install clean

NAME=sine_out
DST=build

$(DST)/teensy/$(NAME).h: _main.pd
	hvcc $< -n $(NAME) -o $(DST) -G hvcc_teensy

install:
	ln -s $$PWD/$(DST)/teensy/ $$HOME/Documents/Arduino/libraries/$(NAME)

clean:
	rm -fr $(DST)
