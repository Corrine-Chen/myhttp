ROOT_PATH=$(shell pwd)
INCLUDE=$(ROOT_PATH)/lib/include
LIB=$(ROOT_PATH)/lib/lib
LDFLAGS=-lmysqlclient     # -lpthread -static


cc=g++

.PHONY:all
all:insert_cgi select_cgi

insert_cgi:insert_cgi.o sql.o
	$(cc) -o $@ $^ -L$(LIB) $(LDFLAGS)

select_cgi:select_cgi.o sql.o
	$(cc) -o $@ $^ -L$(LIB) $(LDFLAGS)


%.o:%.cpp
	$(cc) -c $< -I$(INCLUDE)

.PHONY:clean
clean:
	rm -f *.o insert_cgi select_cgi


