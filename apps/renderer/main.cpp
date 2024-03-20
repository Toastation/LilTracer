#include <iostream>
#include <lt/lt.h>


int main(int argc, char* argv[])
{

    for (int a = 1; a < argc; a++) {

        lt::Renderer ren;
        lt::Scene scn;

        lt::generate_from_path(argv[a], scn, ren);

        float time = 0.;

        for (int s = 0; s < ren.max_sample;  s++) {
            float t = ren.render(scn);
            std::cout << s << ":" << t << std::endl;
            time += t;
        }

        std::cout << "Time elapsed : " << time << " (ms) " << std::endl;

        lt::save_sensor_exr(*ren.sensor,std::string(argv[a])+".exr");
        
    }

    return 0;
}