.PHONY: all clean

all:
	g++ -c main.cpp -g 
	g++ -c epics_db_parse.cpp -g -I/home/awang/EPICS/epics-base/include
	g++ -c epics_dbd_parse.cpp -g 
	g++ -c parse_util.cpp -g
	g++ -L/home/awang/EPICS/epics-base/lib/linux-x86_64 -o main main.o epics_db_parse.o epics_dbd_parse.o parse_util.o -lCom 

clean:
	rm epics_db_parse.o epics_dbd_parse.o main.o parse_util.o main