varying vec3 frag_pos;
varying vec3 frag_normal;
varying vec2 texture_coord;

uniform vec3 light_pos;
uniform vec3 light_dir_parallel;
uniform vec3 view_pos;
uniform vec3 light_color;   //3种颜色分量的强度
uniform vec3 k_a;
uniform vec3 k_d;
uniform vec3 k_s;
uniform sampler2D texture_ambient;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_shadow;
uniform float use_ambient_sample;
uniform float use_diffuse_sample;
uniform float use_specular_sample;
varying vec4 frag_pos_light;

float shadowCalc(vec4 pos){
    vec3 proj3d = (pos.xyz / pos.w) * 0.5 + 0.5; //归一化到采样坐标
    float bias = 0.01;
    float shadow = 0.;
    vec2 step = 1.0 / textureSize(texture_shadow, 0);
    for(int x = -1; x < 2; ++x){
        for(int y = -1; y < 2; ++y){
            float depth_tmp = texture2D(texture_shadow, proj3d.xy + vec2(x, y) * step).r;
            float add = depth_tmp + bias < proj3d.z ? 0.f : 1.f;
            shadow += add;
        }
    }
    shadow /= 9;
    if(proj3d.z > 1.)
        shadow = 1.;
    return shadow;
}

void main(){
    vec4 tex_default = vec4(1,1,1,1);
    vec3 ambient_strength = vec3(1, 1, 1) * 1;
    vec3 ambient = k_a * light_color * ambient_strength
            * mix(texture2D(texture_ambient, texture_coord), tex_default, use_ambient_sample);

    vec3 diff_strength = vec3(1, 1, 1) * 3;
    vec3 norm = normalize(frag_normal);
    float diff = max(dot(norm, light_dir_parallel), 0.0);
    vec3 disfuss = diff * light_color * k_d * diff_strength
            * mix(texture2D(texture_diffuse, texture_coord), tex_default, use_diffuse_sample);
    float specular_strength = vec3(1, 1, 1) * 4;
    vec3 view_dir = normalize(view_pos - frag_pos); //view_dir从物体表面指向观察者
    vec3 reflect_dir = reflect(-light_dir_parallel, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color * k_s
           * mix(texture2D(texture_specular, texture_coord), tex_default, use_specular_sample);
    gl_FragColor = vec4((ambient + diff + specular) * shadowCalc(frag_pos_light), 1.0);
}
