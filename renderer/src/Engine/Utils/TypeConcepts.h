//
// Created by lenovo on 6/19/2024.
//

#ifndef VKCELSHADINGRENDERER_TYPECONCEPTS_H
#define VKCELSHADINGRENDERER_TYPECONCEPTS_H

namespace yic::tp{

    template<typename T>
    concept is_string = std::is_same_v<std::decay_t<T>, std::string> ||
            std::is_same_v<std::decay_t<T>, const char*>;

}


#endif //VKCELSHADINGRENDERER_TYPECONCEPTS_H
