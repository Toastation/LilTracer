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


struct RenderSensor {
    GLuint spec_id;
    GLuint count_id;
    std::shared_ptr<lt::Sensor> sensor = nullptr;
    bool initialized = false;

    GLuint fb_id;

    bool update_data() {

        if (!initialized) {
            return false;
        }

        /*glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
        glViewport(0, 0, sensor->w, sensor->h);*/


        glBindTexture(GL_TEXTURE_2D, spec_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sensor->w, sensor->h, 0, GL_RGB, GL_FLOAT, sensor->data.data());
        glBindTexture(GL_TEXTURE_2D, count_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sensor->w, sensor->h, 0, GL_RED, GL_UNSIGNED_SHORT, sensor->count.data());
        return true;
    }

    bool initialize()
    {
        
        //glGenFramebuffers(1, &fb_id);
        //glBindFramebuffer(GL_FRAMEBUFFER, fb_id);


        glGenTextures(1, &spec_id);
        glBindTexture(GL_TEXTURE_2D, spec_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &count_id);
        glBindTexture(GL_TEXTURE_2D, count_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        
        //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0);
        //GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        //glDrawBuffers(1, DrawBuffers);
        //
        //// Always check that our framebuffer is ok
        //if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        //    return false;
        //
        
        initialized = true;
        return true;
    }

    ~RenderSensor() {
        if (initialized) {
            glDeleteTextures(1, &spec_id);
            glDeleteTextures(1, &count_id);
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

    lt::Scene* scn_dir_light;
    std::shared_ptr<lt::Camera> cam_dir_light;
    std::shared_ptr<lt::Sensor> sen_dir_light;
    lt::Sampler* sam_dir_light;
    lt::Integrator* int_dir_light;
    RenderSensor rsen_dir_light;

    lt::Scene    scn_glo_ill;
    lt::Renderer ren_glo_ill;
    RenderSensor rsen_glo_ill;
};

void AppInit(AppData& app_data) {
    lt::cornell_box(app_data.scn_glo_ill, app_data.ren_glo_ill);
    app_data.rsen_glo_ill.sensor = app_data.ren_glo_ill.sensor;
    app_data.rsen_glo_ill.initialize();

    app_data.s_brdf_slice = std::make_shared<lt::Sensor>(256, 64);
    app_data.rs_brdf_slice.sensor = app_data.s_brdf_slice;
    app_data.rs_brdf_slice.initialize();
    app_data.current_brdf_idx = 0;

    app_data.brdfs.push_back(lt::Factory<lt::Brdf>::create("Diffuse"));
    app_data.brdfs.push_back(lt::Factory<lt::Brdf>::create("RoughConductor"));
    app_data.brdfs.push_back(lt::Factory<lt::Brdf>::create("TestBrdf"));

    app_data.scn_dir_light = new lt::Scene();
    lt::vec3 ld = lt::vec3(std::sin(app_data.theta_i) * std::cos(app_data.phi_i), std::sin(app_data.theta_i) * std::sin(app_data.phi_i), std::cos(app_data.theta_i));
    app_data.scn_dir_light->lights.push_back(lt::Factory<lt::Light>::create("DirectionnalLight"));
    
    std::shared_ptr<lt::Sphere> sph = std::make_shared<lt::Sphere>();
    sph->pos = lt::vec3(0);
    sph->rad = 1;
    sph->brdf = app_data.brdfs[app_data.current_brdf_idx];
    app_data.scn_dir_light->shapes.push_back(sph);

    std::shared_ptr<lt::PerspectiveCamera> cam = std::make_shared<lt::PerspectiveCamera>();
    cam->pos = lt::vec3(10., 0., 0.);
    cam->center = lt::vec3(0.);
    cam->fov = 50.;
    cam->aspect = 1.;
    cam->init();
    
    app_data.cam_dir_light = cam;
    app_data.sen_dir_light = std::make_shared<lt::Sensor>(512, 512);
    app_data.int_dir_light = new lt::DirectIntegrator();
    app_data.sam_dir_light = new lt::Sampler();
    app_data.rsen_dir_light.sensor = app_data.sen_dir_light;
    app_data.rsen_dir_light.initialize();
}


void brdf_slice(std::shared_ptr<lt::Brdf> brdf, float th_i, float ph_i, std::shared_ptr<lt::Sensor> sensor) {
    
    std::vector<float> th = lt::linspace<float>(0, 0.5 * lt::pi, sensor->h);
    std::vector<float> ph = lt::linspace<float>(0, 2.  * lt::pi, sensor->w);
    
    lt::vec3 wi = lt::polar_to_card(th_i, ph_i);
    for (int x = 0; x < sensor->w; x++) {
        for (int y = 0; y < sensor->h; y++) {
            lt::vec3 wo = lt::polar_to_card(th[y], ph[x]);
            sensor->data[y * sensor->w + x] = brdf->eval(wi, wo);
        }
    }

}





static void AppLayout(GLFWwindow* window, AppData& app_data)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ImGui::SetNextWindowSize(ImVec2(width, height)); // ensures ImGui fits the GLFW window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    
    bool open = true;
    if (ImGui::Begin("Simple layout", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        
        {
            ImGui::BeginGroup();
            
            ImGui::BeginChild("top pane", ImVec2(250, ImGui::GetContentRegionAvail().y*0.25),true);
            for (int i = 0; i < app_data.brdfs.size(); i++)
            {
                if (ImGui::Selectable( (app_data.brdfs[i]->type + "##" + std::to_string(app_data.brdfs[i]->id)).c_str(), app_data.current_brdf_idx == i)) {
                    app_data.current_brdf_idx = i;
                    app_data.scn_dir_light->shapes[0]->brdf = app_data.brdfs[i];
                }
            }

            ImGui::EndChild();
            ImGui::BeginChild("bottom pane", ImVec2(250, 0), true);
            
            std::shared_ptr<lt::Brdf> cur_brdf = app_data.brdfs[app_data.current_brdf_idx];

            for (int i = 0; i < cur_brdf->params.count; i++) {
                switch (cur_brdf->params.types[i])
                {
                case lt::Params::Type::FLOAT:
                    ImGui::DragFloat(cur_brdf->params.names[i].c_str(), (float*)cur_brdf->params.ptrs[i]);
                    break;
                case lt::Params::Type::VEC3:
                    ImGui::ColorEdit3(cur_brdf->params.names[i].c_str(), (float*)cur_brdf->params.ptrs[i]);
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
                if (ImGui::BeginTabItem("Polar plot"))
                {
                    static ImPlotSubplotFlags flags = ImPlotSubplotFlags_LinkRows | ImPlotSubplotFlags_LinkCols | ImPlotSubplotFlags_LinkAllX | ImPlotSubplotFlags_LinkAllY;
                    if (ImPlot::BeginSubplots("##AxisLinking", 2, 1,ImVec2(-1,-1), flags)) {
                        

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
                                ImPlot::PlotLine(brdf->type.c_str(), th.data(), xs, 1001);


                            }
                            ImPlot::EndPlot();
                        }

                        if (ImPlot::BeginPlot(app_data.brdfs[app_data.current_brdf_idx]->type.c_str(), "x", "y")) {


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
                if (ImGui::BeginTabItem("Sampling"))
                {
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Directional Light"))
                {
                    
                    app_data.int_dir_light->render(app_data.cam_dir_light, app_data.sen_dir_light, *app_data.scn_dir_light, *app_data.sam_dir_light);
                    app_data.rsen_dir_light.update_data();

                    if (ImPlot::BeginPlot("##image","","", ImVec2(-1,-1),ImPlotFlags_Equal, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
                        ImPlot::PlotImage("", (ImTextureID)app_data.rsen_dir_light.spec_id, ImVec2(0, 0), ImVec2(app_data.sen_dir_light->w, app_data.sen_dir_light->h));
                        ImPlot::EndPlot();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Global Illumination"))
                {
                    app_data.ren_glo_ill.integrator->render(app_data.ren_glo_ill.camera, app_data.ren_glo_ill.sensor, app_data.scn_glo_ill, *app_data.ren_glo_ill.sampler);
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
            ImGui::SliderAngle("th_i", &app_data.theta_i, 0, 90);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.45);
            ImGui::SliderAngle("ph_i", &app_data.phi_i, 0, 360);
            
            std::shared_ptr<lt::DirectionnalLight> dl = std::static_pointer_cast<lt::DirectionnalLight>(app_data.scn_dir_light->lights[0]) ;
            dl->dir = lt::vec3(std::sin(app_data.theta_i) * std::cos(app_data.phi_i), std::cos(app_data.theta_i),std::sin(app_data.theta_i) * std::sin(app_data.phi_i));

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

    //io.Fonts->AddFontFromFileTTF("../../../3rd_party/Roboto-Regular.ttf", 15.);
    
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
   

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