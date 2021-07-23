#include "machyapi.h"
#include "machysockets.h"

namespace machyAPI
{
    void read_csv(std::string filedir, std::vector<Sim> &virposition)
    {
        std::ifstream file(filedir);
        clock_t begin_t = clock();
        if(!file.is_open()) throw std::runtime_error("could not open file");

        if(file.good())
        {
            std::string line;
            float value[7];
            while(std::getline(file, line))
            {
                std::stringstream ss(line);
                for(int i=0; i<7; i++)
                {
                    ss >> value[i];
                    ss.ignore();
                }
                virposition.push_back({value[0], value[1], value[2], value[3], value[6]});
            }
        }
    }

    void read_csv(std::string filedir, std::vector<Data> &position)
    {
        std::ifstream file(filedir);
        clock_t begin_t = clock();

        if(!file.is_open()) throw std::runtime_error("could not open file");

        if(file.good())
        {  
            std::string line;
            float value[2];
            while(std::getline(file, line))
            {
                std::stringstream ss(line);
                ss >> value[0];
                ss.ignore();
                ss >> value[1];
                position.push_back({value[0], value[1]});
            }
        }
        clock_t end_t = clock();
        printf("read file in %lf\n", double(end_t-begin_t)/double(CLOCKS_PER_SEC));
    }

    void print_csv(std::vector<Data> &position)
    {
        for (const auto &arr: position){
            std::cout<< arr.x<< ", " << arr.y <<std::endl;
        }
    }

    void print_csv(std::vector<Sim> &virposition)
    {
        std::cout<<"x, y, t, v, theta"<<std::endl;
        for (const auto &arr: virposition){
            std::cout<<arr.x<<". "<<arr.y<<", "<<arr.t<<", "<<arr.v<<", "<<arr.theta<<std::endl;
        }
    }

    void handler(unsigned int request_id, const std::string& response, const system::error_code& ec)
    {
        if (ec == 0) {
            std::cout << "Request #"<<request_id<<" has completed. Response: "
                << response << std::endl;
        } else if (ec == asio::error::operation_aborted) {
            std::cout << "Request #" << request_id << " has been cancelled by the user.\n";
        } else {
            std::cout << "Request #"<< request_id << " failed! Error code = " << ec.value()
            <<". Error message = "<<ec.message()<<std::endl;
        }

        return;
    }

    int asynchronous_example_client()
    {
        try {
            machysockets_aSync::aSyncTCPClient client;

            client.emulateLongComputationOp(10, "127.0.0.1", 3333, handler, 1);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            client.emulateLongComputationOp(11, "127.0.0.1", 3333, handler, 2);
            client.cancelRequest(1);
            std::this_thread::sleep_for(std::chrono::seconds(6));
            client.emulateLongComputationOp(12, "127.0.0.1", 3333, handler, 3);
            std::this_thread::sleep_for(std::chrono::seconds(15));
            client.close();
        }
        catch (system::system_error &e) {
            std::cout << "Error occured! Error code = " << e.code() << ". Message: "<< e.what();
            return e.code().value();
        }
        return 0;
    }

    int asynchronous_trajectory_client(std::vector<Sim> &virposition)
    {
        try {
            machysockets_aSync::aSyncTCPClient client;
            // concatenate strings
            // print_csv(virposition);
            std::string trajectory_data ("TRAJSIM001:");
            
            for (const auto &arr: virposition){
                trajectory_data += std::to_string(arr.x)+","+std::to_string(arr.y)
                        +","+std::to_string(arr.t)+","+std::to_string(arr.v)+","
                        +std::to_string(arr.theta)+"\n";
            }

            trajectory_data += "\n";
            std::cout<<trajectory_data<<std::endl;
            client.send_data("127.0.0.1", 3333, handler, 1, trajectory_data);
            std::this_thread::sleep_for(std::chrono::seconds(10));
            //client.emulateLongComputationOp(11, "127.0.0.1", 3333, handler, 2);
            //client.cancelRequest(1);
            //std::this_thread::sleep_for(std::chrono::seconds(6));
            //client.emulateLongComputationOp(12, "127.0.0.1", 3333, handler, 3);
            //std::this_thread::sleep_for(std::chrono::seconds(15));
            client.close();
        }
        catch (system::system_error &e) {
            std::cout << "Error occured! Error code = " << e.code() << ". Message: "<< e.what();
            return e.code().value();
        }
        return 0;
    }

    int asynchronous_trajectory_client(std::vector<Sim> &virposition, std::vector<Data> &trajectory)
    {
        try {
            machysockets_aSync::aSyncTCPClient client;
            
            std::string trajectory_data ("TRAJSIM002:");
            std::string position_data ("VIRPOS0001:");

            for (const auto &arr: trajectory){
                trajectory_data += std::to_string(arr.x)+","+std::to_string(arr.y)+"\n";
            }

            trajectory_data += "\n";
            //std::cout<<trajectory_data<<std::endl;
            
            client.send_data("127.0.0.1", 3333, handler, 1, trajectory_data);

            for (const auto &arr: virposition){
                position_data += std::to_string(arr.x)+","+std::to_string(arr.y)+
                ","+std::to_string(arr.t)+","+std::to_string(arr.v)+","+std::to_string(arr.theta)+"\n";
            }

            position_data += "\n";

            client.send_data("127.0.0.1", 3333, handler, 1, position_data);
            client.close();
        }
        catch (system::system_error &e) {
            std::cout << "Error occured! Error code = " << e.code() << ". Message: "<< e.what();
            return e.code().value();
        }
        return 0;
    }
}