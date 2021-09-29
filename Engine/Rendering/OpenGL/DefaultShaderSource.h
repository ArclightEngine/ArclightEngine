std::vector<uint8_t> defaultFragmentShaderData = {
    0x23, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 
    0x20, 0x33, 0x31, 0x30, 0x20, 0x65, 0x73, 0xa, 
    0xa, 0x70, 0x72, 0x65, 0x63, 0x69, 0x73, 0x69, 
    0x6f, 0x6e, 0x20, 0x6d, 0x65, 0x64, 0x69, 0x75, 
    0x6d, 0x70, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 
    0x3b, 0xa, 0xa, 0x6c, 0x61, 0x79, 0x6f, 0x75, 
    0x74, 0x28, 0x62, 0x69, 0x6e, 0x64, 0x69, 0x6e, 
    0x67, 0x20, 0x3d, 0x20, 0x30, 0x29, 0x20, 0x75, 
    0x6e, 0x69, 0x66, 0x6f, 0x72, 0x6d, 0x20, 0x73, 
    0x61, 0x6d, 0x70, 0x6c, 0x65, 0x72, 0x32, 0x44, 
    0x20, 0x74, 0x65, 0x78, 0x53, 0x61, 0x6d, 0x70, 
    0x6c, 0x65, 0x72, 0x3b, 0x20, 0x2f, 0x2f, 0x20, 
    0x54, 0x65, 0x78, 0x74, 0x75, 0x72, 0x65, 0x20, 
    0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x72, 0xa, 
    0xa, 0x6c, 0x61, 0x79, 0x6f, 0x75, 0x74, 0x28, 
    0x6c, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 
    0x20, 0x3d, 0x20, 0x30, 0x29, 0x20, 0x69, 0x6e, 
    0x20, 0x76, 0x65, 0x63, 0x34, 0x20, 0x66, 0x72, 
    0x61, 0x67, 0x43, 0x6f, 0x6c, 0x6f, 0x75, 0x72, 
    0x3b, 0x20, 0x2f, 0x2f, 0x20, 0x46, 0x72, 0x61, 
    0x67, 0x6d, 0x65, 0x6e, 0x74, 0x20, 0x63, 0x6f, 
    0x6c, 0x6f, 0x75, 0x72, 0xa, 0x6c, 0x61, 0x79, 
    0x6f, 0x75, 0x74, 0x28, 0x6c, 0x6f, 0x63, 0x61, 
    0x74, 0x69, 0x6f, 0x6e, 0x20, 0x3d, 0x20, 0x31, 
    0x29, 0x20, 0x69, 0x6e, 0x20, 0x76, 0x65, 0x63, 
    0x32, 0x20, 0x66, 0x72, 0x61, 0x67, 0x54, 0x65, 
    0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 0x3b, 0x20, 
    0x2f, 0x2f, 0x20, 0x54, 0x65, 0x78, 0x74, 0x75, 
    0x72, 0x65, 0x20, 0x63, 0x6f, 0x6f, 0x72, 0x64, 
    0x69, 0x6e, 0x61, 0x74, 0x65, 0xa, 0xa, 0x6c, 
    0x61, 0x79, 0x6f, 0x75, 0x74, 0x28, 0x6c, 0x6f, 
    0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x3d, 
    0x20, 0x30, 0x29, 0x20, 0x6f, 0x75, 0x74, 0x20, 
    0x76, 0x65, 0x63, 0x34, 0x20, 0x6f, 0x75, 0x74, 
    0x43, 0x6f, 0x6c, 0x6f, 0x75, 0x72, 0x3b, 0x20, 
    0x2f, 0x2f, 0x20, 0x4f, 0x75, 0x74, 0x70, 0x75, 
    0x74, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x75, 0x72, 
    0xa, 0xa, 0x76, 0x6f, 0x69, 0x64, 0x20, 0x6d, 
    0x61, 0x69, 0x6e, 0x28, 0x29, 0x20, 0x7b, 0xa, 
    0x20, 0x20, 0x20, 0x20, 0x6f, 0x75, 0x74, 0x43, 
    0x6f, 0x6c, 0x6f, 0x75, 0x72, 0x20, 0x3d, 0x20, 
    0x66, 0x72, 0x61, 0x67, 0x43, 0x6f, 0x6c, 0x6f, 
    0x75, 0x72, 0x20, 0x2a, 0x20, 0x74, 0x65, 0x78, 
    0x74, 0x75, 0x72, 0x65, 0x28, 0x74, 0x65, 0x78, 
    0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x72, 0x2c, 
    0x20, 0x66, 0x72, 0x61, 0x67, 0x54, 0x65, 0x78, 
    0x43, 0x6f, 0x6f, 0x72, 0x64, 0x29, 0x3b, 0xa, 
    0x7d, 0xa, 
};

