#include"http.h"



int startup(const char* _ip,int _port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0 )
	{
		perror("socket");
		return -2;
	}

	int opt =1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(_port);
	local.sin_addr.s_addr = inet_addr(_ip);

	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
	{
		perror("bind");
		return -3;
	}

	if(listen(sock,10) < 0)
	{
		perror("listen");
		return -4;
	}
	return sock;
}


static int get_line(int fd,char* buf,int len)
{
	char c='\0';
	 //len =0;
	int i=0;
	while(c!='\n'&&i<len-1)
	{
		ssize_t s=recv(fd,&c,1,0);
		if(s>0)
		{
			if(c=='\r')
			{//\r,\r\n->\n  \ra
				recv(fd,&c,1,MSG_PEEK);
		    	if(c=='\n')
		    	{
			     	recv(fd,&c,1,0);
		    	}
		    	else
		    	{
			    	c='\n';
		    	}//c->\n
			}
			buf[i++]=c;
		}
	}
	buf[i]=0;
	return i;
}

void print_log(const char* msg,int level)//level
{
	const char*level_msg[]={
		"NOTICE",
		"WARNING",
		"FATAL"
	};
	printf("[%s][%s]\n",msg,level_msg[level]);
}
static	show_404(int fd)
{
	const char *echo_header="HTTP/1.0 404 NOT FONUD\r\n";
	send(fd,echo_header,strlen(echo_header),0);

	const char *type="Content-Type:text/html;charset=ISO-8859-1\r\n";//告诉浏览器说后续发的内容是html文档。
	send(fd,type,strlen(type),0);
	const char* blank_line="\r\n";
	send(fd,blank_line,strlen(blank_line),0);

	const char* msg="<html><h1>Page Not Found!</h1></html>\r\n";
	send(fd,msg,strlen(msg),0);


}
void echo_error(int fd,int error_num)
{
	switch(error_num)
	{
		case 400:
			break;
		case 401:
			break;
		case 403:
			break;
		case 404:
			show_404(fd);
			break;
		case 501:
			break;
		default:
			break;
	}
}
int echo_www(int fd,const char* path,int size)
{
	int new_fd = open(path,O_RDONLY);
	if(new_fd<0)
	{
		print_log("open file error!",FATAL);
	}
	const char *echo_line = "HTTP/1.0 200 OK\r\n";//响应行“协议（HTTP/1.0）  状态码（200）  状态码描述（OK）”
	send(fd,echo_line,strlen(echo_line),0);
	const char *blank_line="\r\n";//空行
	send(fd,blank_line,strlen(blank_line),0);


	if(sendfile(fd,new_fd,NULL,size)<0)//将new_fd中的内容往fd中send，偏移量为NULL，发送的大小为size。
	{
		print_log("send file error!",FATAL);
		return 200;
	}
	close(new_fd);
}


void drop_header(int fd)//将请求报头的消息全部扔掉。
{//一直读一直读知道读到空行。
	int ret =-1;
	char buff[SIZE];
	do
	{
		ret=get_line(fd,buff,sizeof(buff));
	}
	while(ret>0&&strcmp(buff,"\n"));
}
int exe_cgi(int fd,const char *method,\
		const char *path,const char *query_string)
{
	int content_len =-1;
	char METHOD[SIZE/10];//方法：get或者post
	char QUERY_STRING[SIZE];//url中的参数信息
	char CONTENT_LENGTH[SIZE];//为post方法时，url的正文信息。
	if(strcasecmp(method,"GET")==0)
	{
		drop_header(fd);
	}else
	{
		char buff[SIZE];
		int ret = -1;
		do
    	{
	    	ret=get_line(fd,buff,sizeof(buff));
			if(strncasecmp(buff,"Content-Length:",16)==0)
			{
				content_len = atoi(&buff[16]);
			}
    	}
    	while(ret>0&&strcmp(buff,"\n"));
		if(content_len == -1)
		{
			echo_error(fd,401);
			return -1;
		}
	}
	printf("cgi: path:%s\n",path);
	int input[2];
	int output[2];
	if(pipe(input)<0)
	{
		echo_error(fd,401);
		return -2;
	}
	
	if(pipe(output)<0)
	{
		echo_error(fd,401);
		return -3;
	}

	const char *echo_line="HTTP/1.0 200 OK\r\n";//响应一行  “http/1.0表示协议， 200表示状态码   ok表示状态码标志”
	send(fd,echo_line,strlen(echo_line),0);
	const char *type="Content-Type:text/html;charset=ISO-8859-1\r\n";//告诉浏览器说后续发的内容是html文档。
	send(fd,type,strlen(type),0);
	const char *blank_line="\r\n";//空行
	send(fd,blank_line,strlen(blank_line),0);

	pid_t id =fork();
	if(id<0)
	{
		echo_error(fd,501);
		return -2;
	}
	else if(id==0)
	{//child
		close(input[1]);//子进程要往里读数据，故input要读取数据，故关闭‘1’标准输出流。
		close(output[0]);//同上
		sprintf(METHOD,"METHOD=%s",method);//此处用管道导出会比较麻烦，因为不知道内容的大小，所以使用环境变量导出。
		putenv(METHOD);
		if(strcasecmp(method,"GET")==0)
		{
			sprintf(QUERY_STRING,"QUERY_STRING=%s",query_string);
			putenv(QUERY_STRING);//将环境变量(参数)query_string导出到环境变量组里边。
		}
		else
		{
			sprintf(CONTENT_LENGTH,"CONTENT_LENGTH=%d",content_len);
			putenv(CONTENT_LENGTH);//将环境变量content_length导出到环境变量组里边。
		}
		dup2(input[0],0);//将输入重定向。即将标准输入流重定向到input中
		dup2(output[1],1);//同上
		execl(path,path,NULL);//execl只会替换代码和数据，不对文件描述符表进行替换。
		exit(1);
	}
	else
	{
		close(input[0]);//父进程要往里写数据，故input要写数据，所以关闭‘0’标准输入流
		close(output[1]);

		int i=0;
		char c='\0';
		for(;i<content_len;i++)
		{
			recv(fd,&c,1,0);
			write(input[1],&c,1);
		}

		while(1)
		{
			ssize_t s=read(output[0],&c,1);
			if(s>0)
			{
				send(fd,&c,1,0);
			}
			else
			{
				break;
			}
		}
		waitpid(id,NULL,0);
		close(input[1]);//用完以后关闭不用的所有描述符  因为进程是常驻进程不关闭则会导致文件描述符浪费，而子进程在退出时会自动释放，因此不需要关闭。
		close(output[0]);
	}
}

