.PHONY: install clean

NAME=crossover_noise_f32
DST=build

$(DST)/teensy/$(NAME).h: _main.pd meta.json
	hvcc $< -n $(NAME) -o $(DST) -G hvcc_teensy -m meta.json

meta.json:
	echo '{"external": {"OPENAUDIO": 1}}' > $@

install:
	ln -s $$PWD/$(DST)/teensy/ $$HOME/Documents/Arduino/libraries/$(NAME)

clean:
	rm -fr $(DST)
	rm meta.json
