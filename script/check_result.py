import numpy as np
import matplotlib.pyplot as plt
import os  
import math
import yaml
import sys

# print(str(sys.argv[1]))

#file_name = "../dataset/"+str(sys.argv[1])+"/points_on_road.txt"
file_name = "83_2_all_radar_gps_r.txt"
p_r = np.loadtxt(file_name, delimiter=',')

delay_t = 1
delta_r = 3

# radar_r = p_r[delay_t:, 0]
# gps_r = p_r[:-delay_t, 1]

radar_r = p_r[:, 0] +delta_r
gps_r = p_r[:, 1]

error_r = radar_r - gps_r

error_r_np = np.array(error_r)
err = error_r_np.sum() / len(error_r)
print("avg_err:", err)

length = range(len(radar_r))
plt.plot(length, radar_r, 'r*-', label = 'radar_r')
plt.plot(length, gps_r, 'g+-', label = 'gps_r')
plt.show()


# plt.plot(time_se, x_err_arr,'r+')
# plt.plot(time_se, y_err_arr,'g+')

# plt.plot(time_se, r_err_list,'b+')


# plt.xlabel('Time/s',fontsize=14)
# plt.ylabel('Error/m',fontsize=14)

# plt.title('X(r) Y(g) Distance(b) Error',fontsize=24)

# plt.show()

