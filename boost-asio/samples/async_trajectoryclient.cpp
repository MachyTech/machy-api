#include "machyapi.h"

using namespace machyAPI;

int main()
{
    // new sim data instance
    //std::shared_ptr<Sim> sim_data = std::shared_ptr<Sim>(new Sim());
    std::vector <Sim> sim_data;
    read_csv("trajectory_1_fpg_out.txt",
        sim_data);
    int err = asynchronous_trajectory_client(sim_data);
    if(err!=0)
    {
        std::cout<<"error in asyncwehronous trajectory client: "<<err<<std::endl;
        exit(-1);
    }
    else
    {
        std::cout<<"succesfully connected to synchronous server socket!\n";
    }
}