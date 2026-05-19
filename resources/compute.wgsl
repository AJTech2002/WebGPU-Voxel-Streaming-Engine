@group(0) @binding(0) var output_texture: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(8, 8)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>) {
    let dims = textureDimensions(output_texture);
    if (id.x >= dims.x || id.y >= dims.y) {
        return;
    }

    let uv = vec2<f32>(id.xy) / vec2<f32>(dims);

    let r = 1.0;
    let g = uv.x;
    let b = uv.y;

    textureStore(output_texture, id.xy, vec4<f32>(r, g, b, 1.0));
}