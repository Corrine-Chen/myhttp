bin=httpd
cc=gcc
obj=http.o main.o


FLAGS=#-D_DEBUG_
LDFLAGS=-lpthread #-static
CGI_PATH=sql wwwroot/cgi-bin

.PHONY:all
all:$(bin) cgi

$(bin):$(obj)
	@$(cc) -o $@ $^ $(LDFLAGS)
	@echo "[linking] [$^] to [$@] ..done"

%.o:%.c
	@gcc -c $<  $(FLAGS)
	@echo "[compling] [$^] to [$@] ..done"
cgi:
	@for i in `echo $(CGI_PATH)`;\
		do\
	    	cd $$i;\
	    	make;\
	    	cd -;\
		done


.PHONY:clean
clean:
	@rm -rf $(bin) *.o output
	@for i in `echo $(CGI_PATH)`;\
		do\
    		cd $$i;\
	    	make clean;\
	    	cd -;\
		done
	@echo "clean project ..done"
#用来发布所有的东西；make output就可以发布
.PHONY:output
output:
	@mkdir -p output/wwwroot/cgi-bin
	@cp -rf log output
	@cp -rf conf output
	@cp wwwroot/index.html output/wwwroot
	@cp wwwroot/cgi-bin/math-cgi output/wwwroot/cgi-bin
	@cp sql/insert_cgi output/wwwroot/cgi-bin
	@cp sql/select_cgi output/wwwroot/cgi-bin
	@cp -rf sql/lib output
	@cp httpd output
	@cp plugin/ctl_server.sh output
	@echo "output ..done"

