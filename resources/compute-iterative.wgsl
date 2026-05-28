struct Uniform {
    viewProj    : mat4x4<f32>,
    invViewProj : mat4x4<f32>,
    camPos      : vec3<f32>,
    voxelCount  : u32,
};

struct Voxel {
    pos    : vec3<f32>,
    size   : f32,
    color  : vec3<f32>,
    _pad0  : f32,
    normal : vec3<f32>,
    _pad1  : f32,
};

struct Ray {
    origin : vec3<f32>,
    dir    : vec3<f32>,
};

struct Hit {
    pos    : vec3<f32>,
    normal : vec3<f32>,
    color  : vec3<f32>,
    t      : f32,
};

@group(0) @binding(0) var output_texture : texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(1) var<uniform>        uniforms : Uniform;
@group(0) @binding(2) var<storage, read>  voxels   : array<Voxel>;
fn sphere_normal(p: vec3<f32>, sphere_center: vec3<f32>) -> vec3<f32> {
    return normalize(p - sphere_center);
}


fn make_ray(id: vec2<u32>, dims: vec2<u32>) -> Ray {
    // Pixel center in NDC [-1, 1]
    let ndc = vec2<f32>(
         (f32(id.x) + 0.5) / f32(dims.x) * 2.0 - 1.0,
        -(f32(id.y) + 0.5) / f32(dims.y) * 2.0 + 1.0  // flip Y
    );

    // Unproject two points on the ray through invViewProj
    let near_h = uniforms.invViewProj * vec4<f32>(ndc, 0.0, 1.0);
    let far_h  = uniforms.invViewProj * vec4<f32>(ndc, 1.0, 1.0);

    let near_w = near_h.xyz / near_h.w;
    let far_w  = far_h.xyz  / far_h.w;

    return Ray(uniforms.camPos, normalize(far_w - near_w));
}

fn intersect(ray: Ray, voxel: Voxel) -> Hit {
    let half    = voxel.size * 0.5;
    let box_min = voxel.pos - half;
    let box_max = voxel.pos + half;

    let inv_d = 1.0 / ray.dir;
    let t0    = (box_min - ray.origin) * inv_d;
    let t1    = (box_max - ray.origin) * inv_d;

    let tmin  = max(max(min(t0.x, t1.x), min(t0.y, t1.y)), min(t0.z, t1.z));
    let tmax  = min(min(max(t0.x, t1.x), max(t0.y, t1.y)), max(t0.z, t1.z));

    var hit : Hit;
    hit.t = -1.0;

    if (tmax < 0.0 || tmin > tmax) { return hit; }

    let t       = select(tmin, tmax, tmin < 0.0); // inside box: use tmax
    let hit_pos = ray.origin + ray.dir * t;



    hit.pos    = hit_pos;
    hit.normal = voxel.normal;
    hit.color  = voxel.color;
    hit.t      = t;
    return hit;
}

@compute @workgroup_size(8, 8)
fn cs_main(@builtin(global_invocation_id) id: vec3<u32>) {
    let dims = textureDimensions(output_texture);
    if (id.x >= dims.x || id.y >= dims.y) { return; }

    let ray = make_ray(id.xy, dims);

    var hit : Hit;
    hit.t = -1.0;

    var minT = 1e9;
    for (var i = 0u; i < uniforms.voxelCount; i++) {
        let h = intersect(ray, voxels[i]);
        // if (h.t > 0.0) {
        //     hit = h;
        //     // Early exit on first hit (opaque)
        //     break;
        // }
        if (h.t > 0.0 && h.t < minT) {
            hit = h;
            minT = h.t;
        }

        // After getting a hit, override the face normal with the true sphere normal

    }

    var color : vec3<f32>;
    if (hit.t > 0.0) {
        let light = normalize(vec3<f32>(1.0, 2.0, -1.0));
        let diff  = max(dot(hit.normal, light), 0.0);
        color     = hit.color * (diff + 0.15);
    } else {
        color = vec3<f32>(0.05, 0.05, 0.15);
    }

    textureStore(output_texture, id.xy, vec4<f32>(color, 1.0));
}