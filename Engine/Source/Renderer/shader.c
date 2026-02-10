#include "shader.h"
#include "core/string_utils.h"

bool shader_create(Context const* context, Shader_Config_Resource const* config, Shader* shader)
{
    if (string_equal(config->renderpass_name, "renderpass_builtin_world"))
    {
        shader->renderpass = &context->main_renderpass;
    }
    else if (string_equal(config->renderpass_name, "renderpass_builtin_ui"))
    {
        shader->renderpass = &context->ui_renderpass;
    }
    else
    {
        ASSERT(false);
    }

    // TODO: Make configurable
    shader->config.max_descriptor_set_count = 1024;

}