void* handlerRequest(void* arg)
{
	int fd = (int) arg;
	int errno_num=200;
	int cgi=0;
	char* query_string=NULL;//url中的参数信息
#ifdef _DEBUG_
	printf("#########################################");
	int ret =-1;
	char buff[SIZE];
	do
	{
		ret=get_line(fd,buff,sizeof(buff));
		printf("%s",buff);
	}
	while(ret>0&&strcmp(buff,"\n"));
	printf("#########################################");
	//echo_error(fd,404);
	//goto end;
#else
	//1 method, 2 url  ->  GET  POST  /url ->  exist  ->  pri
	char url[SIZE];
	char path[SIZE];
	char buff[SIZE];
	char method[SIZE/10];
	int i,j;

	if(get_line(fd,buff,sizeof(buff))<=0)
	{
		print_log("get request line error",FATAL);
		errno_num =501;
		goto end;
	}

	i=0;j=0;
	while(i<sizeof(method)-1&&j<sizeof(buff)&&!isspace(buff[j]))//可能手动输入的http协议会有空格，此段循环的作用是将空格去除掉
	{
		method[i]=buff[j];
		i++;
		j++;
	}
	method[i]=0;
	//GET       /a/b http/1.0
	while(isspace(buff[j])&&j<sizeof(buff))
	{
		j++;
	}

	i=0;
    while(j < sizeof(buff) && i < sizeof(url)&&!isspace(buff[j]))//将url中的空格都过滤出来
	{
		url[i] = buff[j];
		i++;
		j++;
	}
	url[i]=0;
	
	printf("method:%s ,url:%s\n",method,url);

	if(strcasecmp(method,"GET")&&strcasecmp(method,"POST"))//表示既不是get方法又不是post方法
	{//strcasecmp  可以忽略大小写get GET Get 
		print_log("get method  error",FATAL);
		errno_num =501;
		goto end;
	}

	//默认的方法不是cgi
	if(strcasecmp(method,"POST")==0)
	{
		cgi=1;
	}


	query_string=url;
	while(*query_string!=0)//url中的问号前半部分是所找资源的路径，后半部分是资源参数。
	{
		if(*query_string=='?')
		{
			cgi=1;
			*query_string='\0';

			query_string++;
			break;
		}
		query_string++;
	}
	//此时url只有所找资源的路径,如果是有参数的get方法，则所有参数均在query_string中。
	

	sprintf(path,"wwwroot%s",url);// /  /a/b/c    sprintf是将标准输出输出到path中。//将wwwroot拼到url前边,
//	if(path[strlen(path)-1]=='/')//如果url中只是以根目录‘/’结尾，则返回首页。
//	{
//		strcat(path,"%sindex.html");//拼上首页
//	}

	printf("path:%s\n",path);
	struct stat st;///a/b/c.html
	if(stat(path,&st)<0)//查看资源是否存在。
	{
		print_log("path not fonud!",FATAL);
		errno_num=404;
		goto end;
	}
	else
	{
		if(S_ISDIR(st.st_mode))//判断是否为目录。
		{
	 		strcat(path,"/index.html");
		}
		else//不是目录，但有可能是可执行文件，（比如二进制文件）
		{
			if((st.st_mode & S_IXUSR)||(st.st_mode & S_IXGRP)||(st.st_mode & S_IXOTH))//使用者具有可执行权限 ,所属组具有可执行权限,其他人具有可执行权限
			{
				cgi=1;
			}
		}

		//path是合法的   并且 path是cgi
		if(cgi)//判断cgi是否合法
		{
			exe_cgi(fd,method,path,query_string);
		}
		else//此时只有get方法并且没有参数，而且没有正文。
		{
			drop_header(fd); // 请求行已经读走，接下来此函数是将将请求报头全部丢弃，包括空行。
			errno_num = echo_www(fd,path,st.st_size);//响应根目录。

		}
	}


end:
	echo_error(fd,errno_num);
	close(fd);

#endif

}