std::vector<uint8_t> defaultVertexShaderData = {
    0x23, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 
    0x20, 0x33, 0x31, 0x30, 0x20, 0x65, 0x73, 0xa, 
    0xa, 0x70, 0x72, 0x65, 0x63, 0x69, 0x73, 0x69, 
    0x6f, 0x6e, 0x20, 0x6d, 0x65, 0x64, 0x69, 0x75, 
    0x6d, 0x70, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 
    0x3b, 0xa, 0xa, 0x6c, 0x61, 0x79, 0x6f, 0x75, 
    0x74, 0x28, 0x62, 0x69, 0x6e, 0x64, 0x69, 0x6e, 
    0x67, 0x20, 0x3d, 0x20, 0x31, 0x29, 0x20, 0x75, 
    0x6e, 0x69, 0x66, 0x6f, 0x72, 0x6d, 0x20, 0x54, 
    0x72, 0x61, 0x6e, 0x73, 0x66, 0x6f, 0x72, 0x6d, 
    0x20, 0x7b, 0xa, 0x20, 0x20, 0x20, 0x20, 0x6d, 
    0x61, 0x74, 0x34, 0x20, 0x76, 0x69, 0x65, 0x77, 
    0x70, 0x6f, 0x72, 0x74, 0x3b, 0xa, 0x7d, 0x3b, 
    0xa, 0xa, 0x6c, 0x61, 0x79, 0x6f, 0x75, 0x74, 
    0x28, 0x6c, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 
    0x6e, 0x20, 0x3d, 0x20, 0x30, 0x29, 0x20, 0x75, 
    0x6e, 0x69, 0x66, 0x6f, 0x72, 0x6d, 0x20, 0x6d, 
    0x61, 0x74, 0x34, 0x20, 0x74, 0x72, 0x61, 0x6e, 
    0x73, 0x66, 0x6f, 0x72, 0x6d, 0x3b, 0xa, 0xa, 
    0x6c, 0x61, 0x79, 0x6f, 0x75, 0x74, 0x28, 0x6c, 
    0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 
    0x3d, 0x20, 0x30, 0x29, 0x20, 0x69, 0x6e, 0x20, 
    0x76, 0x65, 0x63, 0x32, 0x20, 0x70, 0x6f, 0x73, 
    0x69, 0x74, 0x69, 0x6f, 0x6e, 0x3b, 0xa, 0x6c, 
    0x61, 0x79, 0x6f, 0x75, 0x74, 0x28, 0x6c, 0x6f, 
    0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x3d, 
    0x20, 0x31, 0x29, 0x20, 0x69, 0x6e, 0x20, 0x76, 
    0x65, 0x63, 0x32, 0x20, 0x74, 0x65, 0x78, 0x43, 
    0x6f, 0x6f, 0x72, 0x64, 0x3b, 0xa, 0x6c, 0x61, 
    0x79, 0x6f, 0x75, 0x74, 0x28, 0x6c, 0x6f, 0x63, 
    0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x3d, 0x20, 
    0x32, 0x29, 0x20, 0x69, 0x6e, 0x20, 0x76, 0x65, 
    0x63, 0x34, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x75, 
    0x72, 0x3b, 0xa, 0xa, 0x6c, 0x61, 0x79, 0x6f, 
    0x75, 0x74, 0x28, 0x6c, 0x6f, 0x63, 0x61, 0x74, 
    0x69, 0x6f, 0x6e, 0x20, 0x3d, 0x20, 0x30, 0x29, 
    0x20, 0x6f, 0x75, 0x74, 0x20, 0x76, 0x65, 0x63, 
    0x34, 0x20, 0x66, 0x72, 0x61, 0x67, 0x43, 0x6f, 
    0x6c, 0x6f, 0x75, 0x72, 0x3b, 0xa, 0x6c, 0x61, 
    0x79, 0x6f, 0x75, 0x74, 0x28, 0x6c, 0x6f, 0x63, 
    0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x3d, 0x20, 
    0x31, 0x29, 0x20, 0x6f, 0x75, 0x74, 0x20, 0x76, 
    0x65, 0x63, 0x32, 0x20, 0x66, 0x72, 0x61, 0x67, 
    0x54, 0x65, 0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 
    0x3b, 0xa, 0xa, 0x76, 0x6f, 0x69, 0x64, 0x20, 
    0x6d, 0x61, 0x69, 0x6e, 0x28, 0x29, 0x20, 0x7b, 
    0xa, 0x20, 0x20, 0x20, 0x20, 0x67, 0x6c, 0x5f, 
    0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 
    0x20, 0x3d, 0x20, 0x76, 0x69, 0x65, 0x77, 0x70, 
    0x6f, 0x72, 0x74, 0x20, 0x2a, 0x20, 0x74, 0x72, 
    0x61, 0x6e, 0x73, 0x66, 0x6f, 0x72, 0x6d, 0x20, 
    0x2a, 0x20, 0x76, 0x65, 0x63, 0x34, 0x28, 0x70, 
    0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x2c, 
    0x20, 0x30, 0x2e, 0x30, 0x2c, 0x20, 0x31, 0x2e, 
    0x30, 0x29, 0x3b, 0xa, 0x20, 0x20, 0x20, 0x20, 
    0x66, 0x72, 0x61, 0x67, 0x43, 0x6f, 0x6c, 0x6f, 
    0x75, 0x72, 0x20, 0x3d, 0x20, 0x63, 0x6f, 0x6c, 
    0x6f, 0x75, 0x72, 0x3b, 0xa, 0x20, 0x20, 0x20, 
    0x20, 0x66, 0x72, 0x61, 0x67, 0x54, 0x65, 0x78, 
    0x43, 0x6f, 0x6f, 0x72, 0x64, 0x20, 0x3d, 0x20, 
    0x74, 0x65, 0x78, 0x43, 0x6f, 0x6f, 0x72, 0x64, 
    0x3b, 0xa, 0x7d, 0xa, 
};
