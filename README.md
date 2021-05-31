# get_radar_rtk_data
该工具用于有效地提取毫米波雷达标定数据
场景：装载rtk的标定车在路口行驶，期间毫米波雷达不断地采集数据。因为采集时间较长，且路口有其他车辆的干扰，属于标定车的毫米波雷达数据难以提取。
该工具 首先自动提取位于路口有效区域内的rtk数据，然后根据比较每一条毫米波雷达与rtk轨迹的相似度，从而得到匹配的雷达标定数据
![image](https://github.com/lei-smile/get_radar_rtk_data/blob/master/record/2021-05-22%2013-45-58%20%E7%9A%84%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE.png)
