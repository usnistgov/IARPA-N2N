.PHONY: all clean tidy
DOC=testplan

all:
	latexmk -pdf -r latexmkrc $(DOC)

clean:
	latexmk -pdf -c -r latexmkrc $(DOC)

tidy: clean
	$(RM) $(DOC).pdf
