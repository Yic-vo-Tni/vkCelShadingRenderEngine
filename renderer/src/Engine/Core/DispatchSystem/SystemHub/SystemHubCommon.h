//
// Created by lenovo on 9/8/2024.
//

#ifndef VKCELSHADINGRENDERER_SYSTEMHUBCOMMON_H
#define VKCELSHADINGRENDERER_SYSTEMHUBCOMMON_H

#define HANA(...) BOOST_HANA_DEFINE_STRUCT(__VA_ARGS__)

#define MAKE_OPTIONAL(r, data, elem) (std::optional<BOOST_PP_TUPLE_ELEM(2, 0, elem)>, BOOST_PP_TUPLE_ELEM(2, 1, elem))
#define HANA_OPT(name, ...) \
    BOOST_HANA_DEFINE_STRUCT(name, \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(MAKE_OPTIONAL, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))) \
    );

#define RETURN(Member) \
    [[nodiscard]] auto& Member##_() { \
        if (!Member.has_value()) \
            throw std::runtime_error(#Member " is not exist"); \
        return Member.value(); \
    }                      \

#endif //VKCELSHADINGRENDERER_SYSTEMHUBCOMMON_H
