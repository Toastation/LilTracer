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

struct RenderTexture {
    GLuint texture_id;
    lt::Texture<Float>* texture = nullptr;
    bool initialized = false;


    bool update_data() {

        if (!initialized) {
            return false;
        }


        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RED, GL_FLOAT, (float*)texture->data);
        return true;
    }

    bool initialize()
    {

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        initialized = true;
        return true;
    }

    ~RenderTexture() {
        if (initialized) {
            glDeleteTextures(1, &texture_id);
        }
    }
};


struct AppData {
    std::shared_ptr<lt::EnvironmentLight> envmap;
    RenderTexture rt;
    std::vector<lt::vec3> samples;
    std::vector<float> x;
    std::vector<float> y;
    int nsample;
    lt::Sampler sampler;

};

void AppInit(AppData& app_data) {
    need_reset = false;

    app_data.envmap = std::make_shared<lt::EnvironmentLight>();
    lt::load_texture_exr("kloofendal_48d_partly_cloudy_puresky_1k.exr", app_data.envmap->envmap);
    app_data.envmap->init();
    app_data.rt.texture = &app_data.envmap->density;
    app_data.rt.initialize();

    app_data.sampler.seed(0);

    app_data.nsample = 20000;
    app_data.samples.resize(app_data.nsample);
    app_data.x.resize(app_data.nsample);
    app_data.y.resize(app_data.nsample);
    for (int i = 0; i < app_data.nsample; i++) {
        app_data.samples[i] = app_data.envmap->sample_light_direction(app_data.sampler);
        app_data.x[i] = (std::atan2(app_data.samples[i].y, app_data.samples[i].x) + lt::pi) / (2. * lt::pi) * (float)app_data.envmap->envmap.w; // app_data.samples[i].x;//
        app_data.y[i] = std::acos(app_data.samples[i].z) / lt::pi * (float)app_data.envmap->envmap.h; // app_data.samples[i].y;// 
    }

}


static void AppLayout(GLFWwindow* window, AppData& app_data)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ImGui::SetNextWindowSize(ImVec2(width, height)); // ensures ImGui fits the GLFW window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    

    if (need_reset) {
        need_reset = false;
    }
    bool open = true;
    if (ImGui::Begin("Simple layout", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        app_data.rt.update_data();

        if (ImPlot::BeginPlot("##image", "", "", ImVec2(-1, -1), ImPlotFlags_Equal)) {

            ImPlot::PlotImage("BRDF slice", (ImTextureID)app_data.rt.texture_id, ImVec2(0, 0), ImVec2(app_data.envmap->envmap.w, app_data.envmap->envmap.h));
            ImPlot::PlotScatter("Samples", app_data.x.data(), app_data.y.data(), app_data.nsample);
            ImPlot::EndPlot();

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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Envmap sampling", nullptr, nullptr);
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