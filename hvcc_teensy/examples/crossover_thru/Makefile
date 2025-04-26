.PHONY: install clean

NAME=crossover_thru
OPENAUDIO=1
DST=build

$(DST)/teensy/$(NAME).h: _main.pd meta.json
	hvcc $< -n $(NAME) -o $(DST) -G hvcc_teensy -m meta.json
	echo "name=$(NAME)" > $(DST)/teensy/library.properties

meta.json:
	echo '{"external": {"OPENAUDIO":' $(OPENAUDIO) '}}' > $@

install:
	ln -s $$PWD/$(DST)/teensy/ $$HOME/Documents/Arduino/libraries/$(NAME)

clean:
	rm -fr $(DST)
	rm meta.json
