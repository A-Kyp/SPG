#include "lab_m2/Tema1/Tema1.h"

#include <vector>
#include <iostream>

#include "stb/stb_image.h"

using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

struct Particle
{
    glm::vec4 position;
    glm::vec4 speed;
    glm::vec4 initialPos;
    glm::vec4 initialSpeed;
    float delay;
    float initialDelay;
    float lifetime;
    float initialLifetime;

    Particle() {}

    Particle(const glm::vec4 &pos, const glm::vec4 &speed)
    {
        SetInitial(pos, speed);
    }

    void SetInitial(const glm::vec4 &pos, const glm::vec4 &speed,
        float delay = 0, float lifetime = 0)
    {
        position = pos;
        initialPos = pos;

        this->speed = speed;
        initialSpeed = speed;

        this->delay = delay;
        initialDelay = delay;

        this->lifetime = lifetime;
        initialLifetime = lifetime;
    }
};

ParticleEffect<Particle> *particleEffect2;

Tema1::Tema1()
{
    framebuffer_object = 0;
    color_texture = 0;
    depth_texture = 0;

    angle = 0;

    type = 0;
}


Tema1::~Tema1()
{
}


void Tema1::Init()
{
    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, -1, 4), glm::quat(glm::vec3(RADIANS(10), 0, 0)));
    camera->Update();

    std::string texturePath = PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube");
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders");

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("mirror");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "screen_quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("cube");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("teapot");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "teapot.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("archer");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "characters", "archer"), "Archer.fbx");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    // Create a shader program for rendering cubemap texture
    {
        Shader* shader = new Shader("CubeMap");
        shader->AddShader(PATH_JOIN(shaderPath, "CubeMap.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "CubeMap.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Create a shader program for standard rendering
    {
        Shader* shader = new Shader("ShaderNormal");
        shader->AddShader(PATH_JOIN(shaderPath, "Normal.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Normal.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Create a shader program for creating a CUBEMAP
    {
        Shader* shader = new Shader("Framebuffer");
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.GS.glsl"), GL_GEOMETRY_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    cubeMapTextureID = UploadCubeMapTexture(
        PATH_JOIN(texturePath, "pos_x.png"),
        PATH_JOIN(texturePath, "pos_y.png"),
        PATH_JOIN(texturePath, "pos_z.png"),
        PATH_JOIN(texturePath, "neg_x.png"),
        PATH_JOIN(texturePath, "neg_y.png"),
        PATH_JOIN(texturePath, "neg_z.png"));

    // Load textures
    {
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS), "characters", "archer", "Akai_E_Espiritu.fbm", "akai_diffuse.png");
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "particle2.png");

        // TODO(student): Load images "rain.png", "snowflake.png" and "fire2.png" as
        // textures, similar to "particle2.png", loaded above. The images can be
        // found in the same directory as "particle2.png"
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "rain.png");
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "snowflake.png");
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "fire.png");
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "fire2.png");
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "nirvana.png");


    }

    // Create the framebuffer on which the scene is rendered from the perspective of the mesh
    // Texture size must be cubic
    CreateFramebuffer(1024, 1024);

    LoadShader("Fireworks", "Particle_fireworks", "Particle_simple", "Particle", true);
    LoadShader("Contur", "Contur", "Contur", "Contur", true);
    
    ResetParticlesFireworks(20,20,20);

    generator_position = glm::vec3(0, 0, 0);
    scene = 0;
    offset = 0.05f;
}

void Tema1::ResetParticlesFireworks(int xSize, int ySize, int zSize)
{
    unsigned int nrParticles = 100;

    particleEffect2 = new ParticleEffect<Particle>();
    particleEffect2->Generate(nrParticles, true);

    auto particleSSBO = particleEffect2->GetParticleBuffer();
    Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

    for (unsigned int i = 0; i < nrParticles; i++)
    {
        glm::vec4 pos(1);
        pos.x = 0;
        pos.y = 0;
        pos.z = 0;

        glm::vec4 speed(0);
        speed.x = (rand() % 20 - 10) / 10.0f;
        speed.z = (rand() % 20 - 10) / 10.0f;
        speed.y = rand() % 2 + 2.0f;

        float lifetime = 1 + (rand() % 100 / 100.0f);
        float delay = (rand() % 100 / 100.0f) * 3.0f;

        data[i].SetInitial(pos, speed, delay, lifetime);
    }

    particleSSBO->SetBufferData(data);
}

