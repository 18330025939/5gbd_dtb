#!/bin/bash

pid=$$
ota_count=`ps -ef | grep ota_client.sh | grep -v grep | grep -v $pid | wc -l`
if [ $ota_count -gt 1 ]; then
    echo "already has a ota task,so just exit"
    exit 0
fi

mission_file=/home/cktt/script/ota_mission
log_file=/upgrade/ota_opreation.log
log_file_bak=/upgrade/ota_opreation.log.bak


function create_dir()
{
    if [ ! -d "/upgrade/$1" ]; then
        mkdir -p /upgrade/$1
    fi
}

function create_file()
{
    time=`date "+%Y%m%d%H%M"`
    file="/upgrade/$1/$2_$3_$time.log"
    touch "$file"
    chmod 644 "$file"
    echo "$file"
}


#执行升级操作
function upgrade()
{
    time=`date "+%Y-%m-%d %H:%M:%S"`
    
    if [ ! -d "/upgrade/$1" -o ! -f "/upgrade/$1/"*.tar.gz"" ]; then
        echo "upgrade dir or file not exit, mission abort" >> $2 2>&1
        ota_report "$devaddr" "$1" "$time" "upgrade dir or file not exit, mission abort"
        return 0
    fi
    
    tar -zxf /upgrade/$1/*.tar.gz -C /upgrade/$1/
    res=$(echo $?)
    if [ $res -ne 0 ]; then
        echo "tar failed and error code $res" >> $2 2>&1
    fi
    cd /upgrade/$1/
    chmod 777 /upgrade/$1/run.sh
    bash  /upgrade/$1/run.sh >> $2 2>&1
    res=$?
    if [ $res -eq 0 ];then
        echo "Excute upgrade success"
    else
        echo "Excute upgrade failed $res"
    fi
    
    # time=`date "+%Y-%m-%d %H:%M:%S"`
    # report=`base64 $2 | tr -d '\n\r'`
    # ota_report "$devaddr" "$1" "$time" "$report"
    # res=$?
    # if [ $res -ne 0 ]; then
    #     count=0
    #     while [ $count -lt 5 ]
    #     do
    #        count=`expr $count + 1`
    #        ota_report "$devaddr" "$1" "$time" "$report"
    #        res=$?
    #        if [ $res -eq 0 ]; then
    #            echo "retry send ota report success"
    #            break
    #        fi
    #        sleep 20
    #     done
    # else
    #     echo "send ota report success"
    # fi
    
    # if [ $count -eq 5 -a $res -ne 0 ]; then
    #     touch /home/cktt/script/$1_ota_report
    # fi
}

#检查ota_mission文件是否存在，如果存在执行升级操作
function ota_upgrade_check()
{
    if [ ! -f "$mission_file" ]; then
        echo  "mission file is not exit, upgrade unnecessary"
        return 0;
    fi

    while read line
    do
        id=`echo $line | awk -F ' ' '{print $1}'`
        file=`echo $line | awk -F ' ' '{print $2}'`
        time=`date "+%Y-%m-%d %H:%M:%S"`
        echo "$time start to excute mission $id $file" >> "$file" 2>&1
        upgrade $id "$file"
        echo "end to excute mission $id"
    done < $mission_file
    rm $mission_file
}

function download_ota_file()
{
    echo "start to download ota file $filename" >> $file 2>&1
    res=`curl -o /upgrade/$task_id/$filename -C - $task_url`
    if [ $res -ne 0 ]; then 
        echo "task $task_id download file failed" >> $file 2>&1
        time=`date "+%Y-%m-%d %H:%M:%S"`
        report=`base64 $file | tr -d '\n\r'`
        res=$(ota_report "$devaddr" "$task_id" "$time" "$report")
        if [ $res -ne 0 ]; then
            touch /home/cktt/script/"$task_id"_ota_report
        fi 
        rm /upgrade/$task_id/$filename
        continue
    fi
    echo "download ota file $filename success" >> $file 2>&1
}

# function process_ota_task()
# {
#     $task_id=$1
#     $task_url=$2
#     $task_md5=$3
#     $task_type=$4

#     create_dir $task_id
#     file=`create_file $task_id $devaddr $task_id`
#     time=`date "+%Y-%m-%d %H:%M:%S"`
#     echo "$time upgrade mission $task_id" >> $file 2>&1
    
#     filename=`echo $task_url | awk -F '/' '{print $NF}'`
#     extension=${filename#*.}
#     if [ "$extension" != "tar.gz" ]; then

#         echo "task $task_id file extension is not tar.gz" >> $file 2>&1
#         time=`date "+%Y-%m-%d %H:%M:%S"`
#         report=`base64 $file | tr -d '\n\r'`
#         ota_report "$devaddr" "$task_id" "$time" "$report"e
#         continue
#     fi
    
#     echo "start to check $filename md5" >> $file 2>&1
#     md5_res=$(md5sum /upgrade/$task_id/$filename | cut -d ' ' -f 1)
#     if [ "$task_md5" != "$md5_res" ]; then
#         echo "md5 not match, do not excute task $task_id, ota failed" >> $file 2>&1
#         time=`date "+%Y-%m-%d %H:%M:%S"`
#         report=`base64 $file | tr -d '\n\r'`
#         res=$(ota_report "$devaddr" "$task_id" "$time" "$report")
#         if [ $? -ne 0 ]; then
#             touch /home/cktt/script/"$task_id"_ota_report
#         fi 
#         #rm /upgrade/$task_id/$filename
#         continue
#     fi
#     echo "end to check $filename md5" >> $file 2>&1
#     if [ "$task_type" = "AT_ONCE" ]; then
#         echo "start the ota task now" >> $file 2>&1
#         upgrade $task_id "$file"
#     else
#         if [ ! -f "$mission_file" ]; then
#             touch "$mission_file"
#         fi
#         echo "start the ota task when next reboot" >> $file 2>&1
#         time=`date "+%Y-%m-%d %H:%M:%S"`
#         report=`base64 $file | tr -d '\n\r'`
#         res=$(ota_report "$devaddr" "$task_id" "$time" "$report")
#         if [ $res -ne 0 ]; then
#             touch /home/cktt/script/"$task_id"_ota_report
#         fi 
#         echo $task_id $file >> $mission_file
#     fi
# }

function get_ota_report_info()
{
    $task_id=$1
    $task_url=$2
    $task_md5=$3
    $task_type=$4

    create_dir $task_id
    file=`create_file $task_id $devaddr $task_id`
    time=`date "+%Y-%m-%d %H:%M:%S"`
    echo "$time upgrade mission $task_id" >> $file 2>&1

    filename=`echo $task_url | awk -F '/' '{print $NF}'`
    extension=${filename#*.}
    if [ "$extension" != "tar.gz" ]; then

        echo "task $task_id file extension is not tar.gz" >> $file 2>&1
        time=`date "+%Y-%m-%d %H:%M:%S"`
        report=`base64 $file | tr -d '\n\r'`
        # ota_report "$devaddr" "$task_id" "$time" "$report"e
        # continue
    else
        echo "start to check $filename md5" >> $file 2>&1
        md5_res=$(md5sum /upgrade/$task_id/$filename | cut -d ' ' -f 1)
        if [ "$task_md5" != "$md5_res" ]; then
            echo "md5 not match, do not excute task $task_id, ota failed" >> $file 2>&1
            time=`date "+%Y-%m-%d %H:%M:%S"`
            report=`base64 $file | tr -d '\n\r'`
            # res=$(ota_report "$devaddr" "$task_id" "$time" "$report")
            # if [ $? -ne 0 ]; then
            #     touch /home/cktt/script/"$task_id"_ota_report
            # fi 
            #rm /upgrade/$task_id/$filename
            # continue
        else
            echo "end to check $filename md5" >> $file 2>&1
            if [ "$task_type" = "AT_ONCE" ]; then
                echo "start the ota task now" >> $file 2>&1
                upgrade $task_id "$file"
                time=`date "+%Y-%m-%d %H:%M:%S"`
                report=`base64 $2 | tr -d '\n\r'`
            else
                # if [ ! -f "$mission_file" ]; then
                #     touch "$mission_file"
                # fi
                echo "start the ota task when next reboot" >> $file 2>&1
                time=`date "+%Y-%m-%d %H:%M:%S"`
                report=`base64 $file | tr -d '\n\r'`
                # res=$(ota_report "$devaddr" "$task_id" "$time" "$report")
                # if [ $res -ne 0 ]; then
                #     touch /home/cktt/script/"$task_id"_ota_report
                # fi 
                # echo $task_id $file >> $mission_file
            fi
        fi
    fi

    echo "$devaddr,$task_id,$time,$report,"
}

function get_heartbeat_base_info()
{
    db_res=`psql -U cktt  -d fkz9 -t -c "set search_path=obc; select dev_addr from device_info" | awk -F ' ' '{print $1}'`
    devaddr=`printf "%04d" $db_res`
    cpu_info=`top -b -n 1 | grep "%Cpu(s)" | awk '{print $2}' | awk -F. '{print $1}'`
    read total_disk used_disk <<< $(df -B 1 | awk '/sda1/{print $2, $3}')
    read total_memory used_memory <<< $(free -b | awk 'NR==2{print $2, $3}')
    up_time=`uptime -s`
    cur_time=`date "+%Y-%m-%d %H:%M:%S"`

    echo "$devaddr,$cpu_info,$total_disk,$used_disk,$total_memory,$used_memory,$up_time,$cur_time,"  
}

function get_heartbeat_unit_info()
{
    db_res=`psql -U cktt  -d fkz9 -t -A -c "set search_path=obc; select cpu_soft_ver,cpu_hard_ver,ad_soft_ver,ad_hard_ver,control_soft_ver,control_hard_ver,net_soft_ver,net_hard_ver from device_info"`
    IFS='|' read -r cpu_soft cpu_hard ad_soft ad_hard con_soft con_hard net_soft net_hard <<<"$db_res"

    echo "中央处理单元:$cpu_soft,$cpu_hard;采集单元:$ad_soft,$ad_hard;控制单元:$con_soft,$con_hard;网络单元:$net_soft,$net_hard;"
}

function ota_report_check()
{
    file_list=`ls /home/cktt/script/*_ota_report`
    if [ -z $file_list ]; then 
        echo "no ota report need to send"
        return 0    
    fi

    for line in "$file_list"
    do
        task_id=$(basename $line _ota_report)
        time=`date "+%Y-%m-%d %H:%M:%S"`
        file=`ls -t /upgrade/$task_id/*.log | head -n 1`
        report=`base64 $file | tr -d '\n\r'`
        ota_report "$devaddr" "$task_id" "$time" "$report"
        res=$?
        if [ $res -ne 0 ]; then
            count=0
            while [ $count -lt 5 ]
            do
               count=`expr $count + 1`
               ota_report "$devaddr" "$task_id" "$time" "$report"
               res=$?
               if [ $res -eq 0 ]; then
                   echo "retry send ota report success"
                   rm $line 
                   break
               fi
               sleep 20
            done
        else
            echo "send ota report success"
            rm $line
        fi
    
        if [ $count -eq 5 -a $res -ne 0 ]; then
            echo "retry send ota report failed"
            rm $line
            exit 0
        fi    
    done
}

function get_cloud_ip_port()
{
    db_res=`psql -U cktt  -d fkz9 -t -A -c "set search_path=obc; select ip_addr, port from device_info"`
    IFS='|' read -r cloud_ip_addr cloud_port <<<"$db_res"

    echo "$cloud_ip_addr,$cloud_port,"
}

#ota 客户端main程序，执行心跳及升级检查

db_res=`psql -U cktt  -d fkz9 -t -c "set search_path=obc; select dev_addr from device_info" | awk -F ' ' '{print $1}'`
devaddr=`printf "%04d" $db_res`
boot_time=$(cut -d. -f1 /proc/uptime)

# echo "start to check ota failed report"
# ota_report_check
# echo "end to check ota failed report"

# if [ $boot_time -gt 120 -a $boot_time -lt 240 ]; then
#     echo "ota upgrade check start"
#     ota_upgrade_check
#     echo "ota upgrade check end"
# fi

# echo "process heartbeat start"
# ota_heartbeat
# echo "process heartbeat end"
# #脚本执行完成后备份执行过程
# if [ -f "$log_file"]; then
#     mv "$log_file" "$log_file_bak"
# fi

case $1 in
    base_info)
        get_heartbeat_base_info
        ;;
    unit_info)
        get_heartbeat_unit_info
        ;;
    report_info)
        get_ota_report_info
        ;;
    cloud_info)
        get_cloud_ip_port
        ;;
    *)
        echo "$devaddr,"
        ;;
esac
