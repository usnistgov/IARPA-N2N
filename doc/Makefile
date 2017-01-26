.PHONY: all testplan doxygen clean

BUILD_DIR=build
TESTPLAN=testplan

all: doxygen testplan

testplan:
	$(MAKE) -C $(TESTPLAN)

doxygen:
	doxygen doxygen.conf
	$(MAKE) -C $(BUILD_DIR)/latex

clean:
	$(RM) -r $(BUILD_DIR)
	$(MAKE) $< -C $(TESTPLAN)
	
