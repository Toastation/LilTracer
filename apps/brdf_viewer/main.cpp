#include <iostream>
#include <lt/lt.h>

#include <glm/glm.hpp>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <implot.h>



static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static bool need_reset;
#define NEED_RESET(x) if(x){ need_reset = true; }

struct RenderSensor {
    GLuint spec_id;
    std::shared_ptr<lt::Sensor> sensor = nullptr;
    bool initialized = false;


    bool update_data() {

        if (!initialized) {
            return false;
        }


        glBindTexture(GL_TEXTURE_2D, spec_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sensor->w, sensor->h, 0, GL_RGB, GL_FLOAT, sensor->value.data());
        return true;
    }

    bool initialize()
    {

        glGenTextures(1, &spec_id);
        glBindTexture(GL_TEXTURE_2D, spec_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        initialized = true;
        return true;
    }

    ~RenderSensor() {
        if (initialized) {
            glDeleteTextures(1, &spec_id);
        }
    }
};


struct AppData {
    std::shared_ptr<lt::Sensor> s_brdf_slice;
    RenderSensor rs_brdf_slice;
    int current_brdf_idx;
    std::vector<std::shared_ptr<lt::Brdf>> brdfs;
    float theta_i = 0.5;
    float phi_i = 0.;

    lt::Scene    scn_dir_light;
    lt::RendererAsync ren_dir_light;
    RenderSensor rsen_dir_light;

    lt::Scene    scn_glo_ill;
    lt::RendererAsync ren_glo_ill;
    RenderSensor rsen_glo_ill;

    std::shared_ptr<lt::HemisphereSensor> s_brdf_sampling;
    RenderSensor rs_brdf_sampling;
    std::shared_ptr<lt::Sensor> s_brdf_sampling_pdf;
    RenderSensor rs_brdf_sampling_pdf;
    std::shared_ptr<lt::Sensor> s_brdf_sampling_diff;
    RenderSensor rs_brdf_sampling_diff;


    lt::Sampler sampler;

};

void AppInit(AppData& app_data) {
    need_reset = false;

    app_data.brdfs.push_back(lt::Factory<lt::Brdf>::create("Diffuse"));
    app_data.current_brdf_idx = 0;
    
    lt::dir_light(app_data.scn_dir_light, app_data.ren_dir_light);
    app_data.rsen_dir_light.sensor = app_data.ren_dir_light.sensor;
    app_data.rsen_dir_light.initialize();
    app_data.scn_dir_light.geometries[0]->brdf = app_data.brdfs[app_data.current_brdf_idx];

    lt::generate_from_path("../../../data/lte-orb/lte-orb.json",app_data.scn_glo_ill, app_data.ren_glo_ill);
    app_data.rsen_glo_ill.sensor = app_data.ren_glo_ill.sensor;
    app_data.rsen_glo_ill.initialize();
    app_data.scn_glo_ill.geometries[1]->brdf = app_data.brdfs[app_data.current_brdf_idx];
    app_data.scn_glo_ill.geometries[3]->brdf = app_data.brdfs[app_data.current_brdf_idx];

    app_data.s_brdf_slice = std::make_shared<lt::Sensor>(256, 64);
    app_data.rs_brdf_slice.sensor = app_data.s_brdf_slice;
    app_data.rs_brdf_slice.initialize();

    app_data.s_brdf_sampling = std::make_shared<lt::HemisphereSensor>(256, 64);
    app_data.rs_brdf_sampling.sensor = app_data.s_brdf_sampling;
    app_data.rs_brdf_sampling.initialize();

    app_data.s_brdf_sampling_pdf = std::make_shared<lt::Sensor>(256, 64);
    app_data.rs_brdf_sampling_pdf.sensor = app_data.s_brdf_sampling_pdf;
    app_data.rs_brdf_sampling_pdf.initialize();

    app_data.s_brdf_sampling_diff = std::make_shared<lt::Sensor>(256, 64);
    app_data.rs_brdf_sampling_diff.sensor = app_data.s_brdf_sampling_diff;
    app_data.rs_brdf_sampling_diff.initialize();


}


void brdf_slice(std::shared_ptr<lt::Brdf> brdf, float th_i, float ph_i, std::shared_ptr<lt::Sensor> sensor) {

#if 1
    std::vector<float> th = lt::linspace<float>(0, 0.5 * lt::pi, sensor->h);
    std::vector<float> ph = lt::linspace<float>(0, 2.  * lt::pi, sensor->w);
    
    lt::vec3 wi = lt::polar_to_card(th_i, ph_i);
    for (int x = 0; x < sensor->w; x++) {
        for (int y = 0; y < sensor->h; y++) {
            lt::vec3 wo = lt::polar_to_card(th[y], ph[x]);
            sensor->value[y * sensor->w + x] = brdf->eval(wi, wo);
        }
    }
#endif
#if 0
    std::vector<float> th_d = lt::linspace<float>(0.5 * lt::pi, 0., sensor->h);
    std::vector<float> th_h = lt::linspace<float>(0, 0.5 * lt::pi, sensor->w);
    float ph = ph_i;

    for (int x = 0; x < sensor->w; x++) {
        for (int y = 0; y < sensor->h; y++) {

            float sin_th_d = std::sin(th_d[y]);
            float cos_th_d = std::cos(th_d[y]);

            float sin_th_h = std::sin(th_h[x]);
            float cos_th_h = std::cos(th_h[x]);

            float sin_ph = std::sin(ph);
            float cos_ph = std::cos(ph);

            lt::vec3 wd = lt::polar_to_card(th_d[y], 0);
            lt::vec3 wi = lt::vec3(cos_th_h * wd.x + sin_th_h * wd.z, 0.,-sin_th_h * wd.x + cos_th_h * wd.z);
            wi = lt::vec3(cos_ph * wi.x + sin_ph * wi.y, -sin_ph * wi.x + cos_ph * wi.y, wi.z);

            lt::vec3 wh = lt::polar_to_card(th_h[x], 0.);
            
            lt::vec3 wo = glm::reflect(-wi, wh);
         
            sensor->value[y * sensor->w + x] = glm::pow(brdf->eval(wi, wo),lt::vec3(0.4545));
        }
    }
#endif
}



static void AppLayout(GLFWwindow* window, AppData& app_data)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ImGui::SetNextWindowSize(ImVec2(width, height)); // ensures ImGui fits the GLFW window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    

