 
const char *ManipulatorFrag = 
"#version 330 core\n"
"in vec4 color;\n"
"out vec4 FragColor;\n"
"uniform vec3 highlight;\n"
"void main() {\n"
"    // if (color.a == 0.0) discard;\n"
"    if (dot(highlight, color.xyz) > 0.9) {\n"
"       FragColor = vec4(1.0, 1.0, 0.2, color.w);\n"
"    } else {\n"
"       FragColor = color;\n"
"    }\n"
"}\n"
;
