#version 430

// Input and output topologies
layout(triangles) in;
layout(triangle_strip, max_vertices = 170) out;

// Input
layout(location = 0) in vec2 v_texture_coord[];

// Uniform properties
uniform mat4 View;
uniform mat4 Projection;
uniform int instances;
// TODO(student): Declare other uniforms here
uniform float shrink;

// Output
layout(location = 0) out vec2 texture_coord;


void EmitPoint(vec3 pos, vec3 offset)
{
    gl_Position = Projection * View * vec4(pos + offset, 1.0);
    EmitVertex();
}


void main()
{
    vec3 p1 = gl_in[0].gl_Position.xyz;
    vec3 p2 = gl_in[1].gl_Position.xyz;
    vec3 p3 = gl_in[2].gl_Position.xyz;

    const vec3 INSTANCE_OFFSET = vec3(1.25, 0, 1.25);
    const int NR_COLS = 6;

    // TODO(student): Second, modify the points so that the
    // triangle shrinks relative to its center

    vec3 center = vec3((p1.x + p2.x + p3.x)/3, (p1.y + p2.y + p3.y)/3, (p1.z + p2.z + p3.z)/3);
    p1 = center + (p1-center) * shrink;
    p2 = center + (p2-center) * shrink;
    p3 = center + (p3-center) * shrink;

    int cols = NR_COLS;
    while(cols > 0) {
        for (int i = 0; i <= instances; i++)
        {
            // TODO(student): First, modify the offset so that instances
            // are displayed on `NR_COLS` columns. Test your code by
            // changing the value of `NR_COLS`. No need to recompile.
            vec3 offset = vec3(cols*0.5, 0, i+1);

            texture_coord = v_texture_coord[0];
            EmitPoint(p1, offset);

            texture_coord = v_texture_coord[1];
            EmitPoint(p2, offset);

            texture_coord = v_texture_coord[2];
            EmitPoint(p3, offset);

            EndPrimitive();
        }
        cols--;
    }

}
