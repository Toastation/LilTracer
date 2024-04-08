#include <iostream>
#include <lt/lt.h>

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(double percentage, float t) {
    int val = (int)(percentage * 100);
    int lpad = (int)(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    printf("\t%f (ms)", t);
    fflush(stdout);
}

void generate(lt::Renderer& ren, lt::Scene& scn,const std::vector<glm::ivec2>& pix, std::string base_path )
{

    std::shared_ptr<lt::VarianceSensor> var_sensor = std::dynamic_pointer_cast<lt::VarianceSensor>(ren.sensor);
    var_sensor->reset();

    int max_sample = 1024;
    float time = 0.;
    int next_sample = 1;

    // init csv
    std::stringstream csv_stream;
    csv_stream << "sample" << "\t";
    csv_stream << "time" << "\t";
    for (const auto& p : pix) {
        csv_stream << "variance" << "_" << p.x << "_" << p.y << "\t";
        csv_stream << "mean" << "_" << p.x << "_" << p.y << "\t";
    }
    csv_stream << "\n";

    for (int s = 0; s < max_sample; s++) {
        float t = ren.render(scn);

        time += t;

        printProgress(double(s) / double(max_sample), t);

        // save data
        csv_stream << s << "\t";
        csv_stream << t << "\t";
        for (const auto& p : pix) {
            csv_stream << var_sensor->get(p.x, p.y).r << "\t";
            csv_stream << var_sensor->acculumator[p.y * var_sensor->w + p.x].r << "\t";
        }
        csv_stream << "\n";

        // save sensor
        if (s == next_sample - 1) {
            std::string output_path = base_path + "_var_" + std::to_string(next_sample) + ".exr";
            lt::save_sensor_exr(*ren.sensor, output_path);

            var_sensor->use_variance(false);
            output_path = base_path + "_" + std::to_string(next_sample) + ".exr";
            lt::save_sensor_exr(*ren.sensor, output_path);

            next_sample = next_sample << 1;
        }
    }

    // save csv
    std::ofstream output;
    output.open(base_path + ".csv");
    output << csv_stream.str();
    output.close();

    std::cout << "Time elapsed : " << time << " (ms) " << std::endl;
}
int main(int argc, char* argv[]) {

    std::string option = argv[1];
    std::string scene_path = argv[1];

    
    std::vector<glm::ivec2> pix = { {242, 216} , { 383, 381 } };


    // Generate Scene
    lt::Renderer ren;
    lt::Scene scn;
    lt::generate_from_path( scene_path, scn, ren);


    std::map<std::string, std::shared_ptr<lt::Brdf>> brdfs = { {"rough_ggx",std::make_shared<lt::RoughGGX>(0.1, 0.1)}
                                                            ,{"rough_ggx_vis",std::make_shared<lt::RoughGGX>(0.1, 0.1)}
                                                            ,{"diffuse_ggx",std::make_shared<lt::DiffuseGGX>(0.1, 0.1)}
                                                            ,{"diffuse_ggx_vis",std::make_shared<lt::DiffuseGGX>(0.1, 0.1)}
    };

    std::dynamic_pointer_cast<lt::RoughGGX>(brdfs["rough_ggx_vis"])->sample_visible_distribution = true;
    std::dynamic_pointer_cast<lt::DiffuseGGX>(brdfs["diffuse_ggx_vis"])->sample_visible_distribution = true;

    // l'écart moyen a la moyenne
    // la moyenne en fonction du nombre d'échantillon
    // le temps de calcul
    // time per frame ?


    for(const auto& [brdf_name, brdf] : brdfs){
        
        // update scene
        scn.geometries[0]->brdf = brdf;
        generate(ren,scn,pix,scene_path + "_" + brdf_name);


    }

    return 0;
}