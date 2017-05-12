#if defined(DIRECTX)
#define PUSH_CONSTANT
#elif defined(VULKAN)
#define PUSH_CONSTANT : layout(push_constant)
#endif