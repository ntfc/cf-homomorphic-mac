# Makefile for MSc Dissertation
# Nuno Carvalho -- based on Andreas Linz's
# http://tex.stackexchange.com/a/122263
#
# @ supresses command output
#
# conditionals are not allowed to have a starting tab, 
# otherwise they will be sent to the shell
#
# Set this option to false if you wish not to use a BUILD_DIR folder to
# compilation
THESIS=Thesis
LATEXMK=/usr/bin/latexmk
LATEXMKRC=latexmkrc
LATEXMKOPTS=
LANG=en_US
USE_HIDDEN_BUILD=false # this is stupid and useless
BUILD_DIR=.build
OUTPUT_DIR=.
RSYNC_OPTS=--verbose --checksum --recursive --human-readable --progress --exclude=.build \
					 --exclude=.git --exclude=.gitignore --exclude=*.tex.latexmain --exclude=*.log

# if using hidden build dir, copy tex files to build, and then compile
ifeq ($(USE_HIDDEN_BUILD), true)
pdf: copy_sources compile copy_output
# if not using hidden build dir, compile right here
else
pdf: compile
endif

copy_sources:
	@echo "Copying sources to build folder: $(BUILD_DIR)"
	@rsync $(RSYNC_OPTS) . $(BUILD_DIR)

compile: *.tex *.bib
ifeq ($(USE_HIDDEN_BUILD), true)
	cd $(BUILD_DIR) ;\
	$(LATEXMK) -r $(LATEXMKRC) $(LATEXMKOPTS) $(THESIS)
else
	$(LATEXMK) -r $(LATEXMKRC) $(LATEXMKOPTS) $(THESIS)
endif

continuous: *.tex *.bib
ifeq ($(USE_HIDDEN_BUILD), true)
	cd $(BUILD_DIR) ;\
	$(LATEXMK) -r $(LATEXMKRC) -pvc $(LATEXMKOPTS) $(THESIS)
else
	$(LATEXMK) -r $(LATEXMKRC) -pvc $(LATEXMKOPTS) $(THESIS)
endif

copy_output:
ifeq ($(USE_HIDDEN_BUILD), true)
	@echo "Copying generated output to: $(OUTPUT_DIR)"
	@cp -vf $(BUILD_DIR)/$(THESIS).pdf $(OUTPUT_DIR)
endif

# run aspell on all tex files
aspell:
	for i in `find . -iname "*.tex" -not -path "./$(BUILD_DIR)/*" \
			-not -path "./Graphics/*" -not -path "./Definitions/*"`; do \
		aspell -t --dont-tex-check-comments --encoding=utf-8 --lang=$(LANG) check $$i; \
	done

clean:
ifeq ($(USE_HIDDEN_BUILD), true)
	@echo "Cleaning $(BUILD_DIR) folder.."
	@rm -rf $(BUILD_DIR)
else
	@echo "Cleaning folder.."
	$(LATEXMK) -r $(LATEXMKRC) -c
endif
	@rm -rf `biber --cache`

