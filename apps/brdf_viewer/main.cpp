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


struct RenderTexture {
    GLuint _id;
    int _w;
    int _h;
    std::vector<lt::vec3> _data;
    bool _initialized = false;

    bool update_data(std::vector<lt::vec3> data, int w, int h) {
        _data = data;
        _w = w;
        _h = h;

        if (_initialized) {
            glBindTexture(GL_TEXTURE, _id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _w, _h, 0, GL_RGB, GL_FLOAT, _data.data());
            return true;
        }
        return false;
    }

    bool initialize()
    {
        glGenTextures(1, &_id);
        glBindTexture(GL_TEXTURE_2D, _id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        _initialized = true;
        return true;
    }

    ~RenderTexture() {
        if (_initialized) {
            glDeleteTextures(1, &_id);
        }
    }
};


struct AppData {
    RenderTexture rt_brdf_slice;
    int current_brdf_idx;
    std::vector<lt::Brdf*> brdfs;
    float theta_i = 0.5;
    float phi_i = 0.;
};

void AppInit(AppData& app_data) {
    app_data.rt_brdf_slice.initialize();
    app_data.current_brdf_idx = 0;

    app_data.brdfs.push_back(new lt::Diffuse);
    app_data.brdfs.push_back(new lt::RoughConductor);
}


std::vector<lt::vec3> brdf_slice(lt::Brdf* brdf, float th_i, float ph_i, int width, int height) {
    std::vector<lt::vec3> arr;
    arr.resize(width * height);

    std::vector<float> th = lt::linspace<float>(0, 0.5 * lt::pi, height);
    std::vector<float> ph = lt::linspace<float>(0, 2.  * lt::pi, width);
    
    lt::vec3 wi = lt::polar_to_card(th_i, ph_i);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            lt::vec3 wo = lt::polar_to_card(th[y], ph[x]);
            arr[y * width + x] = brdf->eval(wi, wo);
        }
    }
    return arr;
}





static void AppLayout(GLFWwindow* window, AppData& app_data)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ImGui::SetNextWindowSize(ImVec2(width, height)); // ensures ImGui fits the GLFW window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    
    bool open = true;
    if (ImGui::Begin("Simple layout", &open, ImGuiWindowFlags_NoTitleBar))
    {
        
        {
            ImGui::BeginGroup();
            
            ImGui::BeginChild("top pane", ImVec2(250, ImGui::GetContentRegionAvail().y*0.25),true);
            for (int i = 0; i < app_data.brdfs.size(); i++)
            {
                if (ImGui::Selectable(app_data.brdfs[i]->name.c_str(), app_data.current_brdf_idx == i))
                    app_data.current_brdf_idx = i;
            }
            ImGui::Separator();
            ImGui::EndChild();
            ImGui::BeginChild("bottom pane", ImVec2(250, 0), true);
            
            lt::Brdf* cur_brdf = app_data.brdfs[app_data.current_brdf_idx];

            for (int i = 0; i < cur_brdf->params.count; i++) {
                switch (cur_brdf->params.types[i])
                {
                case lt::Params::Type::FLOAT:
                    ImGui::DragFloat(cur_brdf->params.names[i].c_str(), (float*)cur_brdf->params.ptrs[i]);
                    break;
                case lt::Params::Type::VEC3:
                    ImGui::ColorEdit3(cur_brdf->params.names[i].c_str(), (float*)cur_brdf->params.ptrs[i]);
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
                    int tex_width = 1024;
                    int tex_height = 256;
                    lt::Brdf* cur_brdf = app_data.brdfs[app_data.current_brdf_idx];

                    app_data.rt_brdf_slice.update_data(brdf_slice(cur_brdf, app_data.theta_i, app_data.phi_i, tex_width, tex_height), tex_width, tex_height);
                 
                    ImGui::SetCursorPos( ImVec2( (ImGui::GetWindowSize().x - tex_width)*0.5, (ImGui::GetWindowSize().y - tex_height)*0.5));
                    ImGui::Image((void*)(intptr_t)app_data.rt_brdf_slice._id, ImVec2(app_data.rt_brdf_slice._w, app_data.rt_brdf_slice._h));


                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Polar plot"))
                {
                 
                    if (ImPlot::BeginPlot("Line Plots", ImVec2(-1, -1))) {

                        ImPlot::SetupLegend(ImPlotLocation_NorthEast);
                        ImPlot::SetupAxes("x", "y");
                        std::vector<float> th = lt::linspace<float>(0., 0.5 * lt::pi, 1001);

                        for (int i = 0; i < app_data.brdfs.size(); i++) {
                            lt::Brdf* brdf = app_data.brdfs[i];

                            static float xs[1001];
                            for (int x = 0; x < 1001; x++) {
                                lt::vec3 wi = lt::polar_to_card(app_data.theta_i, 0.);
                                lt::vec3 wo = lt::polar_to_card(th[x], 0.);
                                xs[x] = brdf->eval(wi,wo).x;
                                
                            }
                            ImPlot::PlotLine(brdf->name.c_str(), th.data(),  xs , 1001);
                            

                        }
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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    io.Fonts->AddFontFromFileTTF("../../../3rd_party/Roboto-Regular.ttf", 15.);
    
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImPlot::CreateContext();

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
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    

    return 0;
}