void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::Update(float deltaTimeSeconds)
{
    // lab 6
    angle += 0.5f * deltaTimeSeconds;

    glm::mat4 cubeView[6] =
        {
            glm::lookAt(glm::vec3(mirror_x, mirror_y, mirror_z), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // +X
            glm::lookAt(glm::vec3(mirror_x, mirror_y, mirror_z), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // -X
            glm::lookAt(glm::vec3(mirror_x, mirror_y, mirror_z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), // +Y
            glm::lookAt(glm::vec3(mirror_x, mirror_y, mirror_z), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f)), // -Y
            glm::lookAt(glm::vec3(mirror_x, mirror_y, mirror_z), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // +Z
            glm::lookAt(glm::vec3(mirror_x, mirror_y, mirror_z), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // -Z
        };

    auto camera = GetSceneCamera();

    // Draw the scene in Framebuffer
    if (framebuffer_object)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
        // Set the clear color for the color buffer
        glClearColor(0, 0, 0, 1);
        // Clears the color buffer (using the previously set color) and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, 1024, 1024);

        Shader* shader;
        if(draw_outline == 0) {
            shader = shaders["Framebuffer"];
        } else {
            shader = shaders["Contur"];
        }
        shader->Use();

        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
        // cout << GetSceneCamera()->m_transform->GetLocalOZVector();
        glUniform3fv(glGetUniformLocation(shader->program, "visual_vector"), 1, glm::value_ptr(GetSceneCamera()->m_transform->GetLocalOZVector()));

        //draw cubemap in frameebuffer
        {
            glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            glUniform1i(glGetUniformLocation(shader->program, "texture_cubemap"), 1);

            glUniform1i(glGetUniformLocation(shader->program, "cube_draw"), 1);

            meshes["cube"]->Render();
        }


        // draw archers in framebuffer
        for (int i = 0; i < 5; i++)
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix *= glm::rotate(glm::mat4(1), angle + i * glm::radians(360.0f) / 5, glm::vec3(0, 1, 0));
            modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(3, -1, 0));
            modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));
            modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(0.01f));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));



            glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "viewMatrices"), 6, GL_FALSE, glm::value_ptr(cubeView[0]));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("Akai_E_Espiritu.fbm\\akai_diffuse.png")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

            glUniform1i(glGetUniformLocation(shader->program, "cube_draw"), 0);

            meshes["archer"]->Render();
        }

        // draw teapot in framebuffer
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(tx, ty, tz));
            modelMatrix *= glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));


            glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "viewMatrices"), 6, GL_FALSE, glm::value_ptr(cubeView[0]));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("nirvana.png")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

            meshes["teapot"]->Render();
        }

        // draw sysrem in framebuffer
        if(draw_particles == 1 && draw_outline != 1)
        {
            // lab 2    
            Shader *shader2 = shaders["Fireworks"];
            shader2->Use();

            glUniformMatrix4fv(glGetUniformLocation(shader2->GetProgramID(), "viewMatrices"), 6, GL_FALSE, glm::value_ptr(cubeView[0]));

            // Send uniforms to shaders
            
            if (b_5_dif == 1) {
                // Bezier 1
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_1"), control_p0_1.x, control_p0_1.y, control_p0_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_1"), control_p1_1.x, control_p1_1.y, control_p1_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_1"), control_p2_1.x, control_p2_1.y, control_p2_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_1"), control_p3_1.x, control_p3_1.y, control_p3_1.z);
                
                // Bezier 2
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_2"), control_p0_2.x, control_p0_2.y, control_p0_2.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_2"), control_p1_2.x, control_p1_2.y, control_p1_2.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_2"), control_p2_2.x, control_p2_2.y, control_p2_2.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_2"), control_p3_2.x, control_p3_2.y, control_p3_2.z);
                
                // Bezier 1
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_3"), control_p0_3.x, control_p0_3.y, control_p0_3.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_3"), control_p1_3.x, control_p1_3.y, control_p1_3.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_3"), control_p2_3.x, control_p2_3.y, control_p2_3.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_3"), control_p3_3.x, control_p3_3.y, control_p3_3.z);
                
                // Bezier 1
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_4"), control_p0_4.x, control_p0_4.y, control_p0_4.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_4"), control_p1_4.x, control_p1_4.y, control_p1_4.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_4"), control_p2_4.x, control_p2_4.y, control_p2_4.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_4"), control_p3_4.x, control_p3_4.y, control_p3_4.z);
                
                // Bezier 1
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_5"), control_p0_5.x, control_p0_5.y, control_p0_5.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_5"), control_p1_5.x, control_p1_5.y, control_p1_5.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_5"), control_p2_5.x, control_p2_5.y, control_p2_5.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_5"), control_p3_5.x, control_p3_5.y, control_p3_5.z);
            } else {
                // Bezier 1
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_1"), control_p0_1.x, control_p0_1.y, control_p0_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_1"), control_p1_1.x, control_p1_1.y, control_p1_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_1"), control_p2_1.x, control_p2_1.y, control_p2_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_1"), control_p3_1.x, control_p3_1.y, control_p3_1.z);
                
                // Bezier 2
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_2"), control_p0_1.x, control_p0_1.y, control_p0_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_2"), control_p1_1.x, control_p1_1.y, control_p1_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_2"), control_p2_1.x, control_p2_1.y, control_p2_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_2"), control_p3_1.x, control_p3_1.y, control_p3_1.z);
                
                // Bezier 1
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_3"), control_p0_1.x, control_p0_1.y, control_p0_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_3"), control_p1_1.x, control_p1_1.y, control_p1_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_3"), control_p2_1.x, control_p2_1.y, control_p2_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_3"), control_p3_1.x, control_p3_1.y, control_p3_1.z);
                
                // Bezier 1
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_4"), control_p0_1.x, control_p0_1.y, control_p0_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_4"), control_p1_1.x, control_p1_1.y, control_p1_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_4"), control_p2_1.x, control_p2_1.y, control_p2_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_4"), control_p3_1.x, control_p3_1.y, control_p3_1.z);
                
                // Bezier 1
                glUniform3f(glGetUniformLocation(shader2->program, "control_p0_5"), control_p0_1.x, control_p0_1.y, control_p0_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p1_5"), control_p1_1.x, control_p1_1.y, control_p1_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p2_5"), control_p2_1.x, control_p2_1.y, control_p2_1.z);
                glUniform3f(glGetUniformLocation(shader2->program, "control_p3_5"), control_p3_1.x, control_p3_1.y, control_p3_1.z);
            }

            
            
            // lab 4
            glLineWidth(3);

            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            if (scene == 0)
            {
                auto shader = shaders["Fireworks"];
                if (shader->GetProgramID())
                {
                    shader->Use();

                    TextureManager::GetTexture("particle2.png")->BindToTextureUnit(GL_TEXTURE0);
                    particleEffect2->Render(GetSceneCamera(), shader);

                    // TODO(student): Send uniforms generator_position,
                    // deltaTime and offset to the shader
                    int location = glGetUniformLocation(shader->program, "deltaTime");
                    glUniform1f(location, deltaTimeSeconds);

                    location = glGetUniformLocation(shader->program, "offset");
                    glUniform1f(location, offset);

                    location = glGetUniformLocation(shader->program, "generator_position");
                    //glUniform3f(location, generator_position.x, generator_position.y, generator_position.z);
                    glUniform3fv(location, 1, glm::value_ptr(generator_position));

                }
            }

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        //reset drawing to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, window->GetResolution().x, window->GetResolution().y);

    // Draw the cubemap
    {
        Shader* shader = shaders["ShaderNormal"];
        shader->Use();

        glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
        int loc_texture = shader->GetUniformLocation("texture_cubemap");
        glUniform1i(loc_texture, 0);

        meshes["cube"]->Render();
    }

    // Draw five archers around the mesh
    for (int i = 0; i < 5; i++)
    {
        Shader* shader = shaders["Simple"];
        shader->Use();

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= glm::rotate(glm::mat4(1), angle + i * glm::radians(360.0f) / 5, glm::vec3(0, 1, 0));
        modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(3, -1, 0));
        modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(0.01f));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("Akai_E_Espiritu.fbm\\akai_diffuse.png")->GetTextureID());
        glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

        meshes["archer"]->Render();
    }

    // draw teapot
    {
        Shader* shader = shaders["Simple"];
        shader->Use();

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(tx, ty, tz));
        modelMatrix *= glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("nirvana.png")->GetTextureID());
        glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

        meshes["teapot"]->Render();
    }

    // Draw the reflection on the mesh
    {
        Shader* shader = shaders["CubeMap"];
        shader->Use();

        glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(2.f, 3.0f, 1.0f));
        modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(mirror_x, mirror_y, mirror_z));
        modelMatrix *= glm::rotate(glm::mat4(1), mirror_rotate_x, glm::vec3(1, 0, 0));
        modelMatrix *= glm::rotate(glm::mat4(1), mirror_rotate_y, glm::vec3(0, 1, 0));
        modelMatrix *= glm::rotate(glm::mat4(1), mirror_rotate_z, glm::vec3(0, 0, 1));


        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        auto cameraPosition = camera->m_transform->GetWorldPosition();

        if (!color_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            int loc_texture = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture, 0);
        }

        if (color_texture) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
            int loc_texture2 = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture2, 1);
        }


        int loc_camera = shader->GetUniformLocation("camera_position");
        glUniform3f(loc_camera, cameraPosition.x, cameraPosition.y, cameraPosition.z);

        glUniform1i(shader->GetUniformLocation("type"), type);

        meshes["mirror"]->Render();
    }

    
}


