#!/bin/bash

ROOT_PATH=$(pwd)
LIB=$ROOT_PATH/lib/lib
BIN=$ROOT_PATH/httpd
CONF=$ROOT_PATH/conf/server.conf

id=''
proc=$(basename $0)
function usage()
{
	printf "Usage:%s [start(-s) | stop(-t) | restart(-rt)] | status [-ss]\n" "$proc"
}

function server_status()
{
	server_bin=$(basename $BIN)
	id=$(pidof $server_bin)
	if [ ! -z "$id" ];then
		return 0
	else
		return 1
	fi
}
function server_start()
{
	if ! server_status;then
		ip=$(awk -F: '/^IP/{print $NF}' $CONF) #awk工具把conf文件里的内容按行拿出来，找是不是以IP开头的，如果是以IP开头的则以‘：’为分隔符，提取出最后一列
		port=$(awk -F: '/^PORT/{print $NF}' $CONF)
		$BIN $ip $port
		echo "start...done"
	else
		echo "server is already running: $id, please stop first!"
	fi

}
function server_stop()
{
	if  server_status;then
		kill -9 $id
		echo "stop...done"
	else
		echo "server is not running, please start first!"
	fi
}
function server_restart()
{
	if server_status;then
		server_stop
	fi
	server_start
}
function service_status()
{
	if server_status;then
		echo "server is running :$id"
	else
		echo "server is not  running"
	fi
}
if [ $# -eq 0 ];then
	usage
	exit 1
fi

if [ -z $LD_LIBRARY_PATH ];then
	export LD_LIBRARY_PATH=/home/corrine/mydata/code/myhttp/output/lib/lib
fi

case $1 in
	start | -s )
		server_start
	;;
	stop | -t )
		server_stop
	;;
	restart | -rt )
		server_restart
	;;
    status | -ss )
		service_status
	;;
    * )
	usage
	exit 1
	;;



esac
