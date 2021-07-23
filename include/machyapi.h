#ifndef MACHYAPI_H
#define MACHYAPI_H

#include <stdio.h>
#include <stdlib.h>

#include <boost/asio.hpp>
//#include <boost/json/src.hpp>

#include <vector>
#include <fstream>
#include <string>
#include <thread>
#include <iostream>

using namespace boost;

namespace machyAPI
{
    /*
     * API machytech
     */
    struct Data{
        float x, y;
    };
    /*
     * data structure with x and y data accepted by the openGL context
     */
    struct Sim{
        float x, y, t, v, theta;
    };
    /*
     * data structure with x and y data. Also it uses simulation
     * data available from the path trajectory.
     */
    void read_csv(std::string filedir, std::vector<Data> &position);
    /* 
    * this function reads a csv file locally and stores the data points on the GPU
    * @param filedir: path to local file
    * @param position: position data 
    */
    void read_csv(std::string filedir, std::vector<Sim> &virposition);
    /*
    * @overload void read_csv(), this function reads a csv file locally and stores the data points on the GPU
    * @param filedir: path to local file
    * @param virposition: use sim data as position
    */
    void print_csv(std::vector<Data> &position);
    /*
        * this function prints data structure
        * @param position: data structure with position data
        */
    void print_csv(std::vector<Sim> &virposition);
    /*
    * this function prints a data structure
    * @param position: data structure with virtual position data
    */
    void handler(unsigned int request_id, const std::string& response, const system::error_code& ec);
    /*
     * callback function to output results of the request execution
     */
    int asynchronous_trajectory_client(std::vector<Sim> &virposition);
    /*
     * connect to server in asynchronous manner and stream virtual position
     * data.
     */
    int asynchronous_trajectory_client(std::vector<Sim> &virposition, std::vector<Data> &trajectory);
}
#endif