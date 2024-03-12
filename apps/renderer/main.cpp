#include <iostream>
#include <lt/lt.h>


int main(int argc, char* argv[])
{

    //for (int a = 1; a < argc; a++) {

        lt::Renderer ren;
        lt::Scene scn;

        //lt::vis_mis_scn(scn, ren);
        lt::cornell_box(scn, ren);

        for (int s = 0; s < 1000; s++) {
            std::cout << ren.render(scn) << std::endl;
        }

        lt::save_sensor_exr(*ren.sensor,"vis.exr");
        
    //}

    return 0;
}