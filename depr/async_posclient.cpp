#include "machyapi.h"

using namespace machyAPI;

int main()
{
    // new sim data instance
    std::vector <Sim> simpos_data;
    // new position data instance
    std::vector <Data> trajectory_data;

    read_csv("/home/timo/Projects/machytech/machytech-core/tests/trajectories/trajectory_100_fpg_out.txt",
        trajectory_data);
    read_csv("/home/timo/Projects/machytech/machytech-core/tests/trajectories/trajectory_100_fpg_out.txt",
        simpos_data);
    int err = asynchronous_trajectory_client(simpos_data, trajectory_data);
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