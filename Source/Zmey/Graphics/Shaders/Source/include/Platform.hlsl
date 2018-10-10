#if defined(DIRECTX)
#define PUSH_CONSTANT
#define SET_BINDING(num)
#elif defined(VULKAN)
#define PUSH_CONSTANT : layout(push_constant)
#define SET_BINDING(num) : layout(binding = num)
#endif