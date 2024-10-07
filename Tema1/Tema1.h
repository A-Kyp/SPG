#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"
#include "core/gpu/particle_effect.h"

#include <string>


namespace m2
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();

        void Init() override;

    private:
        void CreateFramebuffer(int width, int height);
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        unsigned int UploadCubeMapTexture(const std::string& pos_x, const std::string& pos_y, const std::string& pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        // lab 4
        void ResetParticlesFireworks(int xSize, int ySize, int zSize);
        void ResetParticlesRainSnow(int xSize, int ySize, int zSize);
        void ResetParticlesFire(float radius);
        void LoadShader(const std::string& name, 
            const std::string& VS, const std::string& FS, const std::string& GS="",
            bool hasGeomtery = false);

    private:
        int cubeMapTextureID;
        float angle;
        unsigned int framebuffer_object;
        unsigned int color_texture;
        unsigned int depth_texture;
        unsigned int type;



        //lab 2 - bezier
        // Bezier 1
        glm::vec3 control_p0_1 = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 control_p1_1 = glm::vec3(-1.03, -0.12, 0.0);
        glm::vec3 control_p2_1 = glm::vec3(-0.39, 1.36, 0.0);
        glm::vec3 control_p3_1 = glm::vec3(-1.4, 0.62, 0.0);
        // // Bezier 1
        // glm::vec3 control_p0_1 = glm::vec3(0.0, 0.0, 0.0);
        // glm::vec3 control_p1_1 = glm::vec3(-2.1, 1.44, 0.0);
        // glm::vec3 control_p2_1 = glm::vec3(0.63, 1.44, 0.0);
        // glm::vec3 control_p3_1 = glm::vec3(-0.8, 2.83, 0.0);
        // Bezier 2
        glm::vec3 control_p0_2 = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 control_p1_2 = glm::vec3(-1.28, 0.36, 0.0);
        glm::vec3 control_p2_2 = glm::vec3(-0.55, 1.2, 0.0);
        glm::vec3 control_p3_2 = glm::vec3(-2.05, 1.2, 0.0);
        // Bezier 3
        glm::vec3 control_p0_3 = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 control_p1_3 = glm::vec3(-0.9, 0.65, 0.0);
        glm::vec3 control_p2_3 = glm::vec3(-0.68, 1.02, 0.0);
        glm::vec3 control_p3_3 = glm::vec3(-0.12, 1.35, 0.0);
        // Bezier 4
        glm::vec3 control_p0_4 = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 control_p1_4 = glm::vec3(-0.75, 0.14, 0.0);
        glm::vec3 control_p2_4 = glm::vec3(-0.46, 0.6, 0.0);
        glm::vec3 control_p3_4 = glm::vec3(-1.31, 0.28, 0.0);
        // Bezier 5
        glm::vec3 control_p0_5 = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 control_p1_5 = glm::vec3(-1.61, 0.0, 0.0);
        glm::vec3 control_p2_5 = glm::vec3(-0.13, 1.4, 0.0);
        glm::vec3 control_p3_5 = glm::vec3(-1.62, 1.42, 0.0);

        //lab 4 - sisteme de particule
        glm::mat4 modelMatrix;
        glm::vec3 generator_position;
        GLenum polygonMode;
        int scene = 0;
        float offset;
        int location;

        //mirror
        float mirror_x = 0;
        float mirror_y = 0.3f;
        float mirror_z = -6;

        float mirror_rotate_x = 0;
        float mirror_rotate_y = 0;
        float mirror_rotate_z = 0;

        float mirror_speed = 4;
        float mirror_rot_angle = 0.2f;

        //teapot
        float tx = 1;
        float ty = 1;
        float tz = 1;

        //
        int draw_outline = 0;
        int draw_particles = 0;
        int b_5_dif = 0;
        glm::mat4 cubeView2[6];

    };
}   // namespace m2
