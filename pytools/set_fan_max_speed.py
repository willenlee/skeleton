#!/usr/bin/env python

import os

if __name__ == '__main__':
    hwmon_path="/sys/class/hwmon"
    for dir in os.listdir(hwmon_path):
        dir_name = hwmon_path + "/" + dir
        for node in os.listdir(dir_name):
            if node.find("pwm")>=0 and node.find("_enable")<0:
                pwm_node_name = dir_name+"/"+node
                cmd = "echo 255 > " + pwm_node_name
                print cmd
                os.system(cmd)
