#!/usr/bin/env sh
port=$1
pid_list=`ps -ef | grep agent | grep work_dir | grep $port  | grep -v stop | grep -v grep | awk '{print $2}'`
if [ -n "$pid_list" ];then
   for pid in $pid_list;do
        kill  $pid
        sleep 1 
        kill -0 $pid >/dev/null 2>&1
        ret=$?;if [[ $ret == 0 ]];then kill -9 $pid ;fi
        echo "[INFO] agent with pid $pid closed"
   done
   echo "[INFO] close agent successfully"
else
   echo "[INFO] no agent is existing"
fi  


