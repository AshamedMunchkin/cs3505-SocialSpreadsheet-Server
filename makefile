all: connection.o done.o server.cc spreadsheet_database.o spreadsheet_server.o
	/usr/bin/g++ connection.o done.o server.cc spreadsheet_database.o spreadsheet_server.o -lmysqlcppconn-static -lmysqlclient -lboost_system -pthread -I./include -L./lib

connection.o: connection.h connection.cc
	/usr/bin/g++ -c connection.cc

done.o: done.h done.cc
	/usr/bin/g++ -c done.cc

spreadsheet_database.o: spreadsheet_database.h spreadsheet_database.cc
	/usr/bin/g++ -c spreadsheet_database.cc -I./include

spreadsheet_server.o: spreadsheet_server.h spreadsheet_server.cc
	/usr/bin/g++ -c spreadsheet_server.cc

clean:
	rm *.o a.out
