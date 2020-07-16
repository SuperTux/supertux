.PHONY : all make

# So it will be sequential
all :
	@$(MAKE) --no-print-directory -f targets.mk ensure_directories
	@$(MAKE) --no-print-directory -f targets.mk build_extlibs
	@$(MAKE) --no-print-directory -f targets.mk

clean :
	@$(MAKE) --no-print-directory -f targets.mk clean
