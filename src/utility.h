#pragma once

#include <functional>
#include <filesystem>
#include <iostream>
#include <string>

// #define QT_TRANSLATE_NOOP(scope, x) x
#define Tr(x) x

template<typename T, bool setter = true>
struct MetaInfoCommon;
template<typename T>
struct MetaInfoCommon<T, false> {
    T* v;
    std::string name;
    void* widget = nullptr;
    bool enabled = true;
    std::function<void(void)>* regenerateDialog;
};
template<typename T>
struct MetaInfoCommon<T, true> {
    T* v;
    std::string name;
    std::function<void(T)> setter;
    void* widget = nullptr;
    bool enabled = true;
    std::function<void(void)>* regenerateDialog;
};

template<typename T>
struct MetaInfoNumeric {
    T min = 0, max = 100;
    T step = 1;
};

struct MetaInfoStrVec {
    std::vector<std::string> stringVec;
};

// template<typename T, template<class> class... Ts>
// struct MetaInfo : public Ts<T>... {
//     MetaInfo(Ts<T>... ts) : Ts<T>(ts)... {}
// };
template<typename T, typename... Ts>
struct MetaInfo : public Ts... {
    MetaInfo(Ts... ts) : Ts(ts)... {}
};

template<typename T>
concept Numeric = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

template<Numeric T>
using MetaNumeric = MetaInfo<T, MetaInfoCommon<T>, MetaInfoNumeric<T>>;

using MetaInt = MetaNumeric<int>;
using MetaUint = MetaNumeric<unsigned>;
using MetaDouble = MetaNumeric<double>;
using MetaFloat = MetaNumeric<float>;
using MetaBool = MetaInfo<bool, MetaInfoCommon<bool>>;

using MetaMap = MetaInfo<int, MetaInfoCommon<int>, MetaInfoStrVec>;


template<typename T>
using MetaButton = MetaInfo<T, MetaInfoCommon<T>>;
using MetaFiles = MetaButton<std::vector<std::filesystem::path>>;
using MetaFolderPath = MetaButton<std::filesystem::path>;
using MetaSetterButton = MetaButton<std::monostate>;

template<typename T>
using MetaStruct = MetaInfo<T, MetaInfoCommon<T, false>>;