    if (need_reset) {
        app_data.ren_glo_ill.reset();
        app_data.ren_dir_light.reset();
        app_data.s_brdf_sampling->reset();
        app_data.s_brdf_sampling_pdf->reset();
        app_data.s_brdf_sampling_diff->reset();
        need_reset = false;
    }

    bool open = true;
    if (ImGui::Begin("Simple layout", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        
        {
            ImGui::BeginGroup();
            
            ImGui::BeginChild("top pane", ImVec2(250, ImGui::GetContentRegionAvail().y*0.25),true);
            

            const std::vector<std::string>& brdf_names = lt::Factory<lt::Brdf>::names();

            if (ImGui::Button("New BRDF"))
                ImGui::OpenPopup("brdf_popup");

            if (ImGui::BeginPopup("brdf_popup"))
            {
                for (int i = 0; i < brdf_names.size(); i++)
                    if (ImGui::Selectable(brdf_names[i].c_str()))
                        app_data.brdfs.push_back(lt::Factory<lt::Brdf>::create(brdf_names[i]));
                ImGui::EndPopup();
            }
             
            ImGui::Separator();

            for (int i = 0; i < app_data.brdfs.size(); i++)
            {
                if (ImGui::Selectable( (app_data.brdfs[i]->type + "##" + std::to_string(app_data.brdfs[i]->id)).c_str(), app_data.current_brdf_idx == i)) {
                    app_data.current_brdf_idx = i;
                    app_data.scn_dir_light.geometries[0]->brdf = app_data.brdfs[i];
                    app_data.scn_glo_ill.geometries[1]->brdf = app_data.brdfs[i];
                    app_data.scn_glo_ill.geometries[3]->brdf = app_data.brdfs[i];
                    need_reset = true;
                }
            }

            ImGui::EndChild();
            ImGui::BeginChild("bottom pane", ImVec2(250, 0), true);
            
            std::shared_ptr<lt::Brdf> cur_brdf = app_data.brdfs[app_data.current_brdf_idx];

            for (int i = 0; i < cur_brdf->params.count; i++) {
                switch (cur_brdf->params.types[i])
                {
                case lt::Params::Type::FLOAT:
                    NEED_RESET(ImGui::DragFloat(cur_brdf->params.names[i].c_str(), (float*)cur_brdf->params.ptrs[i], 0.01,0.001,3.));
                    break;
                case lt::Params::Type::VEC3:
                    NEED_RESET(ImGui::ColorEdit3(cur_brdf->params.names[i].c_str(), (float*)cur_brdf->params.ptrs[i]));
                    break;
                case lt::Params::Type::SH:
                    if (ImGui::BeginTable("table", 2, ImGuiTableFlags_Borders))
                    {
                        std::vector<float>* sh = (std::vector<float>*)cur_brdf->params.ptrs[i];
                        ImGui::TableSetupColumn("num");
                        ImGui::TableSetupColumn("val");
                        ImGui::TableHeadersRow();
                        
                        for(int j = 0; j < sh->size(); j++){
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", j);
                            ImGui::TableNextColumn();
                            ImGui::Text("%f", sh->at(j));
                        }

                        ImGui::EndTable();
                    }
                    break;
                default:
                    break;
                }
            }
            
            ImGui::EndChild();


            ImGui::EndGroup();
        }

        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, ImGui::GetWindowHeight()*0.9),true); // Leave room for 1 line below us
            
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("BRDF slice"))
                {

                    std::shared_ptr<lt::Brdf> cur_brdf = app_data.brdfs[app_data.current_brdf_idx];

                    brdf_slice(cur_brdf, app_data.theta_i, app_data.phi_i, app_data.s_brdf_slice);
                    app_data.rs_brdf_slice.update_data();
                 
                    if(ImPlot::BeginPlot("##image","","",ImVec2(-1,0),ImPlotFlags_Equal, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
                        
                        ImPlot::PlotImage("BRDF slice", (ImTextureID)app_data.rs_brdf_slice.spec_id, ImVec2(0, 0), ImVec2(app_data.s_brdf_slice->w, app_data.s_brdf_slice->h));
                        ImPlot::EndPlot();
                    }
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("Plot"))
                {
                    if (ImGui::BeginTabBar("##TabsPolar", ImGuiTabBarFlags_None))
                    {

                        if (ImGui::BeginTabItem("Polar"))
                        {
                            static ImPlotSubplotFlags flags = ImPlotSubplotFlags_LinkRows | ImPlotSubplotFlags_LinkCols | ImPlotSubplotFlags_LinkAllX | ImPlotSubplotFlags_LinkAllY;
                            if (ImPlot::BeginSubplots("##AxisLinking", 2, 1, ImVec2(-1, -1), flags)) {

                                std::vector<float> th = lt::linspace<float>(-0.5 * lt::pi, 0.5 * lt::pi, 1001);

                                if (ImPlot::BeginPlot("", "x", "y")) {

                                    for (int i = 0; i < app_data.brdfs.size(); i++) {
                                        std::shared_ptr<lt::Brdf> brdf = app_data.brdfs[i];

                                        static float xs[1001];
                                        static float ys[1001];
                                        for (int x = 0; x < 1001; x++) {
                                            lt::vec3 wi = lt::polar_to_card(app_data.theta_i, 0.);
                                            lt::vec3 wo = lt::polar_to_card(th[x], 0.);
                                            lt::vec3 rgb = brdf->eval(wi, wo);
                                            float l = (rgb.x + rgb.y + rgb.z) / 3.;
                                            xs[x] = wo.x * l;
                                            ys[x] = wo.z * l;
                                        }
                                        ImPlot::PlotLine((brdf->type + "#" + std::to_string(i)).c_str(), xs, ys, 1001);


                                    }
                                    ImPlot::EndPlot();
                                }

                                if (ImPlot::BeginPlot(app_data.brdfs[app_data.current_brdf_idx]->type.c_str(), "x", "y")) {


                                    lt::vec3 rgb[1001];
                                    for (int x = 0; x < 1001; x++) {
                                        lt::vec3 wi = lt::polar_to_card(app_data.theta_i, 0.);
                                        lt::vec3 wo = lt::polar_to_card(th[x], 0.);
                                        rgb[x] = app_data.brdfs[app_data.current_brdf_idx]->eval(wi, wo);
                                    }

                                    const char* col_name[3] = { "r", "g", "b" };
                                    const ImVec4 col[3] = { ImVec4(1., 0., 0., 1.), ImVec4(0., 1., 0., 1.), ImVec4(0., 0., 1., 1.) };
                                    for (int c = 0; c < 3; c++) {
                                        static float xs[1001];
                                        static float ys[1001];
                                        for (int x = 0; x < 1001; x++) {
                                            lt::vec3 wo = lt::polar_to_card(th[x], 0.);
                                            xs[x] = wo.x * rgb[x][c];
                                            ys[x] = wo.z * rgb[x][c];
                                        }
                                        ImPlot::SetNextLineStyle(col[c]);
                                        ImPlot::PlotLine(col_name[c], xs, ys, 1001);

                                    }

                                    ImPlot::EndPlot();
                                }

                                ImPlot::EndSubplots();
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Theta I"))
                        {
                            static ImPlotSubplotFlags flags = ImPlotSubplotFlags_LinkRows | ImPlotSubplotFlags_LinkCols | ImPlotSubplotFlags_LinkAllX | ImPlotSubplotFlags_LinkAllY;
                            if (ImPlot::BeginSubplots("##AxisLinking", 2, 1, ImVec2(-1, -1), flags)) {


                                std::vector<float> th = lt::linspace<float>(0., 0.5 * lt::pi, 1001);

                                if (ImPlot::BeginPlot("", "x", "y")) {

                                    for (int i = 0; i < app_data.brdfs.size(); i++) {
                                        std::shared_ptr<lt::Brdf> brdf = app_data.brdfs[i];

                                        static float xs[1001];
                                        for (int x = 0; x < 1001; x++) {
                                            lt::vec3 wi = lt::polar_to_card(app_data.theta_i, 0.);
                                            lt::vec3 wo = lt::polar_to_card(th[x], 0.);
                                            lt::vec3 rgb = brdf->eval(wi, wo);
                                            xs[x] = (rgb.x + rgb.y + rgb.z) / 3.;

                                        }
                                        ImPlot::PlotLine((brdf->type + "#" + std::to_string(i)).c_str(), th.data(), xs, 1001);


                                    }
                                    ImPlot::EndPlot();
                                }

                                if (ImPlot::BeginPlot(app_data.brdfs[app_data.current_brdf_idx]->type.c_str(), "Theta O", "y")) {


                                    static float r[1001];
                                    static float g[1001];
                                    static float b[1001];
                                    for (int x = 0; x < 1001; x++) {
                                        lt::vec3 wi = lt::polar_to_card(app_data.theta_i, 0.);
                                        lt::vec3 wo = lt::polar_to_card(th[x], 0.);
                                        lt::vec3 rgb = app_data.brdfs[app_data.current_brdf_idx]->eval(wi, wo);
                                        r[x] = rgb.x;
                                        g[x] = rgb.y;
                                        b[x] = rgb.z;

                                    }
                                    ImPlot::SetNextLineStyle(ImVec4(1., 0., 0., 1.));
                                    ImPlot::PlotLine("r", th.data(), r, 1001);
                                    ImPlot::SetNextLineStyle(ImVec4(0., 1., 0., 1.));
                                    ImPlot::PlotLine("g", th.data(), g, 1001);
                                    ImPlot::SetNextLineStyle(ImVec4(0., 0., 1., 1.));
                                    ImPlot::PlotLine("b", th.data(), b, 1001);

                                    ImPlot::EndPlot();
                                }

                                ImPlot::EndSubplots();
                            }

                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Sampling"))
                {
                    lt::vec3 wi = lt::polar_to_card(app_data.theta_i, app_data.phi_i);

                                        
                    std::vector<float> th = lt::linspace<float>(0, 0.5 * lt::pi, app_data.s_brdf_sampling->h);
                    std::vector<float> ph = lt::linspace<float>(0, 2. * lt::pi, app_data.s_brdf_sampling->w);

                    for (int i = 0; i < 10000; i++) {
                        lt::vec3 wo = app_data.brdfs[app_data.current_brdf_idx]->sample(wi, app_data.sampler);
                        float phi = std::atan2(wo.y, wo.x);
                        phi = phi < 0 ? 2 * lt::pi + phi : phi;
                        float x = phi / (2. * lt::pi) * (float)app_data.s_brdf_sampling->w;
                        float y = std::acos(wo.z) / (0.5 * lt::pi) * (float)app_data.s_brdf_sampling->h;
                        if( y < app_data.s_brdf_sampling->h )
                            app_data.s_brdf_sampling->add(int(x),int(y),lt::Spectrum(1));
                    }
                    

                    for (int x = 0; x < app_data.s_brdf_sampling_pdf->w; x++) {
                        for (int y = 0; y < app_data.s_brdf_sampling_pdf->h; y++) {
                            float jw = 2. * lt::pi * (app_data.sampler.next_float() - 0.5) / (float)app_data.s_brdf_sampling_pdf->w;
                            float jh = 0.5 * lt::pi * (app_data.sampler.next_float() - 0.5) / (float)app_data.s_brdf_sampling_pdf->h;
                            //float jw = 0.f;
                            //float jh = 0.f;

                            lt::vec3 wo = lt::polar_to_card(th[y] + jh, ph[x] + jw);
                            app_data.s_brdf_sampling_pdf->add(x,y, lt::Spectrum(app_data.brdfs[app_data.current_brdf_idx]->pdf(wi, wo)));
                            app_data.s_brdf_sampling_diff->set(x,y,glm::abs(app_data.s_brdf_sampling_pdf->get(x, y) - app_data.s_brdf_sampling->get(x, y)));
                        }
                    }

                    app_data.rs_brdf_sampling.update_data();
                    app_data.rs_brdf_sampling_pdf.update_data();
                    app_data.rs_brdf_sampling_diff.update_data();

                    if (ImPlot::BeginPlot("##sample", "", "", ImVec2(-1, 0.), ImPlotFlags_Equal, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
                        ImPlot::PlotImage("", (ImTextureID)app_data.rs_brdf_sampling.spec_id, ImVec2(0, 0), ImVec2(app_data.rs_brdf_sampling.sensor->w, app_data.rs_brdf_sampling.sensor->h));
                        ImPlot::EndPlot();
                    }

                    if (ImPlot::BeginPlot("##sample_pdf", "", "", ImVec2(-1, 0.), ImPlotFlags_Equal, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
                        ImPlot::PlotImage("", (ImTextureID)app_data.rs_brdf_sampling_pdf.spec_id, ImVec2(0, 0), ImVec2(app_data.rs_brdf_sampling_pdf.sensor->w, app_data.rs_brdf_sampling_pdf.sensor->h));
                        ImPlot::EndPlot();
                    }

                    if (ImPlot::BeginPlot("##sample_diff", "", "", ImVec2(-1, 0.), ImPlotFlags_Equal, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
                        ImPlot::PlotImage("", (ImTextureID)app_data.rs_brdf_sampling_diff.spec_id, ImVec2(0, 0), ImVec2(app_data.rs_brdf_sampling_diff.sensor->w, app_data.rs_brdf_sampling_diff.sensor->h));
                        ImPlot::EndPlot();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Directional Light"))
                {
                    auto ms_per_pix = app_data.ren_dir_light.render(app_data.scn_dir_light);
                    //std::cout << ms_per_pix << std::endl;
                    app_data.rsen_dir_light.update_data();

                    if (ImPlot::BeginPlot("##image","","", ImVec2(-1,-1),ImPlotFlags_Equal, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
                        ImPlot::PlotImage("", (ImTextureID)app_data.rsen_dir_light.spec_id, ImVec2(0, 0), ImVec2(app_data.ren_dir_light.sensor->w, app_data.ren_dir_light.sensor->h));
                        ImPlot::EndPlot();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Global Illumination"))
                {
                    //auto ms_per_pix = app_data.ren_glo_ill.render(app_data.scn_glo_ill);
                    //std::cout << ms_per_pix << std::endl;
                    app_data.ren_glo_ill.render(app_data.scn_glo_ill);
                    app_data.rsen_glo_ill.update_data();

                    if (ImPlot::BeginPlot("##image", "", "", ImVec2(-1, -1), ImPlotFlags_Equal, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
                        ImPlot::PlotImage("", (ImTextureID)app_data.rsen_glo_ill.spec_id, ImVec2(0, 0), ImVec2(app_data.ren_glo_ill.sensor->w, app_data.ren_glo_ill.sensor->h));
                        ImPlot::EndPlot();
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();

            }
            ImGui::EndChild();

            ImGui::BeginChild("scene param", ImVec2(0, 0), true);

            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45);
            NEED_RESET(ImGui::SliderAngle("th_i", &app_data.theta_i, 0, 90));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45);
            NEED_RESET(ImGui::SliderAngle("ph_i", &app_data.phi_i, 0, 360));
            
            std::shared_ptr<lt::DirectionnalLight> dl = std::static_pointer_cast<lt::DirectionnalLight>(app_data.scn_dir_light.lights[0]) ;
            dl->dir = -lt::vec3(std::sin(app_data.theta_i) * std::cos(app_data.phi_i), std::cos(app_data.theta_i),std::sin(app_data.theta_i) * std::sin(app_data.phi_i));

            ImGui::EndChild();

            ImGui::EndGroup();
        }
    }
    ImGui::End();
}



// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "BRDF Viewer", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.Fonts->AddFontFromFileTTF("../../../3rd_party/Roboto-Regular.ttf", 15.);
    
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.0f, 1.00f);
    //ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
   
    //lt::GGXMicrosurface ggx_ms(0.4,0.5);


    AppData app_data;
    AppInit(app_data);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();
        //ImPlot::ShowDemoWindow();

        AppLayout(window, app_data);

        // Rendering
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }


    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    

    return 0;
}