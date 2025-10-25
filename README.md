## Vertex Attribute Description
- Binding: The vertex buffer the attribute is located in
- Location
- Offset
- Format: Type of vertex

Example (color) {(x, y), (r, g, b), ...}:
binding = 0
lcoation = 1
offset = 8 bytes
format = VK_FORMAT_R32G32B32_SFLOAT

### Common Vulkan data formats:
- float → VK_FORMAT_R32_SFLOAT
- vec2 → VK_FORMAT_R32G32_SFLOAT
- vec3 → VK_FORMAT_R32G32B32_SFLOAT
- vec4 → VK_FORMAT_R32G32B32A32_SFLOAT
- ivec2 → VK_FORMAT_R32G32_SINT (entero con signo)
- uvec2 → VK_FORMAT_R32G32_UINT (entero sin signo)
- double → VK_FORMAT_R64_SFLOAT

---