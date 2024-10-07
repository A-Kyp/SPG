#version 410

// Input
layout(location = 0) in vec2 texture_coord;

// Uniform properties
uniform sampler2D textureImage;
uniform ivec2 screenSize;
uniform int flipVertical;
uniform int outputMode = 2; // 0: original, 1: grayscale, 2: blur

// Output
layout(location = 0) out vec4 out_color;

// Local variables
vec2 textureCoord = vec2(texture_coord.x, (flipVertical != 0) ? 1 - texture_coord.y : texture_coord.y); // Flip texture


vec4 grayscale()
{
    vec4 color = texture(textureImage, textureCoord);
    float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b; 
    return vec4(gray, gray, gray,  0);
}


vec4 blur(int blurRadius)
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 sum = vec4(0);
    for(int i = -blurRadius; i <= blurRadius; i++)
    {
        for(int j = -blurRadius; j <= blurRadius; j++)
        {
            sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize);
        }
    }
        
    float samples = pow((2 * blurRadius + 1), 2);
    return sum / samples;
}

vec4 blur_grey(int blurRadius) 
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 sum = vec4(0);
    for(int i = -blurRadius; i <= blurRadius; i++)
    {
        for(int j = -blurRadius; j <= blurRadius; j++)
        {
            sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize);
        }
    }
        
    float samples = pow((2 * blurRadius + 1), 2);
    sum = sum / samples;
    float gray = 0.21 * sum.r + 0.71 * sum.g + 0.07 * sum.b; 
    return vec4(gray, gray, gray,  0);
}

vec4 blur_grey_pixel(int blurRadius, int x, int y) 
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 sum = vec4(0);
    for(int i = -blurRadius; i <= blurRadius; i++)
    {
        for(int j = -blurRadius; j <= blurRadius; j++)
        {
            sum += texture(textureImage, textureCoord + vec2(i+x, j+y) * texelSize);
        }
    }
        
    float samples = pow((2 * blurRadius + 1), 2);
    sum = sum / samples;
    float gray = 0.21 * sum.r + 0.71 * sum.g + 0.07 * sum.b; 
    return vec4(gray, gray, gray,  0);
}

vec4 sobel(int blur_radius) 
{

    vec2 texelSize = 1.0f / screenSize;
    vec4 dx = vec4(0),dy = vec4(0), amplitude = vec4(0);
    vec4 color = blur_grey_pixel(blur_radius,0,0);
//    dx += texture(textureImage, textureCoord + vec2(1, 1) * texelSize);
    dx += blur_grey_pixel(blur_radius,1,1);
//    dx += 2*texture(textureImage, textureCoord + vec2(1, 0) * texelSize);
    dx += 2*blur_grey_pixel(blur_radius,1,0);
//    dx += texture(textureImage, textureCoord + vec2(1, -1) * texelSize);
    dx += blur_grey_pixel(blur_radius,1,-1);
//    dx -= texture(textureImage, textureCoord + vec2(-1, 1) * texelSize);
    dx -= blur_grey_pixel(blur_radius,-1,1);
//    dx -= 2*texture(textureImage, textureCoord + vec2(-1, 0) * texelSize);
    dx -= 2*blur_grey_pixel(blur_radius,-1,0);
//    dx -= texture(textureImage, textureCoord + vec2(-1, -1) * texelSize);
    dx -= blur_grey_pixel(blur_radius,-1,-1);

//    dy += texture(textureImage, textureCoord + vec2(1, 1) * texelSize);
    dy += blur_grey_pixel(blur_radius,1,1);
//    dy += 2*texture(textureImage, textureCoord + vec2(0, 1) * texelSize);
    dy += 2*blur_grey_pixel(blur_radius,0,1);
//    dy += texture(textureImage, textureCoord + vec2(-1, 1) * texelSize);
    dy += blur_grey_pixel(blur_radius,-1,1);
//    dy -= texture(textureImage, textureCoord + vec2(1, -1) * texelSize);
    dy -= blur_grey_pixel(blur_radius,1,-1);
//    dy -= 2*texture(textureImage, textureCoord + vec2(0, -1) * texelSize);
    dy -= 2*blur_grey_pixel(blur_radius,0,-1);
//    dy -= texture(textureImage, textureCoord + vec2(-1, -1) * texelSize);
    dy -= blur_grey_pixel(blur_radius,-1,-1);

    float gradient = length(vec2(dx.r + dx.g + dx.b, dy.r + dy.g + dy.b));
    gradient = clamp(gradient, 0.0, 1.0);

    float treshold = 0.9;

    if (gradient < treshold) {
        color = vec4(0);
    }
    else {
        color = vec4(1);
    }

    return color;
}

float bubbleSort(float arr[9]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8-i; j++) {
            if (arr[j] > arr[j+1]) {
                float temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
    return arr[4];
}

float bubbleSort(float arr[25]) {
    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 24-i; j++) {
            if (arr[j] > arr[j+1]) {
                float temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
    return arr[12];
}

vec4 median1() {
    vec2 texelSize = 1.0f / screenSize;
    vec4 final = vec4(0);

    float array_r[9];
    float array_g[9];
    float array_b[9];

    int index = 0;

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            array_r[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).x;
            array_g[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).y;
            array_b[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).z;

            index++;
        }
    }


    final.x = bubbleSort(array_r);
    final.y = bubbleSort(array_g);
    final.z = bubbleSort(array_b);

    return final;
}

vec4 median2() {
    vec2 texelSize = 1.0f / screenSize;
    vec4 final = vec4(0);

    float array_r[25];
    float array_g[25];
    float array_b[25];

    int index = 0;

    for(int i = -2; i <= 2; i++)
    {
        for(int j = -2; j <= 2; j++)
        {
            array_r[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).x;
            array_g[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).y;
            array_b[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).z;

            index++;
        }
    }

    final.x = bubbleSort(array_r);
    final.y = bubbleSort(array_g);
    final.z = bubbleSort(array_b);

    return final;
}


void main()
{
    // switch (outputMode)
    // {
    //     case 1:
    //     {
    //         out_color = grayscale();
    //         break;
    //     }

    //     case 2:
    //     {
    //         out_color = blur(3);
    //         break;
    //     }

    //     case 3:
    //     {
    //         out_color = sobel(1);
    //         break;
    //     }

    //     case 4:
    //     {
    //         out_color = blur_grey(3);
    //         break;
    //     }

    //     default:
    //         out_color = texture(textureImage, textureCoord);
    //         break;
    // }
    out_color = texture(textureImage, textureCoord);
}
