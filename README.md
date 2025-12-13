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
### Lighting

---

To look at:
- Index Buffer
- Light transport equation


## Uniform Buffers
A way to provide arbitrary read only-data to shaders
Desktop hardware -> up to 64KB
Mobile hardware -> usually 16KB (mínimum by specification)

Uniform vs push constants
- Reading is slower
- Require additional setup
- binding descriptor sets has some overhead

## Descriptor sets
- A set of pointers to resources that shaders can access.
- There are Buffer and Image descriptors.


## Billboard
2D objects embeded in a 3D world. It must always face the camera.