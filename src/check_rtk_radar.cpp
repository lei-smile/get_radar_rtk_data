#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <string>

#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/LocalCartesian.hpp>

#include "data_struct.h"

using namespace cv;
using namespace std;


double lat0 = 39.08245244, lon0 = 117.0434205, height0 = 0.0;
static const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();
GeographicLib::LocalCartesian gps_trans(lat0, lon0, height0, earth); //以原点构建局部坐标系(东北天)


void readGPSData(std::string fileName, std::vector<GPSinsData>& gps_datas, char separator)
{
        std::ifstream ifile(fileName);
        std::string lineStr;
        if (!ifile.is_open())
        {
        std::cerr << "RTK ins file cannot openned " << std::endl;
        return;
        }

        while (std::getline(ifile, lineStr))
        {
                std::stringstream ss(lineStr);
                std::string gps_time_str;
                std::string str;
                GPSinsData dataInfo;

                std::getline(ss, gps_time_str, separator);  // 1610171708880000000
                
                string gps_time_s = gps_time_str.substr(0,10);
                
                string gps_time_us = gps_time_str.substr(10,6);
                
                unsigned int time_s = strtoul(gps_time_s.c_str(),NULL,10);
                
                unsigned int time_us = strtoul(gps_time_us.c_str(),NULL,10);
                
                long double time_second = (long double)time_s;
                long double time_usecond = (long double)time_us;
                long double gps_time = time_second + time_usecond/1000000.0;

                dataInfo.time_stamp = gps_time;
                
                std::getline(ss, str, separator); // $GPFPD
                std::getline(ss, str, separator); // 2139
                std::getline(ss, str, separator); // 5539726.860
                std::getline(ss, str, separator); // 177.532
                std::getline(ss, str, separator); // 3.274
                std::getline(ss, str, separator); // 0.923
                std::getline(ss, str, separator); dataInfo.lat = atof(str.c_str());
                std::getline(ss, str, separator); dataInfo.lon = atof(str.c_str());
                std::getline(ss, str, separator); dataInfo.height = atof(str.c_str());

                std::getline(ss, str, separator); string lat_s = str;
                std::getline(ss, str, separator); string lon_s = str;
                std::getline(ss, str, separator); string height_s = str;

                gps_trans.Forward(dataInfo.lat, dataInfo.lon, dataInfo.height, dataInfo.x, dataInfo.y, dataInfo.z);
                
                gps_datas.push_back(dataInfo);
        }
}

bool readRadarData(std::string fileName, RadarDataTime& points_radar, char separator = ',')
{
        cv::Size im_size2(1000, 1000);
        cv::Mat img2(im_size2, CV_8UC3, CV_RGB(200,200,200));
        std::ifstream ifile(fileName);
        if (!ifile.is_open())
        {
        std::cerr << "file can't opened  !!!!!!!!!" <<std::endl;
        return false;
        }

        std::string lineStr;

        while (std::getline(ifile, lineStr))
        {
                std::stringstream ss(lineStr);
                std::string str;
                RadarData p_radar;

                //timestame:1611911455.334571650, objid:1, type:0, xpos:0.0ypos:0.0,xspeed:9.03, yspeed:-6.51,distense:98,angle:-42.7
                //timestame:1614673256.230000000, objid:0, type:0, xpos:0.0ypos:0.0,xspeed:13.14,yspeed:-0.90,angle:-2.21,distense:195.400,rcs:15.000000
                std::getline(ss, str, separator); //timestame:1614673256.230000000
                std::stringstream ss0(str);
                std::string str0;
                std::getline(ss0, str0, ':');  //timestame
                std::getline(ss0, str0, ':'); // 1614673256.230000000
                std::stringstream ss00(str0);
                std::string str00;
                std::getline(ss00, str00, '.'); //1614673256
                unsigned int a_s = strtoul(str00.c_str(),NULL,10);
                // std::cout <<"unsigned int a_s  "<< a_s << std::endl;
                std::getline(ss00, str00, '.'); //230000000
                unsigned int c_ns = strtoul(str00.c_str(),NULL,10);
                // std::cout <<"unsigned int c_ns  "<< c_ns << std::endl;
                points_radar.TimeS = a_s;
                points_radar.TimeNS = c_ns;
                long double time_second = (long double)a_s;
                long double time_ns = (long double)c_ns/1000000000.0;
                points_radar.time = time_second + time_ns;
                p_radar.time =  time_second + time_ns;

                std::getline(ss, str, separator); //objId:0
                std::stringstream ss1(str);
                std::string str1;
                std::getline(ss1, str1, ':');   //objId
                std::getline(ss1, str1, ':'); p_radar.track_id = atoi(str1.c_str()); // 0

                std::getline(ss, str, separator); //type:0
                std::getline(ss, str, separator);
                std::getline(ss, str, separator); //xpos:0.0ypos:0.0

                std::getline(ss, str, separator); //xspeed:13.14
                std::stringstream ss2(str);
                std::string str2;
                std::getline(ss2, str2, ':');   //xspeed
                std::getline(ss2, str2, ':'); p_radar.vx = atof(str2.c_str()); // 13.14

                std::getline(ss, str, separator); //yspeed:-0.90

                std::getline(ss, str, separator); //angle:-2.21
                std::stringstream ss3(str);
                std::string str3;
                std::getline(ss3, str3, ':');  // angle
                std::getline(ss3, str3, ':'); p_radar.angle = atof(str3.c_str()); //-2.21


                std::getline(ss, str, separator); //distense:195.400
                std::stringstream ss4(str);
                std::string str4;
                std::getline(ss4, str4, ':');  //distense
                std::getline(ss4, str4, ':'); p_radar.r = atof(str4.c_str()); // 195.400

                double theta = p_radar.angle*M_PI/180.0;
                p_radar.x_radar = 1.0*p_radar.r*sin(theta);
                p_radar.y_radar = p_radar.r*cos(theta);

                points_radar.radar_datas.push_back(p_radar);

        } 
        ifile.close();
        return true;
}