void Tema1::FrameEnd()
{
    // DrawCoordinateSystem();
}


unsigned int Tema1::UploadCubeMapTexture(const std::string& pos_x, const std::string& pos_y, const std::string& pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z)
{
    int width, height, chn;

    unsigned char* data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &chn, 0);

    unsigned int textureID = 0;
    // TODO(student): Create the texture
    glGenTextures(1, &textureID);

    // TODO(student): Bind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (GLEW_EXT_texture_filter_anisotropic) {
        float maxAnisotropy;

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // TODO(student): Load texture information for each face
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_z);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        cout << "\t[NOTE] : For students : DON'T PANIC! This error should go away when completing the tasks." << std::endl;
    }

    // Free memory
    SAFE_FREE(data_pos_x);
    SAFE_FREE(data_pos_y);
    SAFE_FREE(data_pos_z);
    SAFE_FREE(data_neg_x);
    SAFE_FREE(data_neg_y);
    SAFE_FREE(data_neg_z);

    return textureID;
}

void Tema1::CreateFramebuffer(int width, int height)
{
    // TODO(student): In this method, use the attributes
    // 'framebuffer_object', 'color_texture'
    // declared in Tema1.h

    // TODO(student): Generate and bind the framebuffer
    glGenFramebuffers(1, &framebuffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);

    // TODO(student): Generate and bind the color texture
    glGenTextures(1, &color_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);

    // TODO(student): Initialize the color textures
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }

    if (color_texture) {
        //cubemap params
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (GLEW_EXT_texture_filter_anisotropic) {
            float maxAnisotropy;

            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Bind the color textures to the framebuffer as a color attachments
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        std::vector<GLenum> draw_textures;
        draw_textures.push_back(GL_COLOR_ATTACHMENT0);
        glDrawBuffers(draw_textures.size(), &draw_textures[0]);

    }

    // TODO(student): Generate and bind the depth texture
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_texture);


    // TODO(student): Initialize the depth textures
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    if (depth_texture) {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    }

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Tema1::LoadShader(const std::string& name, const std::string &VS, const std::string& FS, const std::string& GS,  bool hasGeomtery)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders");

    // Create a shader program for particle system
    {
        Shader *shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, VS + ".VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, FS + ".FS.glsl"), GL_FRAGMENT_SHADER);
        if (hasGeomtery)
        {
            shader->AddShader(PATH_JOIN(shaderPath, GS + ".GS.glsl"), GL_GEOMETRY_SHADER);
        }

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}



