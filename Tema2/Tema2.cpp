#include "lab_m2/Tema2/Tema2.h"

#include <vector>
#include <iostream>
#include <chrono>

#include "pfd/portable-file-dialogs.h"

//using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
    outputMode = 0;
    gpuProcessing = false;
    saveScreenToImage = false;
    window->SetSize(600, 600);
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    // Load default texture fore imagine processing
    originalImage       = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, folder, image_name), nullptr, "image", true, true);
    processedImage      = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, folder, image_name), nullptr, "newImage", true, true);
    processedImage2     = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, folder, image_name), nullptr, "newImage2", true, true);
    watermark           = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, folder, watermark_name), nullptr, "watermark", true, true);
    processedWatermark  = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, folder, watermark_name), nullptr, "newWatermark", true, true);
    processedWatermark2 = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, folder, watermark_name), nullptr, "newWatermark2", true, true);

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema2", "shaders");

    // Create a shader program for image processing
    {
        Shader *shader = new Shader("ImageProcessing");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentShader.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


void Tema2::FrameStart()
{
}


void Tema2::Update(float deltaTimeSeconds)
{
    ClearScreen();

    auto shader = shaders["ImageProcessing"];
    shader->Use();

    if (saveScreenToImage)
    {
        window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
    }

    int flip_loc = shader->GetUniformLocation("flipVertical");
    glUniform1i(flip_loc, saveScreenToImage ? 0 : 1);

    int screenSize_loc = shader->GetUniformLocation("screenSize");
    glm::ivec2 resolution = window->GetResolution();
    glUniform2i(screenSize_loc, resolution.x, resolution.y);

    int outputMode_loc = shader->GetUniformLocation("outputMode");
    glUniform1i(outputMode_loc, outputMode);

    int locTexture = shader->GetUniformLocation("textureImage");
    glUniform1i(locTexture, 0);

    auto textureImage = (gpuProcessing == true) ? originalImage : processedImage;
    textureImage->BindToTextureUnit(GL_TEXTURE0);

    RenderMesh(meshes["quad"], shader, glm::mat4(1));

    if (saveScreenToImage)
    {
        saveScreenToImage = false;

        GLenum format = GL_RGB;
        if (originalImage->GetNrChannels() == 4)
        {
            format = GL_RGBA;
        }

        glReadPixels(0, 0, originalImage->GetWidth(), originalImage->GetHeight(), format, GL_UNSIGNED_BYTE, processedImage->GetImageData());
        processedImage->UploadNewData(processedImage->GetImageData());
        SaveImage("shader_processing_" + std::to_string(outputMode));

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}


void Tema2::FrameEnd()
{
    DrawCoordinateSystem();
}


void Tema2::OnFileSelected(const std::string &fileName)
{
    if (fileName.size())
    {
        std::cout << fileName << std::endl;
        originalImage = TextureManager::LoadTexture(fileName, nullptr, "image", true, true);
        processedImage = TextureManager::LoadTexture(fileName, nullptr, "newImage", true, true);
        processedImage2 = TextureManager::LoadTexture(fileName, nullptr, "newImage2", true, true);

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}

void Tema2::DoNothing(Texture2D* originalImage, Texture2D* processedImage)
{
    std::cout << "DoNothing" << std::endl;
    processedImage->UploadNewData(originalImage->GetImageData());
}

void Tema2::GrayScale(Texture2D* originalImage, Texture2D* processedImage)
{
    std::cout << "GrayScale" << std::endl;

    unsigned int channels   = originalImage->GetNrChannels();
    unsigned char* data     = originalImage->GetImageData();
    unsigned char* newData  = processedImage->GetImageData();

    if (channels < 3)
        return;

    glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

    for (int i = 0; i < imageSize.y; i++)
    {
        for (int j = 0; j < imageSize.x; j++)
        {
            int row = channels * (i * imageSize.x + j);

            // Reset save image data
            char value = static_cast<char>(data[row + 0] * 0.2f + data[row + 1] * 0.71f + data[row + 2] * 0.07);
            memset(&newData[row], value, 3);
        }
    }

    processedImage->UploadNewData(newData);
}

void Tema2::Blur(Texture2D* originalImage, Texture2D* processedImage, int blurRadius) {
    std::cout << "Blur" << std::endl;

    unsigned int channels = originalImage->GetNrChannels();
    unsigned char* data     = originalImage->GetImageData();
    unsigned char* newData = processedImage->GetImageData();

    if (channels < 3)
        return;

    glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
    
    for (int i = 0; i < imageSize.y; i++) {
        for (int j = 0; j < imageSize.x; j++) {

            int row = channels * (i * imageSize.x + j);
            //int row = channels * ((blurRadius) * imageSize.x + j - blurRadius);

            // Accumulate color values for the blur
            int blurSum[3] = { 0 };

            for (int blurY = -blurRadius; blurY <= blurRadius; blurY++)
            {
                for (int blurX = -blurRadius; blurX <= blurRadius; blurX++)
                {
                    int neighborX = glm::clamp(j + blurX, 0, imageSize.x - 1);
                    int neighborY = glm::clamp(i + blurY, 0, imageSize.y - 1);

                    int neighborOffset = channels * (neighborY * imageSize.x + neighborX);

                    blurSum[0] += data[neighborOffset + 0];
                    blurSum[1] += data[neighborOffset + 1];
                    blurSum[2] += data[neighborOffset + 2];
                }
            }

            // Average the accumulated color values
            for (int k = 0; k < 3; k++)
            {
                newData[row + k] = static_cast<unsigned char>(blurSum[k] / ((2 * blurRadius + 1) * (2 * blurRadius + 1)));
                //newData[row + k] = static_cast<unsigned char>(blurSum[k] / pow((2 * blurRadius + 1), 2));
            }
        }
    }

    processedImage->UploadNewData(newData);
    processedImage->UploadNewData(newData);
}

void Tema2::Sobel(Texture2D* originalImage, Texture2D* processedImage, Texture2D* processedImage2, int blurRadius)
{
    std::cout << "Sobel" << std::endl;

    Blur(originalImage, processedImage, blurRadius);
    GrayScale(processedImage, processedImage2);

    unsigned int channels = originalImage->GetNrChannels();
    unsigned char* data = processedImage2->GetImageData();
    unsigned char* newData = processedImage->GetImageData();

    glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

    for (int i = 1; i < imageSize.y - 1; i++)
    {
        for (int j = 1; j < imageSize.x - 1; j++)
        {
            int row = channels * (i * imageSize.x + j);

            // Sobel operator masks
            int gx = 
                -data[channels * ((i - 1) * imageSize.x + j - 1)]
                +data[channels * ((i - 1) * imageSize.x + j + 1)] +
                -2 * data[channels * (i * imageSize.x + j - 1)]
                +2 * data[channels * (i * imageSize.x + j + 1)] +
                -data[channels * ((i + 1) * imageSize.x + j - 1)]
                +data[channels * ((i + 1) * imageSize.x + j + 1)];

            int gy = 
                -data[channels * ((i - 1) * imageSize.x + j - 1)] +
                -data[channels * ((i - 1) * imageSize.x + j + 1)] +
                -2 * data[channels * ((i - 1) * imageSize.x + j)] +
                2 * data[channels * ((i + 1) * imageSize.x + j)] +
                data[channels * ((i + 1) * imageSize.x + j - 1)] +
                data[channels * ((i + 1) * imageSize.x + j + 1)];

            // Calculate gradient magnitude
            //int magnitude = static_cast<int>(std::sqrt(gx * gx + gy * gy));
            int magnitude = abs(gx) + abs(gy);

            // Clamp the magnitude to the valid range [0, 255]
            magnitude = min(255, max(0, magnitude));

            //filter values bellow treshold
            int val = magnitude < prag ? 0 : 255;

            //Set the new pixel value
            newData[row + 0] = static_cast<unsigned char>(val);
            newData[row + 1] = static_cast<unsigned char>(val);
            newData[row + 2] = static_cast<unsigned char>(val);
        }
    }

    processedImage->UploadNewData(newData);
}

void Tema2::RemoveWatermark(Texture2D* originalImage, Texture2D* processedImage, Texture2D* processedImage2,
    Texture2D* watermark, Texture2D* processedWatermark, Texture2D* processedWatermark2)
{
    std::cout << "Remove watermark\n";
    Sobel(originalImage, processedImage, processedImage2, 1);
    Sobel(watermark, processedWatermark, processedWatermark2, 1);

    unsigned char* data = processedImage->GetImageData();
    unsigned char* markData = processedWatermark->GetImageData();
    unsigned char* wMarrk = watermark->GetImageData();

    // reset processedImage2 to original Image to use as base for the filtered image
    // so that the original remains unchanged
    memcpy(processedImage2->GetImageData(), originalImage->GetImageData(), 
            originalImage->GetWidth() * originalImage->GetHeight() * originalImage->GetNrChannels() * sizeof(unsigned char));
    unsigned char* result = processedImage2->GetImageData();

    glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
    glm::ivec2 markSize = glm::ivec2(watermark->GetWidth(), watermark->GetHeight());

    int channels = originalImage->GetNrChannels();
    int markChannels = watermark->GetNrChannels();

    register int y, x;
    register int yw, xw;
    register int offsetLineMark, offsetMark;
    register int row, windowTotal, lineImage, localOffset, line, neg;

    //numara cati de "1" sunt in watermark
    register int total = 0;

    for (int yw = 0; yw < markSize.y; yw++) {
        offsetLineMark = yw * markSize.x * markChannels;
        
        for (xw = 0; xw < markSize.x; xw++) {
            offsetMark = markChannels * xw + offsetLineMark;

            if (markData[offsetMark] == 255) {
                total += 1;
            }
        }
    }

    //scaleaza proportional cu numarul de puncte sarite pe x si pe y din watermark
    total = total / (skip * skip);
    int pragHigh = static_cast<int>(total * high);
    int pragLow = static_cast<int>(total * low);

    register int boundY = imageSize.y - markSize.y;
    register int boundX = imageSize.x - markSize.x;

    // optimizari la calcule
    // 1. inversare for-uri: mai intai y si dupa x
    // 2. precalcularea offsetuli de linie
    // 3. folosire "register" pentru indici si constante in bucle
    // 4. detectare constante din bucle
    for (y = 0; y < boundY; ++y)
    {
        line = y * imageSize.x * channels;
        for (x = 0; x < boundX; x++)
        {
            row = line + x * channels;
            windowTotal = 0;
            neg = 0;

            //numara cati de unu sunt in feresatra cu coltul stanga sus in (x, y) din skip in skip pixeli
            for (yw = 0; yw < markSize.y; yw += skip)
            {
                offsetLineMark = yw * markSize.x * markChannels;
                lineImage = yw * imageSize.x * channels + row;

                for (xw = 0; xw < markSize.x; xw += skip)
                {
                    localOffset = lineImage + channels * xw;
                    if (data[localOffset] == 0) { //ignora valorile de 0
                        continue;
                    }
                    offsetMark = offsetLineMark + markChannels * xw;

                    if (data[localOffset] == markData[offsetMark]) {
                        windowTotal += 1;
                    }
                }
            }

            if (windowTotal >= pragHigh) {
                // numarul de puncte de potrivire este peste pragul ales
                // => se trece la scoaterea marcajului
                // adica la operatia inversa din enunt
                for (yw = 0; yw < markSize.y; yw++)
                {
                    offsetLineMark = yw * markSize.x * markChannels;
                    lineImage = yw * imageSize.x * channels + row;
                    for (xw = 0; xw < markSize.x; xw++)
                    {
                        offsetMark = markChannels * xw + offsetLineMark;
                        localOffset = channels * xw + lineImage;

                        result[localOffset + 0] -= wMarrk[offsetMark];
                        result[localOffset + 1] -= wMarrk[offsetMark + 1];
                        result[localOffset + 2] -= wMarrk[offsetMark + 2];
                    }
                }
                x += markSize.x - 2;
            }
            if (windowTotal < pragLow) { 
                // sari peste mai multi pixeli
                // regiunea e cam goala
                // nu are sens sa verific in imediata vecinatate
                x += markSize.x / (skip * 2);
            }
        }
    }

    processedImage->UploadNewData(processedImage2->GetImageData());
}

void Tema2::SaveImage(const std::string &fileName)
{
    std::cout << "Saving image! ";
    processedImage->SaveToFile((fileName + ".png").c_str());
    std::cout << "[Done]" << std::endl;
}


void Tema2::OpenDialog()
{
    std::vector<std::string> filters =
    {
        "Image Files", "*.png *.jpg *.jpeg *.bmp",
        "All Files", "*"
    };

    auto selection = pfd::open_file("Select a file", ".", filters).result();
    if (!selection.empty())
    {
        std::cout << "User selected file " << selection[0] << "\n";
        OnFileSelected(selection[0]);
    }
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}


void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
    }

    if (key == GLFW_KEY_E)
    {
        gpuProcessing = !gpuProcessing;
        if (gpuProcessing == false)
        {
            outputMode = 0;
        }
        std::cout << "Processing on GPU: " << (gpuProcessing ? "true" : "false") << std::endl;
    }

    if (key == GLFW_KEY_UP)
    {
        radius++;
        std::cout << "radius = " << radius << std::endl;
    }

    if (key == GLFW_KEY_DOWN)
    {
        radius--;
        std::cout << "radius = " << radius << std::endl;
    }

    if (key == GLFW_KEY_LEFT)
    {
        prag -= 5;
        std::cout << "prag = " << prag << std::endl;
    }

    if (key == GLFW_KEY_RIGHT)
    {
        prag += 5;
        std::cout << "prag = " << prag << std::endl;
    }

    if (key - GLFW_KEY_0 >= 0 && key <= GLFW_KEY_5)
    {
        outputMode = key - GLFW_KEY_0;

        if (gpuProcessing == false)
        {
            switch (outputMode) 
            {
                case 1:
                {
                    GrayScale(originalImage, processedImage);
                    std::cout << outputMode << std::endl;
                    break;
                }
            
                case 2:
                {
                    Blur(originalImage, processedImage, radius);
                    std::cout << outputMode << std::endl;
                    break;
                }

                case 3:
                {
                    Sobel(originalImage, processedImage, processedImage2, radius);
                    std::cout << outputMode << std::endl;
                    break;
                }

                case 4:
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    RemoveWatermark(originalImage, processedImage, processedImage2, watermark, processedWatermark, processedWatermark2);
                    auto stop = std::chrono::high_resolution_clock::now();
                    
                    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
                    auto durationms = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

                    std::cout << "Time used: " << duration.count() << " seconds" << std::endl;
                    std::cout << "Time used: " << durationms.count() << " microseconds" << std::endl;
                    
                    break;
                }
                default:
                {
                    std::cout << "Invalid outputMode value" << std::endl;
                    std::cout << outputMode << std::endl;
                    DoNothing(originalImage, processedImage);
                    break;
                    //DoNothing(originalImage, processedImage);
                }
            }


        }
    }

    if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
    {
        if (!gpuProcessing)
        {
            SaveImage("processCPU_" + std::to_string(outputMode));
        } else {
            saveScreenToImage = true;
        }
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema2::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
