#pragma once

#include <string>

#include "components/simple_scene.h"
#include "core/gpu/frame_buffer.h"


namespace m2
{
    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void OpenDialog();
        void OnFileSelected(const std::string &fileName);

        // Processing effects
        void GrayScale(Texture2D*, Texture2D*);
        void Blur(Texture2D*, Texture2D*, int);
        void Sobel(Texture2D*, Texture2D*, Texture2D*, int);
        void DoNothing(Texture2D*, Texture2D*);
        void SaveImage(const std::string &fileName);
        void RemoveWatermark(Texture2D*, Texture2D*, Texture2D*, Texture2D*, Texture2D*, Texture2D*);


     private:
        Texture2D *originalImage;
        Texture2D *processedImage;
        Texture2D *processedImage2;
        Texture2D *watermark;
        Texture2D *processedWatermark;
        Texture2D *processedWatermark2;

        std::string folder = "test_images_final";
        std::string image_name = "star.png";
        std::string imageNames[8] = {
            "animation.png", "median.png", "rain.png", "reflection.png", "refraction.png", "sm.png", "snow.png", "star.png"};
        std::string watermark_name = "watermark.png";

        int crtImage = 0;
        int outputMode;
        bool gpuProcessing;
        bool saveScreenToImage;

        int radius = 1;
        int prag = 229;
        int skip = 2;
        float high = 0.84f;
        float low = 0.1f;
    };
}   // namespace m2