//将radar的r和thata转换为x、y
void trans_radar_xy(double Tx, double Ty, double h, double yaw, RadarData& radar_point)
{
        double d = sqrt(radar_point.r * radar_point.r - h*h);
        double x_radar = d*sin(radar_point.angle *M_PI / 180.0);  
        double y_radar = d*cos(radar_point.angle * M_PI / 180.0);

        radar_point.x_in_world = x_radar*cos(yaw) - y_radar*sin(yaw) + Tx;  
        radar_point.y_in_world = x_radar*sin(yaw) + y_radar*cos(yaw) + Ty;
}

//check a point is in a quadrangle or not
double dis(cv::Point2f p1, cv::Point2f p2)
{
        return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}
 
//计算三角形面积
double triangleArea(cv::Point2f p1, cv::Point2f p2, cv::Point2f p3)
{
        double a = dis(p1, p2);
        double b = dis(p2, p3);
        double c = dis(p3, p1);
        double p = (a + b + c) * 0.5;
        return sqrt(p * (p - a) * (p - b) * (p - c));
}
//target_point为需要判断的点
//vertex_four要按照顺时针或者逆时针的顺序给定，且个数为4
bool isInQuadrangle(const cv::Point2f& target_point, 
                                            const std::vector<cv::Point2f>& vertex_four) 
                                            
{
        cv::Point2f A, B, C, D;
        if(vertex_four.size() ==4)
        {
                A = vertex_four[0];
                B = vertex_four[1];
                C = vertex_four[2];
                D = vertex_four[3];
        }
        else
        {
                std::cout<<"input vertex point size is not four!!"<<std::endl;
                return false;
        }

        double dTriangle   = triangleArea(A, B, target_point) + triangleArea(B, C, target_point) 
                                                + triangleArea(C, D, target_point) + triangleArea(D, A, target_point);
        double dQuadrangle = triangleArea(A, B, C) + triangleArea(A, D, C);// s5 + s6;
        if(fabs(dTriangle - dQuadrangle)<1)    //说明理论上应该是相等的，但因为计算本身的原因可能会有细微不同，所以选择小于1，根据实际情况来设置
        {
        return true;
        }
        else
        {
        return false;
        }
}

