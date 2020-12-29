all: orm/easydb

orm/easydb:
	ln -s ../../asst1/easydb $@
	
.PHONY: clean	
clean:
	rm -f orm/easydb
	rm -f tester*
	rm -rf __pycache__
	rm -rf orm/__pycache__
