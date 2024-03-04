#include <iostream>
#include <lt/lt.h>

int main(int, char**)
{
    lt::Renderer ren;
    lt::Scene scn;

    //lt::dir_light(scn, ren);
    lt::cornell_box(scn, ren);

    for (int s = 0; s < 100; s++) {
        std::cout << ren.render(scn) << std::endl;
    }

    lt::save_sensor_exr(*ren.sensor,"cornell.exr");

	
    return 0;
}