int main()
{
        cv::Mat img_birdview(cv::Size(1000, 1000), CV_8UC3,cv::Scalar(255,255,255));
        std::vector<cv::Point2f> vertex_rtk;
        vertex_rtk.push_back(cv::Point2f(39.08245244, 117.0434205));
        vertex_rtk.push_back(cv::Point2f(39.08179735, 117.04361432));
        vertex_rtk.push_back(cv::Point2f(39.08169408, 117.04278509));
        vertex_rtk.push_back(cv::Point2f(39.08232708, 117.04261154));
        std::vector<cv::Point2f> vertex_xy;
        for(int i = 0 ; i < 4; i++)
        {
                double x=0.0, y=0.0, z=0.0; 
                gps_trans.Forward(vertex_rtk[i].x, vertex_rtk[i].y, 0.0, x, y, z);
                vertex_xy.push_back(cv::Point2f(x, y));
                std::cout<<"point_x_y: "<<x<<"     "<<y<<std::endl;
                
        }
        for(int i = 0; i < 4; i++)
        {
                cv::Point2f p_draw(vertex_xy[i].x * 4 + 500, vertex_xy[i].y * 4 + 500);
                cv::circle(img_birdview,p_draw, 5, cv::Scalar(0, 255, 0), -1);
                cv::putText(img_birdview, cv::String(std::to_string(i + 1)),p_draw,0,1, cv::Scalar(0,255,0) );
                cv::Point2f p_1 = p_draw;
                cv::Point2f p_2(vertex_xy[(i+1)%4].x * 4+ 500, vertex_xy[(i+1)%4].y * 4 + 500);
                cv::line(img_birdview,p_1, p_2, cv::Scalar(255, 0, 0), 2);
        }
        
        //读取rtk和radar数据
        std::string rtk_file = "../rtk.txt";
        std::vector<GPSinsData> gps_datas;
        readGPSData(rtk_file, gps_datas, ',');

        std::string radar_path = "../data/radar/";
        std::vector<cv::String> radar_files;
        cv::glob(radar_path + "*.txt", radar_files);
        std::vector<RadarDataTime> radar_times_all;
        for(int i = 0; i < radar_files.size(); i++)
        {
                RadarDataTime points_radar;
                bool flag = readRadarData(radar_files[i], points_radar, ',');
                radar_times_all.push_back(points_radar);
        }

        //处理rtk数据
        std::vector<GPSinsData> gps_datas_inROI;  //提取位于路口的数据
        for(int i =0; i < gps_datas.size(); i++)
        {
                cv::Point2f point_gps(gps_datas[i].x, gps_datas[i].y);
                if(isInQuadrangle(point_gps, vertex_xy))
                {
                        gps_datas_inROI.push_back(gps_datas[i]);
                }
        }

        std::vector< std::vector<GPSinsData>> gps_datas_lines;  //将位于路口的rtk按轨迹分段
        std::vector<GPSinsData> gps_line;
        for(int i = 0; i < gps_datas_inROI.size() -1; i++)       //根据轨迹的连续性来分段
        {       
                cv::Point2f p_now(gps_datas_inROI[i].x, gps_datas_inROI[i].y);
                cv::Point2f p_next(gps_datas_inROI[i+1].x, gps_datas_inROI[i+1].y);
                //TODO 加入轨迹朝向的验证
                double angle = atan((p_next.y-p_now.y) / (p_next.x - p_next.x));
                std::cout<<"trajectory angle:"<<angle<<std::endl;
                if(dis(p_now, p_next) < 2.0)
                {
                        gps_line.push_back(gps_datas_inROI[i]);
                }
                else
                {
                        gps_datas_lines.push_back(gps_line);
                        gps_line.clear();
                        //std::vector<GPSinsData> ().swap(gps_line); //clear只是清空而没有释放vector的内存，使用 ().swap则可以对内存进行清空
                }        
        }
        if(!gps_line.empty())               //上一段循环中对最后一段轨迹没有保存
        {
                gps_datas_lines.push_back(gps_line);
        }
        std::cout<<"gps_lines.size::   "<<gps_datas_lines.size()<<std::endl;

        //处理毫米波数据
        double Tx = 0.0;
        double Ty = 0.0;
        double h = 6.0;
        double yaw_radar = 2.4946;
        for(int i = 0; i < radar_times_all.size(); i ++)
        {
                for(int j = 0; j < radar_times_all[i].radar_datas.size(); j++)
                {
                        RadarData radar_point = radar_times_all[i].radar_datas[j];
                        trans_radar_xy(Tx, Ty,h, yaw_radar, radar_point);
                        //cv::Point2f radar_draw(radar_point.x_in_world *4 +500, radar_point.y_in_world*4 +500);
                        //按ID画毫米波轨迹
                        // if(radar_point.track_id == 5)
                        // {
                        //         cv::circle(img_birdview, radar_draw, 2, cv::Scalar(125,0,0), 1);
                        // }
                }
        }

        //按rtk轨迹提取对应时间段的毫米波数据，并对比每个ID与rtk的相似度，从而得到rtk车辆的ID
        
        for(int i = 0; i < gps_datas_lines.size(); i++)
        {
                 //先跟据rtk轨迹时间段，把该时间段的radar数据抽取出来
                 double line_start_time = gps_datas_lines[i][0].time_stamp;
                 int line_length = gps_datas_lines[i].size();
                 double line_end_time = gps_datas_lines[i][line_length-1].time_stamp;
                 printf("start_time: %lf, end_time: %lf\n", line_start_time, line_end_time);
                 
                std::vector<RadarDataTime> radar_times_selected;  //处于rtk轨迹时间段的radar数据
                for(int n = 0; n < radar_times_all.size(); n++)
                {
                        if(radar_times_all[n].time >= line_start_time && radar_times_all[n].time <= line_end_time)
                        {
                                radar_times_selected.push_back(radar_times_all[n]);
                        }
                }
                int ID_sum = 30;    //预设的总的ID数，与下面k循环的次数一致     
                std::vector<std::vector<RadarData>> radar_IDs;     //按ID存放该rtk轨迹时间段中的所有雷达数据
                radar_IDs.resize(ID_sum);                            
                std::vector<RadarData> radar_ID_lines;  //从radar_IDs中抽取出来的，与每一条rtk轨迹匹配的 某一ID的毫米波轨迹
                
                for(int m =0; m < radar_times_selected.size(); m++)
                {
                        for(int w = 0; w < radar_times_selected[m].radar_datas.size(); w++)
                        {
                                for(int k = 0 ; k < ID_sum; k++)
                                {
                                        if(radar_times_selected[m].radar_datas[w].track_id == k )  //!!!!radar_IDs[0]对应radar ID为0
                                        {
                                                
                                                radar_IDs[k].push_back(radar_times_selected[m].radar_datas[w]);
                                        }
                                }
                        }
                        
                }
                //test above  画出通过时间筛选后的radar轨迹
                for(int m = 0; m < radar_IDs.size(); m++)
                {
                        for(int w = 0; w < radar_IDs[m].size(); w++)
                        {
                                RadarData radar_point = radar_IDs[m][w];
                                trans_radar_xy(Tx, Ty,h, yaw_radar, radar_point);
                                cv::Point2f radar_draw(radar_point.x_in_world *4 +500, radar_point.y_in_world*4 +500);
                                //cv::circle(img_birdview, radar_draw, 2, cv::Scalar(125,0,0), 1);
                        }
                }

                //只是根据ID来区分毫米波轨迹存在问题：同一个ID会对应不同的目标
                //需要进一步地 对已经用ID 区分后的轨迹 再进行连续性判断，从而分段
                
                std::vector<std::vector<RadarData>> radar_trajectorys;
                std::cout<<"radar_IDs.size():   "<<radar_IDs.size()<<std::endl;
                for(int k =0; k < radar_IDs.size(); k++)
                {
                        std::vector<RadarData> radar_traj;
                        std::cout<<"radar_size::  "<<radar_IDs[0].size()<<std::endl;
                        std::cout<<"radar_IDs[k].size()-1::"<<radar_IDs[k].size()-1<<std::endl;
                        for(int n =0; n < int(radar_IDs[k].size()-1); n++)   //Atention:vector .size()是无符号整型，当size为0时，-1会有问题，溢出导致下标引用越界
                        {
                                std::cout<<"k:  "<<k<<"  n:  "<<n<<std::endl;
                                std::cout<<"radar_IDs[k][n]:  "<<radar_IDs[k][n].r<<std::endl;
                                RadarData radar_now = radar_IDs[k][n];
                                std::cout<<"radar_now.r:    "<<radar_now.r<<std::endl;
                                RadarData radar_next = radar_IDs[k][n+1];
                                if(fabs(radar_now.r - radar_next.r) < 5.0)
                                {
                                        
                                        radar_traj.push_back(radar_now);
                                }
                                else
                                {
                                        if(radar_traj.size() < 5)           //去掉一些噪点
                                                continue;
                                        radar_trajectorys.push_back(radar_traj);
                                        radar_traj.clear();
                                }
                                
                        }
                        if(radar_traj.size() != 0)
                        {
                                radar_trajectorys.push_back(radar_traj);
                        }
                }
                std::cout<<"radar_trajectorys.size():"<<radar_trajectorys.size()<<std::endl;

                for(int j = 0; j < gps_datas_lines[i].size(); j+=6)    //rtk的频率较高
                {
                        cv::Point2f p_rtk(gps_datas_lines[i][j].x * 4 +500, gps_datas_lines[i][j].y * 4 + 500);
                        cv::circle(img_birdview, p_rtk, 2, cv::Scalar(0, 0, 255), -1);   
                }

                //对radar_IDs进行处理，从中抽取与rtk轨迹对应的Id
                //无标定数据下的抽取 与 基于标定数据的抽取
                double min_avgdis = 10000.0;
                int rtk_radar_ID = -1;                    //与rtk轨迹匹配的毫米波ID
                for(int m = 0; m < radar_trajectorys.size(); m++)
                {
                        double sum_dis = 0.0;
                        if(radar_trajectorys[m].size() < 100)   //对于较短的毫米波轨迹进行不予考虑
                                continue;
                        for(int w = 0; w < radar_trajectorys[m].size(); w++)
                        {
                                RadarData radar_point = radar_trajectorys[m][w];
                                GPSinsData gps_point_matched;
                                bool match_flag = false;
                                for(int j = 0; j < gps_datas_lines[i].size(); j++)
                                {
                                        
                                        if(fabs(radar_point.time - gps_datas_lines[i][j].time_stamp) <= 0.01)
                                        {
                                                printf("find rtk point with radar point in the same time\n");
                                                gps_point_matched = gps_datas_lines[i][j];
                                                match_flag = true;
                                                break;
                                        }
                                }
                                if(match_flag)
                                {
                                        double gps_r = sqrt( pow(gps_point_matched.x, 2) + pow(gps_point_matched.y, 2));
                                        sum_dis += fabs(radar_point.r - gps_r);
                                }      
                        }
                        double avg_dis = sum_dis / double(radar_trajectorys[m].size());
                        if(avg_dis < min_avgdis)
                        {
                                min_avgdis = avg_dis;
                                rtk_radar_ID = m;             //track_id 也是m
                        }
                }
                //rtk_radar_ID = 5;
                if(rtk_radar_ID != -1)
                {
                        printf("matched radar ID: %d\n", rtk_radar_ID);
                        for(int w = 0; w < radar_trajectorys[rtk_radar_ID].size(); w++)
                        {
                                RadarData radar_point = radar_trajectorys[rtk_radar_ID][w];
                                trans_radar_xy(Tx, Ty,h, yaw_radar, radar_point);
                                std::cout<<"radar_point_id:  "<<radar_point.track_id<<std::endl;
                                cv::Point2f radar_draw(radar_point.x_in_world *4 +500, radar_point.y_in_world*4 +500);
                                cv::circle(img_birdview, radar_draw, 2, cv::Scalar(125,0,0), 1);
                        }
                }


                
                

                
                //开始的做法：逐点去抽取，复杂度太高，先跟据rtk轨迹时间段，把该时间段的radar数据抽取出来比较合理
                // for(int j = 0; j < gps_datas_lines[i].size(); j+=6)    //rtk的频率较高
                // {
                //         cv::Point2f p_rtk(gps_datas_lines[i][j].x * 4 +500, gps_datas_lines[i][j].y * 4 + 500);
                //         cv::circle(img_birdview, p_rtk, 2, cv::Scalar(0, 0, 255), -1);
                //         double rtk_point_time = gps_datas_lines[i][j].time_stamp;
                //         //对于每一条rtk轨迹提取对应时间的毫米波数据
                //         for(int n = 0; n < radar_times_all.size(); n ++)
                //         {
                //                 //TODO 待验证该时间判断方式是否合理
                //                 if(radar_times_all[n].time < rtk_point_time && radar_times_all[n].time > rtk_point_time)
                //                 {
                //                         std::cout<<"find radar point in the same time with rtk point"<<std::endl;
                //                         for(int m = 0; m < radar_times_all[n].radar_datas.size(); m++)
                //                         {
                //                                 for(int k =0; k < 30; k++)    //30是手动设置的该rtk轨迹对应期间的毫米波总ID数，可调
                //                                 {
                //                                         if(radar_times_all[n].radar_datas[m].track_id == k)
                //                                         {
                //                                                 radar_IDs[k].push_back(radar_times_all[n].radar_datas[m]);
                //                                                 break;
                //                                         }

                //                                 }
                //                         }
                //                 }
                //         }
                // }


                
                
        }

        

        cv::imshow("Birdview", img_birdview);
        cv::waitKey();
}

