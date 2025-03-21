DIRS = json

.PHONY: all clean

all:
	@for dirs in $(DIRS); do \
		if [ -d $$dirs ]; then\
			make -C $$dirs || exit 1;\
		fi \
	done
	
clean:
	@for dirs in $(DIRS); do \
		if [ -d $$dirs ]; then\
			make -C $$dirs clean || exit 1;\
		fi \
	done