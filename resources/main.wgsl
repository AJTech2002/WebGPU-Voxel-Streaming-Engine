const positions = array<vec2f, 6>(
    vec2f(-1.0, -1.0), // bottom left
    vec2f( 1.0, -1.0), // bottom right
    vec2f( 1.0,  1.0), // top right

    vec2f(-1.0, -1.0), // bottom left
    vec2f( 1.0,  1.0), // top right
    vec2f(-1.0,  1.0)  // top left
);

@group(0) @binding(0)
var myTexture: texture_2d<f32>;

@group(0) @binding(1)
var mySampler: sampler;

@vertex
fn vs_main(
    @builtin(vertex_index) vertex_index : u32
) -> @builtin(position) vec4f {
    return vec4f(positions[vertex_index], 0.0, 1.0);
}

@fragment
fn fs_main(
    @builtin(position) fragCoord : vec4f
) -> @location(0) vec4f {

    let resolution = vec2f(1280.0, 720.0);
    let uv = fragCoord.xy / resolution;

    // return vec4f(
    //     uv.x,
    //     uv.y,
    //     0.8,
    //     1.0
    // );

    return vec4f(textureSample(myTexture, mySampler, uv).rg,  0.0, 1.0);
}