/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
    if (window->KeyHold(GLFW_KEY_RIGHT)) {
        if(window->KeyHold(GLFW_KEY_T)) {
            mirror_x += mirror_speed * deltaTime;
        }
        if(window->KeyHold(GLFW_KEY_R)) {
            mirror_rotate_x += RADIANS(mirror_rot_angle);
        }
    }
    if (window->KeyHold(GLFW_KEY_LEFT)) {
        if(window->KeyHold(GLFW_KEY_T)) {
            mirror_x -= mirror_speed * deltaTime;
        }
        if(window->KeyHold(GLFW_KEY_R)) {
            mirror_rotate_x -= RADIANS(mirror_rot_angle);
        }
    }
    if (window->KeyHold(GLFW_KEY_UP)) {
        if(window->KeyHold(GLFW_KEY_T)) {
            mirror_y += mirror_speed * deltaTime;
        }
        if(window->KeyHold(GLFW_KEY_R)) {
            mirror_rotate_y += RADIANS(mirror_rot_angle);
        }
    }
    if (window->KeyHold(GLFW_KEY_DOWN)) {
        if(window->KeyHold(GLFW_KEY_T)) {
            mirror_y -= mirror_speed * deltaTime;
        }
        if(window->KeyHold(GLFW_KEY_R)) {
            mirror_rotate_y -= RADIANS(mirror_rot_angle);
        }
    }
    if (window->KeyHold(GLFW_KEY_Z)) {
        if(window->KeyHold(GLFW_KEY_T)) {
            mirror_z += mirror_speed * deltaTime;
        }
        if(window->KeyHold(GLFW_KEY_R)) {
            mirror_rotate_z += RADIANS(mirror_rot_angle);
        }
    }
    if (window->KeyHold(GLFW_KEY_C)) {
        if(window->KeyHold(GLFW_KEY_T)) {
            mirror_z -= mirror_speed * deltaTime;
        }
        if(window->KeyHold(GLFW_KEY_R)) {
            mirror_rotate_z -= RADIANS(mirror_rot_angle);
        }
    }

    if (window->KeyHold(GLFW_KEY_L) && window->KeyHold(GLFW_KEY_T)) {
        tx += mirror_speed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_J) && window->KeyHold(GLFW_KEY_T)) {
        tx -= mirror_speed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_I) && window->KeyHold(GLFW_KEY_T)) {
        ty += mirror_speed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_K) && window->KeyHold(GLFW_KEY_T)) {
        ty -= mirror_speed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_U) && window->KeyHold(GLFW_KEY_T)) {
        tz += mirror_speed * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_O) && window->KeyHold(GLFW_KEY_T)) {
        tz -= mirror_speed * deltaTime;
    }

}


void Tema1::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_1)
    {
        type = 1;
    }

    // for particles
    if (key == GLFW_KEY_P)
    {
        draw_particles = 1 - draw_particles;
    }

    // for outline
    if (key == GLFW_KEY_O)
    {
        draw_outline = 1 - draw_outline;
    }
    
    // for bezier
    if (key == GLFW_KEY_B)
    {
        b_5_dif = 1 - b_5_dif;
    }

    if (key == GLFW_KEY_2)
    {
        type = 0;
    }

    if(key == GLFW_KEY_F1) {
        if(mirror_speed < 10) {
            mirror_speed ++;
        }
    }

    if(key == GLFW_KEY_F2) {
        if(mirror_speed > 0) {
            mirror_speed --;
        }
    }

    if (key == GLFW_KEY_F)
    {
        mirror_x = 0;
        mirror_y = 0.3f;
        mirror_z = -6;

        mirror_rotate_x = 0;
        mirror_rotate_y = 0;
        mirror_rotate_z = 0;

    }

    if (key == GLFW_KEY_1)
    {
        scene = 0;
        ResetParticlesFireworks(20,20,20);
        generator_position = glm::vec3(0, 0, 0);
    }
    
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema1::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
