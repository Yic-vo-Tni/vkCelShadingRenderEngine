//
// Created by lenovo on 6/19/2024.
//

#ifndef VKCELSHADINGRENDERER_TYPECONCEPTS_H
#define VKCELSHADINGRENDERER_TYPECONCEPTS_H

namespace yic::tp{

    template<typename T>
    concept is_string = std::is_same_v<std::decay_t<T>, std::string> ||
            std::is_same_v<std::decay_t<T>, const char*>;

    template<typename T, typename U>
    concept Same = std::same_as<T, U>;

    template<typename U, typename...Args>
    concept Same_Args = std::conjunction_v<std::is_same<Args, U>...>;

    template<typename T, typename U>
    concept Same_orVector = std::same_as<T, U> || std::same_as<T, std::vector<U>>;

}


#endif //VKCELSHADINGRENDERER_TYPECONCEPTS_H
