const auto GridVert=R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 modelView;
uniform mat4 projection;
out vec3 near;
out vec3 far;
out vec3 center;
out mat4 model;
out mat4 proj;
void main()
{
    mat4 invProj = inverse(projection);
    mat4 invModelView = inverse(modelView);
    vec4 near4 = invModelView*invProj*vec4(aPos.x, aPos.y, 0.0, 1.0);
    if (near4.w!=0) {near4.xyz /= near4.w;}
    vec4 far4 = invModelView*invProj*vec4(aPos.x, aPos.y, 1.0, 1.0);
    if (far4.w!=0) {far4.xyz /= far4.w;}
    gl_Position = vec4(aPos, 1.0);
    model = modelView;
    proj = projection;
    near = near4.xyz;
    far = far4.xyz;
}
)";

const auto GridFrag=R"(
#version 330 core
//
// Near and far gives 2 points on the camera ray for a particular fragment, we can then compute a so ray intersection with the
// plane in the fragment shader. Using this ray, [pos = near + alpha*(far-near)] to compute the intersection with the ground plane
// (pos.y=0 or pos.z=0) The position of the plance on the ray is then alpha = -near.y/(far.y-near.y) alpha < 0 means it intersects
// behind the camera, so we won't see it in front, so we can discard the fragment
//
out vec4 FragColor;
in vec3 near;
in vec3 far;
in mat4 model;
in mat4 proj;
uniform float planeOrientation;
void main()
{
    // cellSize could be a function parameter, to display grids at multiple levels
    const float cellSize = 1000.0;
    float distanceToPlane = mix(-near.z/(far.z-near.z), -near.y/(far.y-near.y), planeOrientation);
    if (distanceToPlane < 0) discard;
    // plane_pt is the actual point on the plan
    vec3 pointOnPlane = near + distanceToPlane * (far - near);
    vec2 gridCoord = mix(pointOnPlane.xy, pointOnPlane.xz, planeOrientation);
    // We need the sum of the absolute value of derivatives in x and y to get the same projected line widt
    vec2 gridCoordDerivative = fwidth(gridCoord);
    float fadeFactor = length(gridCoordDerivative)/(cellSize/2.f);
    vec2 gridLine = 1.f - abs(clamp(mod(gridCoord, cellSize) / (2.0*gridCoordDerivative), 0.0, 1.0) * 2 - 1.f);
    float gridLineAlpha = max(gridLine.x, gridLine.y);
    vec4 c = vec4(0.7, 0.7, 0.7, 1.0);
    c.a = max(0, gridLineAlpha * (1-fadeFactor*fadeFactor));
    if (c.a == 0) discard;
    FragColor = c;
    // planeProj is the projection of the plane point on this fragment, it allows to compute its dept
    vec4 planeProj = proj*model*vec4(pointOnPlane, 1.0);
    gl_FragDepth = (((gl_DepthRange.far-gl_DepthRange.near) * (planeProj.z/planeProj.w)) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}
)";
