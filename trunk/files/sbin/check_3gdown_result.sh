#!/bin/bash

restart_lte_modules() {
        cdma_off.sh 2>/dev/null
        sleep 1
        cdma_on.sh 2>/dev/null
        sleep 5
}

get_hdrcsq() {
        local k=0
        while [ $k -lt 5 ]
        do
                local signal=$(at_ctrl at^hdrcsq |awk -F ':' '/HDRCSQ/{print $2}' |sed -n '$p' 2>/dev/null)
                if [[ ! -z ${signal} ]];then
                        break
                else
                        k=$(($k+1))
                        sleep 1
                fi
        done
        if [[ -z ${signal} ]];then
                signal="0"
        fi
        echo ${signal}
}

get_sim_sysinfo() {
        local sim_sysinfo=$(at_ctrl at^sysinfo |grep -r 'SYSINFO' |sed -n '$p' |awk -F ':' '{print $2}')
        local srv_status=$(echo ${sim_sysinfo} |awk -F ',' '{print $1}')
        local srv_domain=$(echo ${sim_sysinfo} |awk -F ',' '{print $2}')
        local sys_mode=$(echo ${sim_sysinfo} |awk -F ',' '{print $4}')
        local simst=$(echo ${sim_sysinfo} |awk -F ',' '{print $5}')

        local n=0
        while [ $n -lt 5 ]
        do
                if [[ ! -z ${srv_status} && ! -z ${srv_domain} && ! -z ${sys_mode} && ! -z ${simst} ]];then
                        break
                else
                        n=$(($n+1))
                        sleep 1
                fi
        done
        if [[ -z ${srv_status} ]];then
                srv_status=0
        fi
        if [[ -z ${srv_domain} ]];then
                srv_domain=0
        fi
        if [[ -z ${sys_mode} ]];then
                sys_mode=0
        fi
        if [[ -z ${simst} ]];then
                simst=1
        fi
        echo ${srv_status},${srv_domain},${sys_mode},${simst}
}

write_sim_flag() {
        if [[ ${sim_flag} -eq 0 ]];then
                sim_flag=1
        fi
}

check_lte_modules() {
        local model_3g=$(cat /tmp/3g_model)
        local ttyUSB_sum=$(ls /dev/ttyUSB* |wc -w)

        case ${model_3g} in
                "MC271X")
                        if [[ ${ttyUSB_sum} -ne 4 ]];then
                                restart_lte_modules
                                write_sim_flag
                        fi
                        ;;
                "SIM6320C")
                        if [[ ${ttyUSB_sum} -ne 5 ]];then
                                restart_lte_modules
                                write_sim_flag
                        fi
                        ;;
                "DM111")
                        if [[ ${ttyUSB_sum} -ne 2 ]];then
                                restart_lte_modules
                                write_sim_flag
                        fi
                        ;;
                *)
                        logger -t $0 "Model=${model} Not Support"
                        ;;
        esac
}

check_prefmodel() {
        local model_3g=$(cat /tmp/3g_model)
        local i=0
        while [ $i -lt 3 ]
        do
                local current_prefmodel=$(at_ctrl at^prefmode? |awk -F ':' '/PREFMODE/{print $2}' |sed -n '$p' 2>/dev/null)
                if [[ -z "${current_prefmodel}" ]];then
                        sleep 1
                        i=$((i+1))
                else
                        break
                fi
        done

        if [[ "${model_3g}" == "MC271X" ]];then
                if [[ "${current_prefmodel}" != "4" ]];then
                        at_ctrl at^prefmode=4 >/dev/null 2>&1
                fi
        fi
}

check_signal_3g() {
        local signal1=$(get_hdrcsq)
        sleep 2
        local signal2=$(get_hdrcsq)
        if [[ ${signal1} -le 20 && ${signal2} -le 20 ]];then
                echo "***** the signal is weak *****" >>${log_file}
                write_sim_flag
        fi
        echo signal1=${signal1} >>${log_file}
        echo signal2=${signal2} >>${log_file}
}

check_sim_state() {
        local SRV_status=$(echo $(get_sim_sysinfo) |awk -F ',' '{print $1}')
        local SRV_domain=$(echo $(get_sim_sysinfo) |awk -F ',' '{print $2}')
        local SYS_mode=$(echo $(get_sim_sysinfo) |awk -F ',' '{print $3}')

        if [[ ${SRV_status} -eq 0 ]];then
                echo "***** the SRV_status is No Service *****" >>${log_file}
                write_sim_flag
        fi
        if [[ ${SRV_domain} -eq 0 ]];then
                echo "***** the SRV_domain is No Service *****" >>${log_file}
                write_sim_flag
        else
                if [[ ${SRV_domain} -eq 255 ]];then
                        echo "***** CDMA does not support the service domain *****" >>${log_file}
                        write_sim_flag
                fi
        fi
        if [[ ${SYS_mode} -eq 0 ]];then
                echo "***** the SYS_mode is No Service *****" >>${log_file}
                write_sim_flag
        fi
}

check_sim_exist() {
        local sim_exite_state=$(echo $(get_sim_sysinfo) |awk -F ',' '{print $4}')

        if [[ ${sim_exite_state} -eq 255 ]];then
                echo "***** SIM is not EXIST,Please insert the SIM card *****" >>${log_file}
                write_sim_flag
                killall light_nosim.sh 2>/dev/null
                /sbin/light_nosim.sh &
        else
                check_sim_state
#                if [[ ${sim_flag}  -eq 0 ]];then
#                        echo "***** the SIM card NO Money *****" >>${log_file}
#                       sim_nomoney_light
#                fi
        fi
}

main() {
        local sim_flag=0
        local log_file=/tmp/3gdown.log

        ubus call network.interface.evdo down 2>/dev/null
        sleep 1
        check_lte_modules
        check_prefmodel
#       check_signal_3g
#       check_sim_exist
        ubus call network.interface.evdo up 2>/dev/null
}

main "$@"