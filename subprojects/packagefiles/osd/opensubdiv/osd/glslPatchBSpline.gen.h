"//\n"
"//   Copyright 2013 Pixar\n"
"//\n"
"//   Licensed under the Apache License, Version 2.0 (the \"Apache License\")\n"
"//   with the following modification; you may not use this file except in\n"
"//   compliance with the Apache License and the following modification to it:\n"
"//   Section 6. Trademarks. is deleted and replaced with:\n"
"//\n"
"//   6. Trademarks. This License does not grant permission to use the trade\n"
"//      names, trademarks, service marks, or product names of the Licensor\n"
"//      and its affiliates, except as required to comply with Section 4(c) of\n"
"//      the License and to reproduce the content of the NOTICE file.\n"
"//\n"
"//   You may obtain a copy of the Apache License at\n"
"//\n"
"//       http://www.apache.org/licenses/LICENSE-2.0\n"
"//\n"
"//   Unless required by applicable law or agreed to in writing, software\n"
"//   distributed under the Apache License with the above modification is\n"
"//   distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY\n"
"//   KIND, either express or implied. See the Apache License for the specific\n"
"//   language governing permissions and limitations under the Apache License.\n"
"//\n"
"\n"
"//----------------------------------------------------------\n"
"// Patches.VertexBSpline\n"
"//----------------------------------------------------------\n"
"#ifdef OSD_PATCH_VERTEX_BSPLINE_SHADER\n"
"\n"
"layout(location = 0) in vec4 position;\n"
"OSD_USER_VARYING_ATTRIBUTE_DECLARE\n"
"\n"
"out block {\n"
"    ControlVertex v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} outpt;\n"
"\n"
"void main()\n"
"{\n"
"    outpt.v.position = position;\n"
"    OSD_PATCH_CULL_COMPUTE_CLIPFLAGS(position);\n"
"    OSD_USER_VARYING_PER_VERTEX();\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//----------------------------------------------------------\n"
"// Patches.TessControlBSpline\n"
"//----------------------------------------------------------\n"
"#ifdef OSD_PATCH_TESS_CONTROL_BSPLINE_SHADER\n"
"\n"
"patch out vec4 tessOuterLo, tessOuterHi;\n"
"\n"
"in block {\n"
"    ControlVertex v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} inpt[];\n"
"\n"
"out block {\n"
"    OsdPerPatchVertexBezier v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} outpt[16];\n"
"\n"
"layout(vertices = 16) out;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 cv[16];\n"
"    for (int i=0; i<16; ++i) {\n"
"        cv[i] = inpt[i].v.position.xyz;\n"
"    }\n"
"\n"
"    ivec3 patchParam = OsdGetPatchParam(OsdGetPatchIndex(gl_PrimitiveID));\n"
"    OsdComputePerPatchVertexBSpline(patchParam, gl_InvocationID, cv, outpt[gl_InvocationID].v);\n"
"\n"
"    OSD_USER_VARYING_PER_CONTROL_POINT(gl_InvocationID, gl_InvocationID);\n"
"\n"
"#if defined OSD_ENABLE_SCREENSPACE_TESSELLATION\n"
"    // Wait for all basis conversion to be finished\n"
"    barrier();\n"
"#endif\n"
"    if (gl_InvocationID == 0) {\n"
"        vec4 tessLevelOuter = vec4(0);\n"
"        vec2 tessLevelInner = vec2(0);\n"
"\n"
"        OSD_PATCH_CULL(16);\n"
"\n"
"#if defined OSD_ENABLE_SCREENSPACE_TESSELLATION\n"
"        // Gather bezier control points to compute limit surface tess levels\n"
"        OsdPerPatchVertexBezier cpBezier[16];\n"
"        cpBezier[0] = outpt[0].v;\n"
"        cpBezier[1] = outpt[1].v;\n"
"        cpBezier[2] = outpt[2].v;\n"
"        cpBezier[3] = outpt[3].v;\n"
"        cpBezier[4] = outpt[4].v;\n"
"        cpBezier[5] = outpt[5].v;\n"
"        cpBezier[6] = outpt[6].v;\n"
"        cpBezier[7] = outpt[7].v;\n"
"        cpBezier[8] = outpt[8].v;\n"
"        cpBezier[9] = outpt[9].v;\n"
"        cpBezier[10] = outpt[10].v;\n"
"        cpBezier[11] = outpt[11].v;\n"
"        cpBezier[12] = outpt[12].v;\n"
"        cpBezier[13] = outpt[13].v;\n"
"        cpBezier[14] = outpt[14].v;\n"
"        cpBezier[15] = outpt[15].v;\n"
"\n"
"        OsdEvalPatchBezierTessLevels(cpBezier, patchParam,\n"
"                         tessLevelOuter, tessLevelInner,\n"
"                         tessOuterLo, tessOuterHi);\n"
"#else\n"
"        OsdGetTessLevelsUniform(patchParam, tessLevelOuter, tessLevelInner,\n"
"                         tessOuterLo, tessOuterHi);\n"
"#endif\n"
"\n"
"        gl_TessLevelOuter[0] = tessLevelOuter[0];\n"
"        gl_TessLevelOuter[1] = tessLevelOuter[1];\n"
"        gl_TessLevelOuter[2] = tessLevelOuter[2];\n"
"        gl_TessLevelOuter[3] = tessLevelOuter[3];\n"
"\n"
"        gl_TessLevelInner[0] = tessLevelInner[0];\n"
"        gl_TessLevelInner[1] = tessLevelInner[1];\n"
"    }\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//----------------------------------------------------------\n"
"// Patches.TessEvalBSpline\n"
"//----------------------------------------------------------\n"
"#ifdef OSD_PATCH_TESS_EVAL_BSPLINE_SHADER\n"
"\n"
"layout(quads) in;\n"
"layout(OSD_SPACING) in;\n"
"\n"
"patch in vec4 tessOuterLo, tessOuterHi;\n"
"\n"
"in block {\n"
"    OsdPerPatchVertexBezier v;\n"
"    OSD_USER_VARYING_DECLARE\n"
"} inpt[];\n"
"\n"
"out block {\n"
"    OutputVertex v;\n"
"#if defined OSD_PATCH_ENABLE_SINGLE_CREASE\n"
"    vec2 vSegments;\n"
"#endif\n"
"    OSD_USER_VARYING_DECLARE\n"
"} outpt;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 P = vec3(0), dPu = vec3(0), dPv = vec3(0);\n"
"    vec3 N = vec3(0), dNu = vec3(0), dNv = vec3(0);\n"
"\n"
"    OsdPerPatchVertexBezier cv[16];\n"
"    for (int i = 0; i < 16; ++i) {\n"
"        cv[i] = inpt[i].v;\n"
"    }\n"
"\n"
"    vec2 UV = OsdGetTessParameterization(gl_TessCoord.xy,\n"
"                                         tessOuterLo,\n"
"                                         tessOuterHi);\n"
"\n"
"    ivec3 patchParam = inpt[0].v.patchParam;\n"
"    OsdEvalPatchBezier(patchParam, UV, cv, P, dPu, dPv, N, dNu, dNv);\n"
"\n"
"    // all code below here is client code\n"
"    outpt.v.position = OsdModelViewMatrix() * vec4(P, 1.0f);\n"
"    outpt.v.normal = (OsdModelViewMatrix() * vec4(N, 0.0f)).xyz;\n"
"    outpt.v.tangent = (OsdModelViewMatrix() * vec4(dPu, 0.0f)).xyz;\n"
"    outpt.v.bitangent = (OsdModelViewMatrix() * vec4(dPv, 0.0f)).xyz;\n"
"#ifdef OSD_COMPUTE_NORMAL_DERIVATIVES\n"
"    outpt.v.Nu = dNu;\n"
"    outpt.v.Nv = dNv;\n"
"#endif\n"
"#if defined OSD_PATCH_ENABLE_SINGLE_CREASE\n"
"    outpt.vSegments = cv[0].vSegments;\n"
"#endif\n"
"\n"
"    outpt.v.tessCoord = UV;\n"
"    outpt.v.patchCoord = OsdInterpolatePatchCoord(UV, patchParam);\n"
"\n"
"    OSD_USER_VARYING_PER_EVAL_POINT(UV, 5, 6, 9, 10);\n"
"\n"
"    OSD_DISPLACEMENT_CALLBACK;\n"
"\n"
"    gl_Position = OsdProjectionMatrix() * outpt.v.position;\n"
"}\n"
"\n"
"#endif\n"
"\n"
